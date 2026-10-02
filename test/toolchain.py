#!/usr/bin/env python3
"""Negative controls for analysis selection and tool exit handling."""
import importlib.util
import io
import json
import os
import shutil
import signal
import time
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from contextlib import redirect_stderr, redirect_stdout
from unittest.mock import patch

SCRIPT = Path(__file__).with_name('lint-analysis.py').resolve()
spec = importlib.util.spec_from_file_location('lint_analysis', SCRIPT)
analysis = importlib.util.module_from_spec(spec)
spec.loader.exec_module(analysis)
verify_spec = importlib.util.spec_from_file_location(
    'verify_snapshot', Path(__file__).resolve().parents[1] / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(verify_spec)
verify_spec.loader.exec_module(verify)
seed_spec = importlib.util.spec_from_file_location(
    'seed_demo', Path(__file__).resolve().parents[1] / 'scripts/seed_demo.py')
seed = importlib.util.module_from_spec(seed_spec)
seed_spec.loader.exec_module(seed)
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
milestone_spec = importlib.util.spec_from_file_location(
    'ut_milestone', Path(__file__).resolve().parents[1] / 'scripts/ut_milestone.py')
milestone = importlib.util.module_from_spec(milestone_spec)
milestone_spec.loader.exec_module(milestone)
unity_spec = importlib.util.spec_from_file_location(
    'unity_groups', Path(__file__).resolve().parents[1] / 'scripts/unity_groups.py')
unity = importlib.util.module_from_spec(unity_spec)
unity_spec.loader.exec_module(unity)


@unittest.skipUnless(shutil.which('clang++'), 'tree gate requires clang++')
class TreeSyntaxGate(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        for name in ('scripts', 'cmake', 'include', 'apps/Fixture'):
            (self.root / name).mkdir(parents=True)
        repository = SCRIPT.parents[1]
        for name in ('tree_syntax.sh', 'tree_keys.py'):
            shutil.copy2(repository / 'scripts' / name, self.root / 'scripts' / name)
        (self.root / 'cmake/Precompiled.h').write_text('// isolated tree gate fixture\n')

    def run_tree(self, path='apps'):
        environment = dict(os.environ, JOBS='2', AGIRU_TREE_CACHE='build/tree-cache')
        return subprocess.run(['sh', 'scripts/tree_syntax.sh', path], cwd=self.root,
                              env=environment, text=True, capture_output=True, timeout=30)

    def test_valid_header_passes(self):
        (self.root / 'apps/Fixture/Valid.h').write_text('#pragma once\nstruct Valid {};\n')
        result = self.run_tree()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('1 of 1 generated headers compile', result.stdout)

    def test_invalid_header_and_cached_failure_are_red(self):
        (self.root / 'apps/Fixture/Invalid.h').write_text('#pragma once\nstruct Invalid {\n')
        for _ in range(2):
            result = self.run_tree()
            self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
            self.assertIn('0 of 1 generated headers compile', result.stdout)
            self.assertEqual(len((self.root / 'build/tree-syntax/failed')
                                 .read_text().splitlines()), 1)
            self.assertIn('expected', (self.root / 'build/tree-syntax/roots').read_text())

    def test_warning_only_header_is_red(self):
        (self.root / 'apps/Fixture/Warning.h').write_text('''#pragma once
[[deprecated]] inline void old_api() {}
inline void consumer() { old_api(); }
''')
        result = self.run_tree()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('-Werror', (self.root / 'build/tree-syntax/errors').read_text())

    def test_empty_population_is_red(self):
        result = self.run_tree()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('no generated headers', result.stderr)

    def test_permissive_cached_pass_cannot_hide_strict_failure(self):
        (self.root / 'apps/Fixture/Warning.h').write_text('''#pragma once
[[deprecated]] inline void old_api() {}
inline void consumer() { old_api(); }
''')
        script = self.root / 'scripts/tree_syntax.sh'
        strict = script.read_text()
        script.write_text(strict.replace(' -Werror', ''))
        result = self.run_tree()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        script.write_text(strict)
        result = self.run_tree()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('0 of 1 generated headers compile', result.stdout)

    def test_missing_population_is_red(self):
        result = self.run_tree('missing-apps')
        self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
        self.assertIn('does not exist', result.stderr)


class SeedTransferGate(unittest.TestCase):
    def test_provenance_records_artefact_source_and_schema_hashes(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'scripts').mkdir()
            (root / 'work').mkdir()
            (root / 'BC_VERSION').write_text('28.4-fixture\n')
            (root / 'scope.json').write_text('{}\n')
            (root / 'work/bc-28.4-fixture-w1.zip').write_bytes(b'fixture archive')
            with patch.object(seed, '__file__', str(root / 'scripts/seed_demo.py')), \
                    patch.object(seed.subprocess, 'run', return_value=SimpleNamespace(
                        returncode=0, stdout=f'{root / "bc"}\nbc-fixture-revision\n')), \
                    patch.dict(os.environ, AGIRU_BC_SOURCE=str(root / 'bc')):
                details = seed.provenance('CRONUS', {'A': ['id']}, {'A': ['id']}, 'target')
            self.assertEqual(details['artefact_version'], '28.4-fixture')
            self.assertEqual(details['bc_source_revision'], 'bc-fixture-revision')
            self.assertEqual(len(details['artefact_sha256']), 64)
            self.assertEqual(len(details['scope_sha256']), 64)
            self.assertEqual(len(details['target_schema_sha256']), 64)
            self.assertTrue(details['id'])
            with patch.object(seed, '__file__', str(root / 'scripts/seed_demo.py')), \
                    patch.object(seed.subprocess, 'run', return_value=SimpleNamespace(
                        returncode=0, stdout=f'{root}\nagiru-fixture-revision\n')), \
                    patch.dict(os.environ, AGIRU_BC_SOURCE=str(root / 'frozen-bc')), \
                    redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit):
                    seed.provenance('CRONUS', {'A': ['id']}, {'A': ['id']}, 'target')

    def test_incomplete_seed_identity_is_rejected_and_complete_identity_is_read(self):
        def reply(output):
            return SimpleNamespace(returncode=0, stdout=output, stderr='')
        with patch.object(milestone.subprocess, 'run', side_effect=[
                reply('agiru_seed_provenance\n'), reply('building\t{"id":"fixture"}\n')]):
            with self.assertRaisesRegex(ValueError, 'not complete'):
                milestone.seed_identity('fixture-dsn')
        with patch.object(milestone.subprocess, 'run', side_effect=[
                reply('agiru_seed_provenance\n'), reply('complete\t{"id":"fixture"}\n')]):
            self.assertEqual(milestone.seed_identity('fixture-dsn'), {'id': 'fixture'})

    def test_reader_failure_cannot_be_hidden_by_successful_writer(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            fake = root / 'podman'
            fake.write_text('''#!/bin/sh
case "$*" in
  *" -d cronus "*)
    if [ "$FAKE_SOURCE_FAIL" = 1 ]; then
      printf 'ERROR: source refused\\n' >&2
      exit 17
    fi
    printf 'row\\n'
    ;;
  *" -d test_seed "*)
    input=$(cat)
    if [ -z "$input" ]; then printf 'COPY 0\\n'; else printf 'COPY 1\\n'; fi
    ;;
esac
''')
            fake.chmod(0o755)
            with patch.dict(os.environ, PATH=f'{root}:{os.environ["PATH"]}',
                            FAKE_SOURCE_FAIL='1'):
                reader, writer, note = seed.transfer('read', 'write', 'test_seed')
            self.assertEqual((reader, writer), (17, 0))
            self.assertIn('ERROR: source refused', note)
            self.assertIn('COPY 0', note)
            with patch.dict(os.environ, PATH=f'{root}:{os.environ["PATH"]}',
                            FAKE_SOURCE_FAIL='0'):
                reader, writer, note = seed.transfer('read', 'write', 'test_seed')
            self.assertEqual((reader, writer), (0, 0))
            self.assertIn('COPY 1', note)

    def test_one_failed_table_keeps_a_partly_populated_seed_red(self):
        tables = {'A': ['id'], 'B': ['id']}
        output = io.StringIO()
        errors = io.StringIO()
        with patch.object(seed, 'columns', side_effect=[tables, tables]), \
                patch.object(seed, 'transfer', side_effect=[(0, 0, 'COPY 1\n'),
                                                          (17, 0, 'ERROR: source refused\n')]), \
                patch.object(seed, 'provenance', return_value={'id': 'fixture'}), \
                patch.object(seed, 'begin_seed'), \
                patch.object(sys, 'argv', ['seed_demo.py']), \
                redirect_stdout(output), redirect_stderr(errors):
            with self.assertRaises(SystemExit) as refused:
                seed.main()
        self.assertEqual(refused.exception.code, 1)
        self.assertIn('1 table(s) carry 1 row(s)', output.getvalue())
        self.assertIn('the seed is incomplete', errors.getvalue())


class CompilerCacheGate(unittest.TestCase):
    def test_unity_groups_are_bounded_and_stable_when_the_slice_grows(self):
        sources = [f'app/module/source-{number}.cpp' for number in range(160)]
        before = unity.group_sources(sources, root_groups=32, max_group_size=8)
        counts = {}
        for group in before.values():
            counts[group] = counts.get(group, 0) + 1
        self.assertLessEqual(max(counts.values()), 8)

        candidate = next(
            f'app/module/added-{number}.cpp' for number in range(10000)
            if sum(unity.stable_root(source, 32) ==
                   unity.stable_root(f'app/module/added-{number}.cpp', 32)
                   for source in sources) < 8)
        after = unity.group_sources(sources + [candidate], root_groups=32, max_group_size=8)
        self.assertTrue(all(after[source] == before[source] for source in sources))

    def test_only_an_overfull_unity_root_splits(self):
        roots = {0: [], 1: []}
        number = 0
        while len(roots[0]) < 5 or len(roots[1]) < 2:
            source = f'app/module/source-{number}.cpp'
            root = unity.stable_root(source, 2)
            if len(roots[root]) < (5 if root == 0 else 2):
                roots[root].append(source)
            number += 1
        sources = roots[0][:4] + roots[1]
        before = unity.group_sources(sources, root_groups=2, max_group_size=4)
        after = unity.group_sources(sources + [roots[0][4]], root_groups=2,
                                    max_group_size=4)
        self.assertTrue(all(after[source] == before[source] for source in roots[1]))
        counts = {}
        for group in after.values():
            counts[group] = counts.get(group, 0) + 1
        self.assertLessEqual(max(counts.values()), 4)

    def test_adding_a_slice_source_changes_only_its_cmake_unity_file(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'apps/app/module').mkdir(parents=True)
            sources = [f'app/module/source-{number}.cpp' for number in range(80)]
            for number, source in enumerate(sources):
                (root / 'apps' / source).write_text(f'int source_{number}() {{ return {number}; }}\n')
            (root / 'slice').write_text('\n'.join(sources) + '\n')
            script = Path(__file__).resolve().parents[1] / 'scripts/unity_groups.py'
            (root / 'CMakeLists.txt').write_text(f'''cmake_minimum_required(VERSION 3.28)
project(UnityStability CXX)
execute_process(COMMAND python3 "{script}" "${{CMAKE_SOURCE_DIR}}/slice"
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "${{error}}")
endif()
string(REPLACE "\\n" ";" rows "${{output}}")
foreach(row IN LISTS rows)
  if(NOT row MATCHES "^([^|]+)[|](.+)$")
    message(FATAL_ERROR "invalid row")
  endif()
  set(source "${{CMAKE_SOURCE_DIR}}/apps/${{CMAKE_MATCH_1}}")
  list(APPEND sources "${{source}}")
  set_source_files_properties("${{source}}" PROPERTIES UNITY_GROUP "${{CMAKE_MATCH_2}}")
endforeach()
add_library(slice OBJECT ${{sources}})
set_target_properties(slice PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE GROUP)
''')
            build = root / 'build'
            subprocess.run(['cmake', '-S', str(root), '-B', str(build), '-G', 'Ninja'],
                           check=True, capture_output=True)

            def unity_files():
                unity_root = build / 'CMakeFiles/slice.dir/Unity'
                return {path.relative_to(unity_root): path.read_bytes()
                        for path in unity_root.rglob('*.cxx')}

            before = unity_files()
            target_root = unity.stable_root(sources[0])
            candidate = next(
                f'app/module/added-{number}.cpp' for number in range(100000)
                if unity.stable_root(f'app/module/added-{number}.cpp') == target_root)
            (root / 'apps' / candidate).write_text('int added() { return 1; }\n')
            (root / 'slice').write_text(candidate + '\n' + '\n'.join(sources) + '\n')
            subprocess.run(['cmake', '-S', str(root), '-B', str(build), '-G', 'Ninja'],
                           check=True, capture_output=True)
            after = unity_files()
            changed = [path for path, content in before.items() if after[path] != content]
            self.assertEqual(len(changed), 1)
            self.assertEqual(set(before), set(after))

    def test_clang_pch_recompile_hits_the_configured_cache(self):
        repo = Path(__file__).resolve().parents[1]
        commands = json.loads((repo / 'build/compile_commands.json').read_text())
        pch = [row for row in commands
               if 'agiru_slice' in row['command'] and 'cmake_pch.hxx.cxx' in row['file']]
        self.assertEqual(len(pch), 1)
        self.assertIn('-Xclang -fno-pch-timestamp', pch[0]['command'])
        setting = subprocess.run(
            ['make', '-s', '--eval=cache-env:; @printf %s "$$CCACHE_SLOPPINESS"', 'cache-env'],
            cwd=repo, capture_output=True, text=True, check=True).stdout
        self.assertEqual(setting, 'pch_defines,time_macros')
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'Pch.h').write_text('#ifndef PROBE_PCH_H\n#define PROBE_PCH_H\n'
                                        '#include <vector>\n#endif\n')
            (root / 'probe.cpp').write_text('int main() { return 0; }\n')
            (root / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.28)
project(PchCacheProbe CXX)
add_executable(probe probe.cpp)
target_precompile_headers(probe PRIVATE Pch.h)
target_compile_options(probe PRIVATE -Xclang -fno-pch-timestamp)
''')
            env = dict(os.environ, CCACHE_DIR=str(root / 'cache'), CCACHE_SLOPPINESS=setting)
            subprocess.run(['cmake', '-S', str(root), '-B', str(root / 'build'), '-G', 'Ninja',
                            '-DCMAKE_CXX_COMPILER=clang++-19',
                            '-DCMAKE_CXX_COMPILER_LAUNCHER=ccache'], env=env, check=True,
                           capture_output=True)
            subprocess.run(['cmake', '--build', str(root / 'build')], env=env, check=True,
                           capture_output=True)
            (root / 'build/CMakeFiles/probe.dir/probe.cpp.o').unlink()
            subprocess.run(['cmake', '--build', str(root / 'build')], env=env, check=True,
                           capture_output=True)
            stats = json.loads(subprocess.run(['ccache', '--print-stats', '--format=json'],
                                              env=env, check=True, capture_output=True,
                                              text=True).stdout)
            self.assertEqual(stats['could_not_use_precompiled_header'], 0)
            self.assertGreater(stats['direct_cache_hit'] + stats['preprocessed_cache_hit'], 0)

    def test_targeted_lint_refreshes_the_graph_without_building_objects(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            shutil.copyfile(Path(__file__).resolve().parents[1] / 'Makefile', root / 'Makefile')
            build = root / 'build'
            build.mkdir()
            compiler = shutil.which('sh')
            (build / 'CMakeCache.txt').write_text(f'CMAKE_CXX_COMPILER:STRING={compiler}\n')
            cmake = root / 'cmake'
            cmake.write_text('#!/bin/sh\nprintf "%s\\n" "$@" >> cmake-args\n')
            cmake.chmod(0o755)
            python = root / 'python3'
            python.write_text('#!/bin/sh\ncase "$1" in *lint-analysis.py) '
                              'printf "%s" "$AGIRU_LINT_UNIT" > analysed;; esac\n')
            python.chmod(0o755)
            env = dict(os.environ, PATH=f'{root}:{os.environ["PATH"]}')
            result = subprocess.run(['make', '--no-print-directory', 'lint-one',
                                     'UNIT=src/rt/RecordRef.cpp', f'CXX={compiler}', f'B={build}'],
                                    cwd=root, env=env, capture_output=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual((root / 'cmake-args').read_text().splitlines(),
                             ['--build', str(build), '--target', 'build.ninja'])
            self.assertEqual((root / 'analysed').read_text(), 'src/rt/RecordRef.cpp')

    def test_single_gate_preserves_build_and_test_failures(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            shutil.copyfile(Path(__file__).resolve().parents[1] / 'Makefile', root / 'Makefile')
            (root / 'test/gate').mkdir(parents=True)
            (root / 'test/gate/ProbeGate.cpp').write_text('')
            build = root / 'build'
            build.mkdir()
            cmake = root / 'cmake'
            cmake.write_text('#!/bin/sh\nexit 23\n')
            cmake.chmod(0o755)
            binary = build / 'gate_ProbeGate'
            marker = root / 'ran'
            binary.write_text('#!/bin/sh\ntouch ran\nexit 17\n')
            binary.chmod(0o755)
            env = dict(os.environ, PATH=f'{root}:{os.environ["PATH"]}')
            command = ['make', '--no-print-directory', '-o', 'comments', '-o', 'db',
                       'gate', 'GATE=ProbeGate', f'B={build}']
            failed_build = subprocess.run(command, cwd=root, env=env, capture_output=True)
            self.assertNotEqual(failed_build.returncode, 0)
            self.assertFalse(marker.exists())
            cmake.write_text('#!/bin/sh\nexit 0\n')
            failed_gate = subprocess.run(command, cwd=root, env=env, capture_output=True)
            self.assertNotEqual(failed_gate.returncode, 0)
            self.assertTrue(marker.exists())
            binary.write_text('#!/bin/sh\nexit 0\n')
            passed = subprocess.run(command, cwd=root, env=env, capture_output=True)
            self.assertEqual(passed.returncode, 0, passed.stderr)

    def test_compiler_names_resolve_through_path_and_mismatches_still_fail(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            shutil.copyfile(Path(__file__).resolve().parents[1] / 'Makefile', root / 'Makefile')
            build = root / 'build'
            build.mkdir()
            for name in ('fixture-cxx', 'other-cxx'):
                executable = root / name
                executable.write_text('#!/bin/sh\nexit 0\n')
                executable.chmod(0o755)
            (build / 'CMakeCache.txt').write_text('CMAKE_CXX_COMPILER:STRING=fixture-cxx\n')
            cmake = root / 'cmake'
            cmake.write_text('#!/bin/sh\nexit 0\n')
            cmake.chmod(0o755)
            env = dict(os.environ, PATH=f'{root}:{os.environ["PATH"]}')
            command = ['make', '--no-print-directory', 'db', f'B={build}']
            same = subprocess.run(command + [f'CXX={root / "fixture-cxx"}'], cwd=root,
                                  env=env, capture_output=True, text=True)
            self.assertEqual(same.returncode, 0, same.stderr)
            different = subprocess.run(command + ['CXX=other-cxx'], cwd=root,
                                       env=env, capture_output=True, text=True)
            self.assertNotEqual(different.returncode, 0)
            self.assertIn('Compiler mismatch', different.stderr)


class SnapshotGate(unittest.TestCase):
    def test_frozen_bc_revision_reaches_the_ut_runner(self):
        with tempfile.TemporaryDirectory() as folder:
            base = Path(folder)
            root = base / 'project'
            root.mkdir()
            (root / 'Makefile').write_text('ut:\n'
                                           '\t@mkdir -p "$(dir $(UT_LOG))"\n'
                                           '\t@printf "%s" "$$AGIRU_BC_REVISION" > "$(UT_LOG)"\n')
            bc = base / 'BCApps'
            (bc / 'src').mkdir(parents=True)
            (bc / 'src/Fixture.al').write_text('fixture\n')
            for repo in (root, bc):
                subprocess.run(['git', '-C', str(repo), 'init', '-q'], check=True)
                subprocess.run(['git', '-C', str(repo), 'add', '.'], check=True)
                subprocess.run(['git', '-C', str(repo), '-c', 'user.name=Gate',
                                '-c', 'user.email=gate@example.invalid', 'commit', '-qm',
                                'fixture'], check=True)
            revision = subprocess.check_output(['git', '-C', str(bc), 'rev-parse', 'HEAD'],
                                               text=True).strip()
            arguments = SimpleNamespace(targets=['ut'], jobs=1, detach=False, reuse=False)
            with patch.object(verify, 'ROOT', root), \
                    patch.dict(os.environ, {'AGIRU_BC_SOURCE': str(bc / 'src'),
                                            'AGIRU_BC_REVISION': 'stale-parent-value'}):
                run = verify.start(arguments)
            self.assertEqual(run, 0)
            archive = Path((root / 'build/verify/latest').read_text().strip())
            result = json.loads((archive / 'result.json').read_text())
            self.assertEqual(result['bc_source_revision'], revision)
            self.assertEqual((archive / 'artifacts/ut.log').read_text(), revision)

    def test_reused_lane_keeps_each_runs_ut_artifacts(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            lane = root / 'lane'
            lane.mkdir()
            (lane / 'Makefile').write_text('UT_LOG ?= build/ut.log\nut:\n'
                                         '\t@mkdir -p "$(dir $(UT_LOG))"\n'
                                         '\t@printf "%s\\n" "$(UT_LOG)" > "$(UT_LOG)"\n')
            originals = []
            for name in ('first', 'second'):
                run = root / name
                run.mkdir()
                (run / 'result.json').write_text(json.dumps({
                    'status': 'queued', 'targets': ['ut'], 'jobs': 1,
                    'build_source': str(lane), 'source_sha256': verify.digest(lane)}))
                self.assertEqual(verify.run_snapshot(run), 0)
                artifact = run / 'artifacts/ut.log'
                self.assertTrue(artifact.is_file(), 'results must belong to the run, not the lane')
                originals.append((artifact, artifact.read_text()))
            (lane / 'build').mkdir(exist_ok=True)
            (lane / 'build/ut.log').write_text('unrelated later result')
            for artifact, original in originals:
                self.assertEqual(artifact.read_text(), original)

    def test_fast_detached_child_keeps_its_completed_result(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'Makefile').write_text('help:\n\t@true\n')
            for arguments in (['init', '-q'], ['add', 'Makefile'],
                              ['-c', 'user.name=Gate', '-c', 'user.email=gate@example.invalid',
                               'commit', '-qm', 'fixture']):
                subprocess.run(['git', '-C', str(root), *arguments], check=True,
                               capture_output=True)
            original = subprocess.Popen

            def launch(command, **kwargs):
                if command[0] == sys.executable and command[2] == 'run':
                    verify.run_snapshot(Path(command[3]))
                    return SimpleNamespace(pid=os.getpid())
                return original(command, **kwargs)

            arguments = SimpleNamespace(targets=['help'], jobs=1, detach=True, reuse=False)
            with patch.object(verify, 'ROOT', root), \
                    patch.object(subprocess, 'Popen', side_effect=launch):
                self.assertEqual(verify.start(arguments), 0)
            run = Path((root / 'build/verify/latest').read_text().strip())
            result = json.loads((run / 'result.json').read_text())
            self.assertEqual(result['status'], 'passed')
            self.assertEqual(result['pid'], os.getpid())
            self.assertEqual(result['target_exits'], {'help': 0})

    def test_running_lane_refuses_a_second_start(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            lane = root / 'lane'
            active = root / 'active'
            lane.mkdir()
            active.mkdir()
            (active / 'result.json').write_text('{"status":"running"}')
            (lane / 'latest').write_text(str(active))
            with self.assertRaisesRegex(RuntimeError, 'still running'):
                verify.prepare_lane(root, root, 'unused', 'unused')

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
            (lane / '__pycache__').mkdir()
            (lane / '__pycache__/stale.pyc').write_bytes(b'stale')
            original = (lane / 'same.h').stat().st_mtime_ns
            verify.sync_source(source, lane)
            self.assertEqual((lane / 'same.h').stat().st_mtime_ns, original)
            self.assertEqual((lane / 'changed.cpp').read_text(), 'new')
            self.assertFalse((lane / 'obsolete.cpp').exists())
            self.assertTrue((lane / 'build/retained.o').exists())
            self.assertFalse((lane / '__pycache__').exists())
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
        (self.root / 'build/build.ninja').write_text('')
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

    def test_frozen_bc_revision_is_not_the_parent_project_revision(self):
        self.runner_output('2 of 2 passed')
        env = dict(self.env, AGIRU_BC_REVISION='bc-fixture-revision')
        result = subprocess.run(self.command, env=env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        metadata = json.loads((self.root / 'build/ut.log.run.json').read_text())
        self.assertEqual(metadata['source_revision'], 'bc-fixture-revision')

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
        (self.root / 'al/Layers/W1/Tests/Queued.al').write_text('''codeunit 50101 "Queued UT" {
            Subtype = Test;
            [Test] procedure NeverStarted() begin end;
        }''')
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
        parts = list((self.root / 'build').glob('ut.log.parts.*'))
        self.assertEqual(len(parts), 1)
        self.assertEqual((parts[0] / '50100.status').read_text().strip(), '130')
        self.assertEqual((parts[0] / '50101.status').read_text().strip(), '-1')

    def test_cleanup_failure_is_red(self):
        self.runner_output('2 of 2 passed')
        self.psql.write_text('#!/bin/sh\ncase "$*" in *"DROP DATABASE"*) exit 42;; esac\nexit 0\n')
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
        self.assert_preflight_population(result, 'linked library is stale')

    def assert_preflight_population(self, result, reason, status=2):
        self.assertEqual(result.returncode, status, result.stdout + result.stderr)
        self.assertIn('0 of 2 over 1 codeunits, 1 incomplete', result.stdout)
        output = self.root / 'build/ut.log'
        self.assertIn(reason, output.read_text())
        manifest = json.loads(Path(str(output) + '.manifest.json').read_text())
        self.assertEqual(manifest[0]['methods'], ['First', 'Second'])
        rows = [json.loads(line) for line in
                Path(str(output) + '.results.jsonl').read_text().splitlines()]
        self.assertEqual([row['method'] for row in rows], ['First', 'Second'])
        self.assertEqual([row['status'] for row in rows], ['missing', 'missing'])
        metadata = json.loads(Path(str(output) + '.run.json').read_text())
        self.assertEqual(metadata['status'], status)
        self.assertIn(reason, metadata['infrastructure_errors'][0])
        self.assertFalse((self.root / 'build/runner.pid').exists())

    def test_absent_image_keeps_the_source_population(self):
        result = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assert_preflight_population(result, 'No such file')

    def test_incomplete_seed_keeps_the_source_population(self):
        self.runner_output('2 of 2 passed')
        self.psql.write_text('''#!/bin/sh
case "$*" in
  *to_regclass*) printf 'agiru_seed_provenance\\n';;
  *"SELECT status"*) printf 'building\\t{"id":"fixture"}\\n';;
esac
''')
        result = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assert_preflight_population(result, 'seed provenance is not complete')

    def test_failed_build_keeps_the_source_population(self):
        self.runner_output('2 of 2 passed')
        make = self.root / 'build/make'
        make.write_text('#!/bin/sh\nprintf "fixture build failed\\n"\nexit 19\n')
        make.chmod(0o755)
        shutil.copyfile(SCRIPT.parents[1] / 'Makefile', self.root / 'Makefile')
        result = subprocess.run([shutil.which('make'), '-C', str(self.root), 'ut', 'JOBS=1'],
                                env=self.env, capture_output=True, text=True)
        self.assert_preflight_population(result, 'build exited 19')

    def test_successful_build_runs_the_same_source_population(self):
        self.runner_output('2 of 2 passed')
        make = self.root / 'build/make'
        make.write_text('#!/bin/sh\nexit 0\n')
        make.chmod(0o755)
        result = subprocess.run(self.command + ['--build'], env=self.env,
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('2 of 2 over 1 codeunits, 0 incomplete', result.stdout)

    def test_interrupt_during_build_retains_population_and_stops_children(self):
        self.runner_output('2 of 2 passed')
        make = self.root / 'build/make'
        make.write_text('#!/usr/bin/env python3\nimport os, pathlib, subprocess, sys, time\n'
                        + f'root = pathlib.Path({str(self.root / "build")!r})\n'
                        + 'child = subprocess.Popen([sys.executable, "-c", "import time; time.sleep(60)"])\n'
                        + '(root / "build-child.pid").write_text(str(child.pid))\n'
                        + '(root / "build.pid").write_text(str(os.getpid()))\n'
                        + 'time.sleep(60)\n')
        make.chmod(0o755)
        process = subprocess.Popen(self.command + ['--build'], env=self.env,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        def cleanup():
            if process.poll() is None:
                process.kill()
                process.wait()
            for name in ('build.pid', 'build-child.pid'):
                path = self.root / 'build' / name
                if path.exists():
                    try:
                        os.kill(int(path.read_text()), signal.SIGTERM)
                    except ProcessLookupError:
                        pass
            for stream in (process.stdout, process.stderr):
                stream.close()
        self.addCleanup(cleanup)
        pid_file = self.root / 'build/build.pid'
        deadline = time.monotonic() + 5
        while not pid_file.exists() and time.monotonic() < deadline:
            time.sleep(0.05)
        self.assertTrue(pid_file.exists(), 'fixture build did not start')
        process.send_signal(signal.SIGTERM)
        stdout, stderr = process.communicate(timeout=8)
        self.assert_preflight_population(SimpleNamespace(
            returncode=process.returncode, stdout=stdout, stderr=stderr), 'build interrupted', 130)
        with self.assertRaises(ProcessLookupError):
            os.kill(int(pid_file.read_text()), 0)
        child = int((self.root / 'build/build-child.pid').read_text())
        stat = Path(f'/proc/{child}/stat')
        try:
            state = stat.read_text().split()[2]
        except FileNotFoundError:
            state = None
        self.assertIn(state, (None, 'Z'), 'the interrupted build left an executing child')

    def test_cmake_glob_checks_do_not_hide_image_staleness(self):
        real_ninja = shutil.which('ninja')
        self.ninja.unlink()
        self.ninja.symlink_to(real_ninja)
        dependency = self.root / 'input.txt'
        dependency.write_text('original')
        (self.root / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.20)
project(ImageProbe NONE)
file(GLOB inputs CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/*.txt")
add_custom_command(OUTPUT image
  COMMAND ${CMAKE_COMMAND} -E touch image
  DEPENDS ${inputs})
add_custom_target(agiru DEPENDS image)
''')
        subprocess.run(['cmake', '-S', str(self.root), '-B', str(self.root / 'build'),
                        '-G', 'Ninja', f'-DCMAKE_MAKE_PROGRAM={real_ninja}'],
                       check=True, capture_output=True)
        subprocess.run([real_ninja, '-C', str(self.root / 'build'), 'agiru'],
                       check=True, capture_output=True)
        self.runner_output('2 of 2 passed')
        current = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assertEqual(current.returncode, 0, current.stdout + current.stderr)
        dependency.write_text('changed')
        stale = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assertEqual(stale.returncode, 2, stale.stdout + stale.stderr)
        self.assertIn('linked library is stale', stale.stderr)


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

    def test_internal_test_procedure_is_counted(self):
        self.write('Example.al', '''codeunit 50100 "Example UT" {
    Subtype = Test;
    [Test] internal procedure InternalCase() begin end;
}''')
        self.assertEqual(self.manifest.scan(self.root)[0]['methods'], ['InternalCase'])

    def test_utf16_source_is_counted_in_both_byte_orders(self):
        for encoding in ('utf-16', 'utf-16-be'):
            with self.subTest(encoding=encoding):
                source = '''codeunit 50100 "Example UT" {
    Subtype = Test;
    [Test] procedure UnicodeCase() begin end;
}'''
                if encoding == 'utf-16-be':
                    source = '\ufeff' + source
                (self.root / 'Example.al').write_text(source, encoding=encoding)
                self.assertEqual(self.manifest.scan(self.root)[0]['methods'], ['UnicodeCase'])

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
