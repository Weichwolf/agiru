#!/usr/bin/env python3
"""Negative controls for analysis selection and tool exit handling."""
import importlib.util
import json
import os
import shutil
import signal
import time
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

SCRIPT = Path(__file__).with_name('lint-analysis.py').resolve()
spec = importlib.util.spec_from_file_location('lint_analysis', SCRIPT)
analysis = importlib.util.module_from_spec(spec)
spec.loader.exec_module(analysis)
verify_spec = importlib.util.spec_from_file_location(
    'verify_snapshot', Path(__file__).resolve().parents[1] / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(verify_spec)
verify_spec.loader.exec_module(verify)


class SnapshotGate(unittest.TestCase):
    def test_sync_preserves_unchanged_objects_and_replaces_changed_inputs(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            source = root / 'archive'
            lane = root / 'lane'
            source.mkdir()
            lane.mkdir()
            for name, value in [('same.h', 'same'), ('changed.cpp', 'new')]:
                (source / name).write_text(value)
            (lane / 'same.h').write_text('same')
            (lane / 'changed.cpp').write_text('old')
            (lane / 'obsolete.cpp').write_text('gone')
            (lane / 'build').mkdir()
            (lane / 'build/retained.o').write_text('object')
            original = (lane / 'same.h').stat().st_mtime_ns
            verify.sync_source(source, lane)
            self.assertEqual((lane / 'same.h').stat().st_mtime_ns, original)
            self.assertEqual((lane / 'changed.cpp').read_text(), 'new')
            self.assertFalse((lane / 'obsolete.cpp').exists())
            self.assertTrue((lane / 'build/retained.o').exists())
            self.assertEqual(verify.digest(source), verify.digest(lane))


class AnalysisGate(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'src').mkdir()
        (self.root / 'src/a.cpp').write_text('int a;\n')
        self.run_git('init', '-q')
        self.run_git('add', 'src/a.cpp')
        self.run_git('-c', 'user.name=Gate', '-c', 'user.email=gate@example.invalid', 'commit', '-qm', 'fixture')
        (self.root / 'compile_commands.json').write_text(json.dumps([
            {'directory': str(self.root), 'file': str(self.root / 'src/a.cpp'),
             'command': 'clang++ -c src/a.cpp -o a.o'},
            {'directory': str(self.root), 'file': str(self.root / 'build/unlinked.cpp'),
             'command': 'clang++ -c build/unlinked.cpp'},
        ]))

    def run_git(self, *args):
        subprocess.run(['git', *args], cwd=self.root, check=True, stdout=subprocess.DEVNULL)

    def test_full_denominator_excludes_generated_build_sources(self):
        selected, total = analysis.select_units(self.root, True)
        self.assertEqual(selected, [self.root / 'src/a.cpp'])
        self.assertEqual(total, 1)

    def test_untracked_source_without_compile_command_fails(self):
        (self.root / 'src/new.cpp').write_text('int b;\n')
        with self.assertRaisesRegex(RuntimeError, 'no compile command'):
            analysis.select_units(self.root, False)

    def test_full_analysis_cannot_omit_uncompiled_source(self):
        (self.root / 'src/new.cpp').write_text('int b;\n')
        with self.assertRaisesRegex(RuntimeError, 'no compile command'):
            analysis.select_units(self.root, True)

    def test_staged_source_is_checked(self):
        (self.root / 'src/a.cpp').write_text('int changed;\n')
        self.run_git('add', 'src/a.cpp')
        self.assertEqual(analysis.select_units(self.root, False)[0], [self.root / 'src/a.cpp'])

    def test_tool_failure_without_diagnostic_is_red(self):
        tidy = self.root / 'tidy'
        tidy.write_text('#!/bin/sh\necho crashed-without-a-diagnostic\nexit 42\n')
        tidy.chmod(0o755)
        result = subprocess.run([sys.executable, str(SCRIPT), '--root', str(self.root),
                                 '--tidy', str(tidy), '--full', '--jobs', '1'],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('exit 42', (self.root / 'build/lint/tidy.log').read_text())

    def test_changed_transitive_header_selects_its_consumer(self):
        header = self.root / 'src/indirect.h'
        header.write_text('#pragma once\n')
        original = subprocess.check_output

        def output(command, **kwargs):
            if command[0] == 'ninja':
                return f'{self.root}/a.o: #deps 2, deps mtime 1 (VALID)\n    {header}\n'
            return original(command, **kwargs)

        with patch.object(subprocess, 'check_output', side_effect=output):
            self.assertEqual(analysis.select_units(self.root, False)[0], [self.root / 'src/a.cpp'])

    def test_changed_header_without_a_compiled_consumer_is_red(self):
        (self.root / 'src/orphan.h').write_text('#pragma once\n')
        original = subprocess.check_output

        def output(command, **kwargs):
            if command[0] == 'ninja':
                return ''
            return original(command, **kwargs)

        with patch.object(subprocess, 'check_output', side_effect=output):
            with self.assertRaisesRegex(RuntimeError, 'header has no compiled consumer'):
                analysis.select_units(self.root, False)

    def test_empty_compile_database_is_not_green(self):
        (self.root / 'compile_commands.json').write_text('[]')
        with self.assertRaisesRegex(RuntimeError, 'no hand-written'):
            analysis.select_units(self.root, True)

    def test_targeted_analysis_requires_a_compiled_unit(self):
        tidy = self.root / 'tidy'
        tidy.write_text('#!/bin/sh\nexit 0\n')
        tidy.chmod(0o755)
        result = subprocess.run([sys.executable, str(SCRIPT), '--root', str(self.root),
                                 '--tidy', str(tidy), '--unit', 'src/missing.cpp'],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn('not a compiled handwritten source', result.stderr)

    def test_targeted_analysis_keeps_full_report_intact(self):
        tidy = self.root / 'tidy'
        tidy.write_text('#!/bin/sh\nexit 0\n')
        tidy.chmod(0o755)
        report = self.root / 'build/lint'
        report.mkdir(parents=True)
        (report / 'tidy.log').write_text('full report survives\n')
        result = subprocess.run([sys.executable, str(SCRIPT), '--root', str(self.root),
                                 '--tidy', str(tidy), '--unit', 'src/a.cpp'],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual((report / 'tidy.log').read_text(), 'full report survives\n')
        self.assertEqual(json.loads((report / 'targeted-units.json').read_text())['checked'], 1)


class DiscoveryGate(unittest.TestCase):
    def test_missing_gate_binary_cannot_shrink_the_population(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / 'test/gate').mkdir(parents=True)
            (root / 'build').mkdir()
            shutil.copyfile(SCRIPT.parents[1] / 'test/run.sh', root / 'test/run.sh')
            (root / 'test/gate/Fixture.cpp').touch()
            for name in ('door-reproduces.sh', 'one-definition.sh'):
                (root / 'test' / name).write_text('exit 0\n')
            (root / 'test/toolchain.py').write_text('raise SystemExit(0)\n')
            command = ['sh', str(root / 'test/run.sh')]
            env = dict(os.environ, B=str(root / 'build'))
            result = subprocess.run(command, env=env, capture_output=True, text=True)
            self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
            self.assertIn('4 case(s), 1 red', result.stdout)
            binary = root / 'build/gate_Fixture'
            binary.write_text('#!/bin/sh\nexit 0\n')
            binary.chmod(0o755)
            result = subprocess.run(command, env=env, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('4 case(s), 0 red', result.stdout)


class ReproductionGate(unittest.TestCase):
    def test_stale_generated_file_is_red_without_mutating_the_source(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for directory in ('test', 'scripts', 'include', 'src/rt/written'):
                (root / directory).mkdir(parents=True)
            shutil.copyfile(SCRIPT.parents[1] / 'test/door-reproduces.sh', root / 'test/door-reproduces.sh')
            (root / 'include/Builtins.h').write_text('expected header\n')
            (root / 'src/rt/Builtins.cpp').write_text('expected body\n')
            (root / 'src/rt/written/BuiltinsWritten.cpp').write_text('void f() {\n  Work();\n}\n}\n')
            generator = root / 'scripts/gen_builtins.py'
            generator.write_text("""import pathlib, sys
out = pathlib.Path(sys.argv[sys.argv.index('--output-root') + 1])
(out / 'include').mkdir()
(out / 'src/rt').mkdir(parents=True)
(out / 'include/Builtins.h').write_text('expected header\\n')
(out / 'src/rt/Builtins.cpp').write_text('expected body\\n')
""")
            command = ['sh', str(root / 'test/door-reproduces.sh')]
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            (root / 'include/Builtins.h').write_text('stale header\n')
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
            self.assertIn('does not reproduce include/Builtins.h', result.stderr)
            self.assertEqual((root / 'include/Builtins.h').read_text(), 'stale header\n')


class SymbolGate(unittest.TestCase):
    def test_nm_failure_is_red(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            (root / 'libagiru_fixture.so').touch()
            nm = root / 'nm'
            nm.write_text('#!/bin/sh\necho symbol-reader-failed >&2\nexit 42\n')
            nm.chmod(0o755)
            result = subprocess.run(['sh', str(SCRIPT.parents[1] / 'test/one-definition.sh')],
                                    env=dict(os.environ, B=str(root),
                                             PATH=str(root) + os.pathsep + os.environ['PATH']),
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
            self.assertIn('symbol-reader-failed', result.stderr)


class MilestoneGate(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / 'scripts').mkdir()
        (self.root / 'build').mkdir()
        tests = self.root / 'al/Layers/W1/Tests'
        tests.mkdir(parents=True)
        tests.joinpath('Example.al').write_text('''codeunit 50100 "Example UT" {
            Subtype = Test;
            // [Test] procedure NotATest() begin end;
            [Test] procedure First() begin end;
            [Test] procedure Second() begin end;
        }''')
        repo = SCRIPT.parents[1]
        for name in ('ut-milestone.sh', 'ut_milestone.py', 'ut_manifest.py', 'ut_results.py'):
            shutil.copyfile(repo / 'scripts' / name, self.root / 'scripts' / name)
        self.runner = self.root / 'build/agiru'
        self.psql = self.root / 'build/psql'
        self.psql.write_text('#!/bin/sh\nexit 0\n')
        self.psql.chmod(0o755)
        self.ninja = self.root / 'build/ninja'
        self.ninja.write_text('#!/bin/sh\necho "ninja: no work to do."\n')
        self.ninja.chmod(0o755)
        self.env = dict(os.environ, AGIRU_BC_SOURCE=str(self.root / 'al'),
                        PATH=str(self.root / 'build') + os.pathsep + os.environ['PATH'])
        self.command = ['sh', str(self.root / 'scripts/ut-milestone.sh'),
                        str(self.root / 'build/ut.log'), '1']

    def runner_output(self, summary, status=0, delay=0):
        rows = [dict(schema=1, codeunit_id=50100, codeunit='Example UT',
                     method=name, passed=True, error='') for name in ('First', 'Second')]
        self.runner.write_text('#!/usr/bin/env python3\nimport json, os, pathlib, sys, time\n'
                               + f'rows = {rows!r}\n'
                               + f"pathlib.Path({str(self.root / 'build/runner.pid')!r}).write_text(str(os.getpid()))\n"
                               + f'time.sleep({delay})\n'
                               + "path = pathlib.Path(sys.argv[sys.argv.index('--results-jsonl') + 1])\n"
                               + "path.write_text(''.join(json.dumps(row) + '\\n' for row in rows))\n"
                               + f'print({summary!r})\nraise SystemExit({status})\n')
        self.runner.chmod(0o755)

    def test_runner_total_cannot_shrink_source_population(self):
        self.runner_output('1 of 1 passed')
        result = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('of 2 over 1 codeunits, 1 incomplete', result.stdout)
        self.runner_output('2 of 2 passed')
        result = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.runner_output('2 of 2 passed', 42)
        result = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)

    def test_timeout_retains_logs_and_marks_missing_methods(self):
        self.runner_output('2 of 2 passed', delay=20)
        env = dict(self.env, AGIRU_UT_TIMEOUT_SECONDS='1')
        started = time.monotonic()
        result = subprocess.run(self.command, env=env, capture_output=True, text=True, timeout=8)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertLess(time.monotonic() - started, 8)
        self.assertIn('timed out', (self.root / 'build/ut.log').read_text())
        self.assertEqual(len(list((self.root / 'build').glob('ut.log.parts.*/50100.log'))), 1)
        rows = (self.root / 'build/ut.log.results.jsonl').read_text().splitlines()
        self.assertEqual([json.loads(row)['status'] for row in rows], ['missing', 'missing'])

    def test_interrupt_terminates_runner_and_retains_logs(self):
        self.runner_output('2 of 2 passed', delay=20)
        process = subprocess.Popen(self.command, env=self.env, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, text=True)
        pid_file = self.root / 'build/runner.pid'
        deadline = time.monotonic() + 5
        while not pid_file.exists() and time.monotonic() < deadline:
            time.sleep(0.05)
        self.assertTrue(pid_file.exists(), 'fixture runner did not start')
        child = int(pid_file.read_text())
        process.send_signal(signal.SIGINT)
        stdout, stderr = process.communicate(timeout=8)
        self.assertEqual(process.returncode, 1, stdout + stderr)
        with self.assertRaises(ProcessLookupError):
            os.kill(child, 0)
        self.assertIn('interrupted', (self.root / 'build/ut.log').read_text())
        self.assertEqual(len(list((self.root / 'build').glob('ut.log.parts.*/50100.log'))), 1)

    def test_cleanup_failure_is_red(self):
        self.runner_output('2 of 2 passed')
        self.psql.write_text('#!/bin/sh\nexit 42\n')
        result = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('cleanup failed', (self.root / 'build/ut.log').read_text())
        self.assertEqual(json.loads((self.root / 'build/ut.log.run.json').read_text())['status'], 1)

    def test_stale_linked_image_refuses_the_run(self):
        self.runner_output('2 of 2 passed')
        self.ninja.write_text('#!/bin/sh\necho "[1/1] Linking agiru"\n')
        result = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
        self.assertIn('linked library is stale', result.stderr)
        self.assertFalse((self.root / 'build/runner.pid').exists())


class ManifestGate(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        spec = importlib.util.spec_from_file_location(
            'ut_manifest', SCRIPT.parents[1] / 'scripts/ut_manifest.py')
        self.manifest = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.manifest)

    def write(self, filename, contents):
        (self.root / filename).write_text(contents)

    def test_multiline_attribute_and_quoted_method_are_counted(self):
        self.write('Example.al', '''codeunit 50100 "Example UT"
{
    Subtype = Test;
    // [Test] procedure Ghost() begin end;
    [Test]
    [HandlerFunctions('Confirm Handler')]
    local procedure "Quoted Name"()
    begin
    end;
    [Test] procedure Second() begin end;
}''')
        entries = self.manifest.scan(self.root)
        self.assertEqual(entries[0]['methods'], ['Quoted Name', 'Second'])

    def test_unmatched_test_attribute_is_red(self):
        self.write('Example.al', '''codeunit 50100 "Example UT" {
    Subtype = Test;
    [Test] procedure First() begin end;
    [Test] procedure NotParsed;
}''')
        with self.assertRaisesRegex(ValueError, 'cannot reconcile 2 Test attributes'):
            self.manifest.scan(self.root)

    def test_duplicate_codeunit_id_is_red(self):
        for name in ('First', 'Second'):
            self.write(name + '.al', f'''codeunit 50100 "{name} UT" {{
    Subtype = Test;
    [Test] procedure Case() begin end;
}}''')
        with self.assertRaisesRegex(ValueError, 'IDs and names must be unique'):
            self.manifest.scan(self.root)

    def test_duplicate_method_is_red(self):
        self.write('Example.al', '''codeunit 50100 "Example UT" {
    Subtype = Test;
    [Test] procedure Duplicate() begin end;
    [Test] procedure Duplicate() begin end;
}''')
        with self.assertRaisesRegex(ValueError, 'unique procedures'):
            self.manifest.scan(self.root)

    def test_zero_population_is_red(self):
        self.write('Example.al', '''codeunit 50100 "Example" {
    Subtype = Test;
    [Test] procedure Case() begin end;
}''')
        with self.assertRaisesRegex(ValueError, 'no UT codeunits'):
            self.manifest.scan(self.root)


class ResultIdentityGate(unittest.TestCase):
    def setUp(self):
        spec = importlib.util.spec_from_file_location('ut_results', SCRIPT.parents[1] / 'scripts/ut_results.py')
        self.results = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.results)
        self.entry = dict(id=50100, name='Example UT', methods=['First', 'Second'])

    def row(self, name):
        return json.dumps(dict(schema=1, codeunit_id=50100, codeunit='Example UT',
                               method=name, passed=True, error='')) + '\n'

    def test_duplicate_cannot_replace_missing_method(self):
        methods, errors = self.results.reconcile(self.entry, self.row('First') * 2, '2 of 2 passed', 0)
        self.assertIn('duplicate method First', errors)
        self.assertIn('missing method Second', errors)
        self.assertEqual(sum(row['passed'] for row in methods), 0)

    def test_unexpected_identity_is_red(self):
        _, errors = self.results.reconcile(self.entry, self.row('First') + self.row('Unrelated'),
                                           '2 of 2 passed', 0)
        self.assertTrue(any('unexpected' in error for error in errors))
        self.assertIn('missing method Second', errors)

    def test_timeout_keeps_completed_results_and_marks_missing_method(self):
        methods, errors = self.results.reconcile(self.entry, self.row('First'), '', 124)
        self.assertTrue(methods[0]['passed'])
        self.assertEqual(methods[1]['status'], 'missing')
        self.assertTrue(any('process exit 124' in error for error in errors))

    def test_invalid_json_cannot_become_a_result(self):
        _, errors = self.results.reconcile(self.entry, '{broken', '2 of 2 passed', 0)
        self.assertTrue(any('invalid JSON' in error for error in errors))

    def test_names_with_the_same_sanitized_filename_keep_distinct_artifacts(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            names = ('A/B UT', 'A?B UT')
            manifest = []
            for identifier, name in enumerate(names, 50100):
                entry = dict(id=identifier, name=name, methods=['Case'])
                manifest.append(entry)
                stem = root / str(identifier)
                stem.with_suffix('.jsonl').write_text(json.dumps(dict(
                    schema=1, codeunit_id=identifier, codeunit=name,
                    method='Case', passed=True, error='')) + '\n')
                stem.with_suffix('.log').write_text('1 of 1 passed\n')
                stem.with_suffix('.status').write_text('0\n')
            output = root / 'summary.log'
            self.assertEqual(self.results.aggregate(manifest, root, output, 1, 0), 0)
            results = [json.loads(line) for line in
                       (root / 'summary.log.results.jsonl').read_text().splitlines()]
            self.assertEqual([row['codeunit'] for row in results], list(names))


if __name__ == '__main__':
    unittest.main()
