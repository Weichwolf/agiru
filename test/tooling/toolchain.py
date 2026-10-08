#!/usr/bin/env python3
"""Negative controls for analysis selection and tool exit handling."""
import importlib.util
import hashlib
import io
import json
import os
import re
import shlex
import shutil
import signal
import stat
import time
from pathlib import Path
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
import zipfile
from contextlib import redirect_stderr, redirect_stdout
from unittest.mock import patch

SCRIPT = Path(__file__).with_name('lint-analysis.py').resolve()
spec = importlib.util.spec_from_file_location('lint_analysis', SCRIPT)
analysis = importlib.util.module_from_spec(spec)
spec.loader.exec_module(analysis)
verify_spec = importlib.util.spec_from_file_location(
    'verify_snapshot', Path(__file__).resolve().parents[2] / 'scripts/verify_snapshot.py')
verify = importlib.util.module_from_spec(verify_spec)
verify_spec.loader.exec_module(verify)
seed_spec = importlib.util.spec_from_file_location(
    'seed_demo', Path(__file__).resolve().parents[2] / 'scripts/seed_demo.py')
seed = importlib.util.module_from_spec(seed_spec)
seed_spec.loader.exec_module(seed)
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / 'scripts'))
milestone_spec = importlib.util.spec_from_file_location(
    'ut_milestone', Path(__file__).resolve().parents[2] / 'scripts/ut_milestone.py')
milestone = importlib.util.module_from_spec(milestone_spec)
milestone_spec.loader.exec_module(milestone)
unity_spec = importlib.util.spec_from_file_location(
    'unity_groups', Path(__file__).resolve().parents[2] / 'scripts/unity_groups.py')
unity = importlib.util.module_from_spec(unity_spec)
unity_spec.loader.exec_module(unity)
symbols_spec = importlib.util.spec_from_file_location(
    'fetch_symbols', Path(__file__).resolve().parents[2] / 'scripts/fetch_symbols.py')
symbols = importlib.util.module_from_spec(symbols_spec)
symbols_spec.loader.exec_module(symbols)
inventory_spec = importlib.util.spec_from_file_location(
    'scope_inventory', Path(__file__).resolve().parents[2] / 'scripts/scope_inventory.py')
scope_inventory = importlib.util.module_from_spec(inventory_spec)
inventory_spec.loader.exec_module(scope_inventory)


def isolated_make_environment():
    environment = dict(os.environ)
    for name in ('MAKEFLAGS', 'MFLAGS', 'MAKEOVERRIDES', 'B', 'UT_LOG'):
        environment.pop(name, None)
    return environment


class NativeSourceCompilerGate(unittest.TestCase):
    def setUp(self):
        self.repository = Path(__file__).resolve().parents[2]
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.source = self.root / 'source'
        self.source.mkdir()
        self.package = self.root / 'native'
        (self.package / 'src').mkdir(parents=True)
        self.native = self.package / 'src/unusual-name.aL'
        shutil.copyfile(self.repository / 'test/transpiler/platform-source/PageTableField.Table.al', self.native)
        self.apps = self.root / 'apps.json'
        self.apps.write_text(json.dumps({'apps': [{'name': 'fixture', 'source': 'source'}]}))
        (self.root / 'scope.json').write_text(json.dumps({'include': ['System', 'Microsoft'], 'exclude': []}))
        original = Path(os.environ.get('AGIRU_BC_SOURCE', str(Path.home() / 'Git/BCApps/src'))) / \
            'Layers/W1/BaseApp/Modules/System/PageDesigner/PageFieldsSelectionList.Page.al'
        shutil.copyfile(original, self.source / 'Original.Page.al')
        (self.source / 'Numeric.Page.al').write_text('namespace System.Tooling;\n'
            'page 50171 "Numeric Source" { SourceTable = 2000000171; '
            'layout { area(Content) { field(Caption; Caption) {} } } '
            'trigger OnOpenPage() begin Rec.SetRange(Type, Rec.Type::Integer); end; }')
        (self.source / 'Population.Codeunit.al').write_text('namespace Microsoft.Fixture;\n'
            'codeunit 50172 "Native Source UT"\n{ Subtype = Test; [Test] procedure Kept() begin end; }')
        self.transpiler = (self.repository / os.environ.get('B', 'build') / 'agirutc').resolve()
        self.generated = self.root / 'generated'

    def run_compiler(self, extra=None, output=True):
        arguments = [str(self.transpiler), str(self.root), str(self.apps)]
        if output:
            arguments.append(str(self.generated))
        arguments += ['--system-symbols', str(self.package)] if extra is None else extra
        return subprocess.run(arguments, text=True, capture_output=True, timeout=30)

    def native_table_manifest(self):
        manifest = self.package / 'NavxManifest.xml'
        manifest.write_text('<Package xmlns="http://schemas.microsoft.com/navx/2015/manifest">'
            '<App Id="85a884cd-20d8-4d18-91bd-e6c1baaa3a32" Name="Table Only" '
            'Publisher="agiru tests" Version="1.2.3.4"/></Package>')
        return manifest

    def test_table_only_native_owner_validates_a_legal_field_takeover(self):
        self.native_table_manifest()
        source = self.native.read_text()
        original = 'field(1; "Page ID"; Integer)\n        {\n        }'
        self.assertEqual(source.count(original), 1)
        self.native.write_text(source.replace(original,
            'field(1; "Page ID"; Integer) { ObsoleteState = Moved; '
            "MovedTo = '118874ab-44bc-4ccb-9daf-59763539ab16'; }"))
        (self.source / 'app.json').write_text(json.dumps({
            'id': '118874ab-44bc-4ccb-9daf-59763539ab16', 'name': 'Table Destination',
            'publisher': 'agiru tests', 'version': '1.0.0.0'}))
        (self.source / 'Takeover.TableExt.al').write_text('namespace Microsoft.Fixture; '
            'tableextension 50183 Destination extends "Page Table Field" { fields { '
            'field(1; "Page ID"; Integer) { '
            "MovedFrom = '85a884cd-20d8-4d18-91bd-e6c1baaa3a32'; } } }")
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('1 table sources parsed, 1 bound, 0 unbound', result.stdout)
        compiled = self.compile_page()
        self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
        previous = os.environ.get('AGIRU_NATIVE_OWNER_PREVIOUS')
        if previous:
            original_compiler = self.transpiler
            self.transpiler = Path(previous)
            try:
                old = self.run_compiler()
            finally:
                self.transpiler = original_compiler
            self.assertEqual(old.returncode, 1, old.stdout + old.stderr)
            self.assertIn('invalid moved field takeover: Page ID', old.stderr)

    def test_table_only_manifest_malformed_identity_and_dtd_refuse(self):
        manifest = self.native_table_manifest()
        original = manifest.read_text()
        for mutant in ('<Package>',
                       original.replace('85a884cd-20d8-4d18-91bd-e6c1baaa3a32', 'invalid'),
                       '<!DOCTYPE Package [<!ENTITY name "Unsafe">]>' + original):
            with self.subTest(mutant=mutant):
                manifest.write_text(mutant)
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('System', result.stderr)
                self.assertFalse(self.generated.exists())

    def test_table_only_manifest_symlinks_refuse_before_output(self):
        manifest = self.native_table_manifest()
        held = self.package / 'held.xml'
        manifest.rename(held)
        for target in (held, self.package / 'missing.xml'):
            with self.subTest(target=target):
                manifest.symlink_to(target)
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('System manifest is a symlink', result.stderr)
                self.assertFalse(self.generated.exists())
                manifest.unlink()

    def test_table_only_raw_fixture_without_manifest_stays_unqualified(self):
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertFalse((self.generated / 'platform/PlatformModule.h').exists())
        self.assertIn('no provider or business execution proof', result.stdout)

    def test_native_product_scope_retains_raw_identity_and_required_page(self):
        commercial = self.package / 'src/Commercial.Table.al'
        commercial.write_text('namespace System.Security.AccessControl; '
            'table 2000000998 "Authored Commercial" { fields { field(1; ID; Integer) {} } }')
        rule = 'bc-licensing:system-symbols/src/Commercial.Table.al'
        policy = {'include': ['System', 'Microsoft'], 'exclude': [], 'product_exclude': [rule]}
        (self.root / 'scope.json').write_text(json.dumps(policy))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('native product-excluded table 2000000998', result.stdout)
        self.assertIn('1 table sources parsed, 1 bound, 0 unbound', result.stdout)
        report = scope_inventory.inventory(self.package,
            {'apps': [{'name': 'native', 'source': 'src'}]}, policy, 'system-symbols')
        self.assertFalse(report['errors'], report['errors'])
        self.assertEqual(report['summary']['objects_by_kind']['table'], 2)
        excluded = [row for row in report['objects'] if row['product_exclusion_reason']]
        self.assertEqual([(row['id'], row['product_exclusion_reason']) for row in excluded],
                         [(2000000998, 'bc-licensing')])
        compiled = self.compile_page()
        self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
        self.assertEqual(sum(len(row['methods']) for row in milestone.scan(self.source)), 1)
        policy['product_exclude'] = []
        (self.root / 'scope.json').write_text(json.dumps(policy))
        reintroduced = self.run_compiler()
        self.assertEqual(reintroduced.returncode, 1, reintroduced.stdout + reintroduced.stderr)
        self.assertIn('native-unbound 2000000998', reintroduced.stdout)

    def test_native_rule_cannot_exclude_the_same_relative_bcapps_path(self):
        renamed = self.package / 'src/unusual-name.Codeunit.al'
        self.native.rename(renamed)
        self.native = renamed
        (self.root / 'scope.json').write_text(json.dumps({'include': ['System', 'Microsoft'],
            'exclude': [], 'product_exclude': ['bc-licensing:system-symbols/src/unusual-name.Codeunit.al']}))
        (self.root / 'src').mkdir()
        (self.root / 'src/unusual-name.Codeunit.al').write_text('namespace Microsoft.Fixture; '
            'codeunit 50186 Retained { procedure Execute() begin end; }')
        apps = json.loads(self.apps.read_text())
        apps['apps'].append({'name': 'other', 'source': 'src'})
        self.apps.write_text(json.dumps(apps))
        result = self.run_compiler()
        self.assertTrue(list(self.generated.rglob('Retained.h')),
            result.stdout + result.stderr + '\n' +
            '\n'.join(str(path.relative_to(self.generated)) for path in self.generated.rglob('*')))
        self.assertIn('native product-excluded table 2000000171', result.stdout)
        report = scope_inventory.inventory(self.root, apps, json.loads((self.root / 'scope.json').read_text()))
        retained = next(row for row in report['objects'] if row['id'] == 50186)
        self.assertIsNone(retained['product_exclusion_reason'])

    def test_native_exclusion_requires_its_original_source_package(self):
        (self.root / 'scope.json').write_text(json.dumps({'include': ['System', 'Microsoft'],
            'exclude': [], 'product_exclude': ['bc-licensing:system-symbols/src/unusual-name.aL']}))
        result = self.run_compiler([])
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('System product exclusion requires --system-symbols', result.stderr)
        self.assertFalse(self.generated.exists())

    def moved_field_sources(self, reverse=False, destination=True):
        source_id = '118874ab-44bc-4ccb-9daf-59763539ab16'
        destination_id = '85a884cd-20d8-4d18-91bd-e6c1baaa3a32'
        (self.source / 'app.json').write_text(json.dumps({
            'id': source_id, 'name': 'Moved Field Source',
            'publisher': 'agiru tests', 'version': '1.0.0.0'}))
        (self.source / 'Moved.Table.al').write_text('namespace Microsoft.Fixture; '
            'table 50180 "Moved Row" { fields { field(1; ID; Integer) {} } }')
        (self.source / 'Old.TableExt.al').write_text('namespace Microsoft.Fixture; '
            'tableextension 50181 OldOwner extends "Moved Row" { fields { '
            'field(2; Value; Code[10]) { ObsoleteState = Moved; '
            f"MovedTo = '{destination_id}';" + ' } } }')
        successor = self.root / 'successor'
        successor.mkdir()
        (successor / 'app.json').write_text(json.dumps({
            'id': destination_id, 'name': 'Moved Field Destination',
            'publisher': 'agiru tests', 'version': '1.0.0.0'}))
        declaration = successor / 'New.TableExt.al'
        if destination:
            declaration.write_text('namespace Microsoft.Fixture; '
                'tableextension 50182 NewOwner extends "Moved Row" { fields { '
                'field(2; Value; Code[10]) { Caption = \'Owned value\'; '
                f"MovedFrom = '{source_id}';" + ' } } }')
        apps = [{'name': 'fixture', 'source': 'source'},
                {'name': 'successor', 'source': 'successor'}]
        self.apps.write_text(json.dumps({'apps': list(reversed(apps)) if reverse else apps}))
        return declaration, source_id, destination_id

    def test_moved_field_takeover_replaces_the_source_in_both_orders(self):
        self.moved_field_sources()
        for reverse in (False, True):
            apps = json.loads(self.apps.read_text())
            if reverse:
                apps['apps'].reverse()
                self.apps.write_text(json.dumps(apps))
            result = self.run_compiler()
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            definitions = (self.generated / 'fixture/fixture/table/MovedRow.def.cpp').read_text()
            self.assertIn('.movedFrom = "118874ab-44bc-4ccb-9daf-59763539ab16"', definitions)
            self.assertNotIn('.obsoleteState = "Moved"', definitions)
            self.assertEqual(definitions.count('"Owned value"'), 1)
            self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')

    def test_wrong_field_takeover_refuses_instead_of_first_writer_wins(self):
        declaration, source_id, destination_id = self.moved_field_sources()
        original = declaration.read_text()
        for mutant in (original.replace(source_id, destination_id),
                       original.replace('Code[10]', 'Code[20]'),
                       original.replace('field(2;', 'field(3;'),
                       original.replace('Value;', 'Other;')):
            declaration.write_text(mutant)
            result = self.run_compiler()
            self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('invalid moved field takeover: Value', result.stderr)

    def test_missing_field_destination_is_inaccessible_and_reported(self):
        self.moved_field_sources(destination=False)
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('moved-field-unavailable Moved Row.Value (2)', result.stdout)
        self.assertIn('destination declaration absent; stored data retained', result.stdout)
        header = (self.generated / 'fixture/fixture/table/MovedRow.h').read_text()
        self.assertNotIn('Code<10> Value', header)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')

    def page_body(self):
        return self.generated / 'fixture/system/tooling/page/PageFieldsSelectionList.cpp'

    def compile_page(self):
        arguments = [os.environ.get('CXX', 'clang++-19'), '-std=c++23', '-stdlib=libc++',
            '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-fsyntax-only',
            f'-I{self.repository / "include"}']
        arguments += [f'-I{self.generated / name}' for name in ('fixture', 'shared', 'absent')]
        arguments += [str(self.page_body()), str(self.page_body().with_suffix('.def.cpp'))]
        return subprocess.run(arguments, text=True, capture_output=True, timeout=60)

    def run_original_primitive(self):
        build = (self.repository / os.environ.get('B', 'build')).resolve()
        executable = self.root / 'reader'
        arguments = [os.environ.get('CXX', 'clang++-19'), '-std=c++23', '-stdlib=libc++',
            '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
            '-Wall', '-Wextra', '-Wpedantic', '-Werror', f'-I{self.repository / "include"}',
            f'-I{self.repository / "test/gate"}']
        arguments += [f'-I{self.generated / name}' for name in ('fixture', 'shared', 'absent')]
        arguments += [str(self.repository / 'test/transpiler/native-binding/PageRunner.cpp'),
            str(self.page_body()), str(self.page_body().with_suffix('.def.cpp')),
            f'-L{build}', f'-Wl,-rpath,{build}', '-lagiru_rt', '-lagiru_net', '-lagiru_db',
            '-o', str(executable)]
        compiled = subprocess.run(arguments, text=True, capture_output=True, timeout=60)
        self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
        return subprocess.run([str(executable)], text=True, capture_output=True, timeout=30)

    def test_original_bare_and_numeric_fields_use_the_source_ast(self):
        before = self.run_compiler([])
        self.assertEqual(before.returncode, 0, before.stdout + before.stderr)
        self.assertIn('Format(Caption)', self.page_body().read_text())
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('1 table sources parsed, 1 bound, 0 unbound', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')
        self.assertNotIn('Format(Caption)', self.page_body().read_text())
        definitions = self.page_body().with_suffix('.def.cpp').read_text()
        self.assertIn('native field declaration mismatch: Page Table Field.Caption', definitions)
        numeric_body = (self.generated / 'fixture/system/tooling/page/NumericSource.cpp').read_text()
        self.assertIn('::agiru::platform::PageTableFieldType::Integer', numeric_body)
        self.assertNotIn('::agiru::options::Option', numeric_body)
        self.assertIn('::agiru::FieldNo{5}', definitions)
        numeric = self.generated / 'fixture/system/tooling/page/NumericSource.def.cpp'
        self.assertIn('::agiru::FieldNo{5}', numeric.read_text())
        compiled = self.compile_page()
        self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
        executed = self.run_original_primitive()
        self.assertEqual(executed.returncode, 0, executed.stdout + executed.stderr)
        self.assertIn('6 check(s), 0 red', executed.stdout)

    def test_original_source_expression_mutant_fails_the_production_primitive(self):
        page = self.source / 'Original.Page.al'
        original = page.read_text()
        self.assertEqual(original.count('field(Caption; Caption)'), 1)
        page.write_text(original.replace('field(Caption; Caption)', 'field(Caption; Name)'))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        executed = self.run_original_primitive()
        self.assertNotEqual(executed.returncode, 0)
        self.assertIn('6 check(s), 4 red', executed.stdout)

    def test_original_native_field_number_mutant_refuses_at_compilation(self):
        original = self.native.read_text()
        self.assertEqual(original.count('field(5; Caption;'), 1)
        self.native.write_text(original.replace('field(5; Caption;', 'field(99999; Caption;'))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        compiled = self.compile_page()
        self.assertNotEqual(compiled.returncode, 0)
        self.assertIn('native field declaration mismatch: Page Table Field.Caption', compiled.stderr)

    def test_unbound_and_non_table_sources_remain_red_and_counted(self):
        self.native_table_manifest()
        (self.package / 'src/Unknown.al').write_text('table 50199 Unknown { fields { field(1; ID; Integer) {} } }')
        (self.package / 'src/Code.al').write_text('codeunit 50200 Unactivated { [Native] procedure Read() begin end; }')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('native-unbound 50199 Unknown', result.stdout)
        self.assertIn('2 table sources parsed, 1 bound, 1 unbound', result.stdout)
        self.assertIn('0 other AL sources not activated', result.stdout)
        self.assertIn('1 codeunit declarations selected; 1 native methods unbound', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')

    def test_native_codeunit_source_identity_and_policy_are_retained(self):
        self.native_table_manifest()
        declared = self.package / 'src/not-a-codeunit-name.aL'
        declared.write_text('namespace System.Fixture; codeunit 50200 Unactivated { '
            '[Native] procedure Read(var Value: Integer): Text begin end; }')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('native parsed codeunit declarations: 1 before policy, 1 selected, 0 product-excluded', result.stdout)
        self.assertIn('native-codeunit-declared 50200 System.Fixture.Unactivated: src/not-a-codeunit-name.aL', result.stdout)
        self.assertIn('native-method-unbound codeunit 50200 System.Fixture.Unactivated.Read(var Value: Integer): Text', result.stdout)
        self.assertIn('0 other AL sources not activated', result.stdout)
        self.assertIn('1 codeunit sources indexed; 1 codeunit objects written into the platform app', result.stdout)
        self.assertIn('0 declaration(s) of 0 kind(s) are read and dropped', result.stdout)
        header = self.generated / 'platform/system/fixture/codeunit/Unactivated.h'
        body = header.with_suffix('.cpp')
        self.assertIn('CodeunitId kId{50200}', header.read_text())
        self.assertIn('src/not-a-codeunit-name.aL', body.read_text())
        self.assertIn('has no native implementation', body.read_text())
        module = self.generated / 'platform/PlatformModule.h'
        self.assertIn('85a884cd-20d8-4d18-91bd-e6c1baaa3a32', module.read_text())
        self.assertIn('1.2.3.4', module.read_text())
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')
        policy = {'include': ['System', 'Microsoft'], 'exclude': [],
            'product_exclude': ['bc-licensing:system-symbols/src/not-a-codeunit-name.aL']}
        (self.root / 'scope.json').write_text(json.dumps(policy))
        excluded = self.run_compiler()
        self.assertEqual(excluded.returncode, 0, excluded.stdout + excluded.stderr)
        self.assertIn('native parsed codeunit declarations: 1 before policy, 0 selected, 1 product-excluded', excluded.stdout)
        self.assertIn('native product-excluded codeunit 50200 System.Fixture.Unactivated:', excluded.stdout)
        self.assertNotIn('native-method-unbound', excluded.stdout)
        self.assertFalse(header.exists())

    def test_native_base64_binding_census_preserves_unbound_signatures(self):
        self.native_table_manifest()
        declared = self.package / 'src/conversion.aL'
        declared.write_text('''namespace System.Runtime;
codeunit 2000000024 Base64Convert {
 [Native] procedure ToBase64(S: Text; L: Boolean; E: TextEncoding; P: Integer): Text begin end;
 [Native] procedure ToBase64(S: Text; L: Boolean; E: TextEncoding; P: Integer; O: OutStream) begin end;
 [Native] procedure FromBase64(S: Text; E: TextEncoding; P: Integer): Text begin end;
 [Native] procedure FromBase64(S: Text; E: TextEncoding; P: Integer; O: OutStream) begin end;
 [Native] procedure FromBase64(S: Text; O: OutStream) begin end;
 [Native] procedure ToBase64(S: InStream; L: Boolean): Text begin end;
 [Native] procedure ToBase64(S: InStream; L: Boolean; O: OutStream) begin end;
 [Native] procedure FromBase64(S: InStream): Text begin end;
 [Native] procedure FromBase64(S: InStream; O: OutStream) begin end;
}''')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertEqual(result.stdout.count('native-method-bound codeunit'), 5)
        self.assertEqual(result.stdout.count('native-method-unbound codeunit'), 4)
        self.assertIn('1 codeunit declarations selected; 4 native methods unbound', result.stdout)
        body = self.generated / 'platform/system/runtime/codeunit/Base64Convert.cpp'
        self.assertEqual(body.read_text().count('has no native implementation'), 4)
        self.assertEqual(body.read_text().count('::agiru::NativeToBase64('), 2)
        self.assertEqual(body.read_text().count('::agiru::NativeFromBase64('), 3)
        declared.write_text(declared.read_text().replace('namespace System.Runtime;', 'namespace Other;'))
        wrong_identity = self.run_compiler()
        self.assertEqual(wrong_identity.returncode, 1, wrong_identity.stdout + wrong_identity.stderr)
        self.assertNotIn('native-method-bound codeunit', wrong_identity.stdout)
        self.assertEqual(wrong_identity.stdout.count('native-method-unbound codeunit'), 9)

    def test_unbound_native_methods_refuse_in_analysis_with_and_without_a_system_package(self):
        self.native_table_manifest()
        declaration = ('namespace System.Fixture; codeunit 50200 NativeUnit { '
            '[Native] procedure Read(): Integer begin exit(7); end; }')
        (self.package / 'src/Code.al').write_text(declaration)
        (self.source / 'Unbound.Codeunit.al').write_text(declaration.replace('50200', '50201')
            .replace('NativeUnit', 'AppNative'))
        for extra in (None, []):
            with self.subTest(extra=extra):
                result = self.run_compiler(extra=extra, output=False)
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('Native in codeunit 50201 System.Fixture.AppNative.Read(): Integer', result.stdout)
                self.assertIn('0 declaration(s) of 0 kind(s) are read and dropped', result.stdout)
                self.assertFalse(self.generated.exists())

    def test_an_app_cannot_replace_native_codeunit_identity_or_ambiguously_rebind_its_name(self):
        self.native_table_manifest()
        (self.package / 'src/Native.al').write_text('namespace System.Fixture; codeunit 50200 NativeUnit {}')
        for source, refusal in (
                ('namespace Microsoft.Fixture; codeunit 50200 Other {}',
                 'duplicates declared System codeunit ID 50200'),
                ('namespace System.Fixture; codeunit 50201 NativeUnit {}',
                 'duplicates declared System qualified codeunit name: System.Fixture.NativeUnit'),
                ('namespace Microsoft.Fixture; codeunit 50201 NativeUnit {}',
                 'ambiguous native/app codeunit name: NativeUnit')):
            with self.subTest(source=source):
                (self.source / 'Conflict.Codeunit.al').write_text(source)
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn(refusal, result.stderr)
                self.assertFalse((self.generated / 'platform/system/fixture/codeunit/NativeUnit.h').exists())

    def test_native_codeunit_bare_name_ambiguity_refuses_instead_of_overwriting(self):
        self.native_table_manifest()
        (self.package / 'src/First.al').write_text('namespace System.First; codeunit 50200 NativeUnit {}')
        (self.package / 'src/Second.al').write_text('namespace System.Second; codeunit 50201 NativeUnit {}')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('ambiguous native codeunit name: NativeUnit', result.stderr)
        self.assertFalse((self.generated / 'platform').exists())

    def test_codeunit_only_package_reserves_platform_and_emits_ordinary_al_bodies(self):
        self.native.unlink()
        self.native_table_manifest()
        (self.package / 'src/Code.aL').write_text('namespace System.Fixture; codeunit 50200 NativeUnit { '
            'procedure Read(): Integer begin exit(7); end; }')
        result = self.run_compiler()
        self.assertIn('1 codeunit sources indexed; 1 codeunit objects written', result.stdout)
        self.assertNotIn('native-method-unbound', result.stdout)
        body = self.generated / 'platform/system/fixture/codeunit/NativeUnit.cpp'
        self.assertIn('return 7;', body.read_text())
        self.apps.write_text(json.dumps({'apps': [{'name': 'platform', 'source': 'source'}]}))
        refused = self.run_compiler()
        self.assertEqual(refused.returncode, 1, refused.stdout + refused.stderr)
        self.assertIn('platform is reserved for the supplied System package', refused.stderr)

    def test_native_codeunit_duplicate_id_or_qualified_name_refuses_before_output(self):
        self.native_table_manifest()
        original = 'namespace System.Fixture; codeunit 50200 Unactivated {}'
        (self.package / 'src/First.al').write_text(original)
        for mutant in (original.replace('Unactivated', 'Other'), original.replace('50200', '50201')):
            with self.subTest(mutant=mutant):
                (self.package / 'src/Duplicate.al').write_text(mutant)
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('invalid or duplicate codeunit identity', result.stderr)
                self.assertFalse(self.generated.exists())

    def test_native_codeunit_missing_manifest_is_not_an_invented_owner(self):
        (self.package / 'src/Code.al').write_text('codeunit 50200 Unactivated {}')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('cannot read System NavxManifest.xml', result.stderr)
        self.assertFalse(self.generated.exists())

    def test_native_codeunit_parse_refusal_retains_source_and_ut_population(self):
        (self.package / 'src/Code.al').write_text('codeunit 50200 Unactivated { [Native] procedure Read(')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('native-refused src/Code.al', result.stdout)
        self.assertIn('1 source refusals', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')

    def test_comments_cannot_manufacture_a_native_declaration(self):
        self.native.write_text('// table 2000000171 "Page Table Field" { fields {} }\n')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('0 table sources parsed, 0 bound, 0 unbound', result.stdout)
        self.assertIn('1 other AL sources not activated', result.stdout)
        self.assertNotIn('Format(Rec.Caption)', self.page_body().read_text())

    def test_broken_source_refuses_without_losing_the_ut_population(self):
        self.native.write_text('table 2000000171 "Page Table Field" { fields {')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('native-refused src/unusual-name.aL', result.stdout)
        self.assertIn('1 source refusals', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')

    def test_duplicate_source_identity_refuses(self):
        shutil.copyfile(self.native, self.package / 'src/Duplicate.al')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('duplicate System table declaration', result.stderr)
        self.assertFalse(self.page_body().exists())

    def test_an_app_cannot_replace_a_declared_native_table_id(self):
        (self.source / 'Conflict.Table.al').write_text('namespace Microsoft.Fixture;\n'
            'table 2000000171 Conflict { fields { field(1; ID; Integer) {} } }')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('duplicates declared System table ID 2000000171', result.stderr)
        self.assertFalse(self.page_body().exists())

    def test_source_symlinks_and_missing_roots_refuse_before_output(self):
        (self.package / 'src/Symlink.al').symlink_to(self.native)
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('symlink', result.stderr)
        self.assertFalse(self.generated.exists())
        result = self.run_compiler(['--system-symbols', str(self.root / 'missing')])
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('source directory is missing', result.stderr)
        self.assertFalse(self.generated.exists())

    def test_empty_duplicate_and_unknown_options_are_not_silently_ignored(self):
        for extra in (['--system-symbols', ''], ['--system-symbols'],
                      ['--unknown'], ['--system-symbols', str(self.package), '--extra']):
            with self.subTest(extra=extra):
                result = self.run_compiler(extra)
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertFalse(self.generated.exists())

    def test_native_extension_keeps_the_source_contract_and_cannot_pass_the_old_abi(self):
        (self.source / 'Extension.TableExt.al').write_text('namespace Microsoft.Fixture;\n'
            'tableextension 50173 Added extends "Page Table Field" { fields { '
            'field(60000; Extra; Integer) {} } }')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        definitions = self.page_body().with_suffix('.def.cpp').read_text()
        self.assertIn('native field declaration mismatch: Page Table Field.Extra', definitions)
        self.assertIn('return declared == 16;', definitions)
        compiled = self.compile_page()
        self.assertNotEqual(compiled.returncode, 0)
        self.assertIn('native field count mismatch: Page Table Field', compiled.stderr)


class NativeReportSourceCompilerGate(unittest.TestCase):
    def setUp(self):
        self.repository = Path(__file__).resolve().parents[2]
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.package = self.root / 'package'
        (self.package / 'src').mkdir(parents=True)
        self.source = self.root / 'source'
        self.source.mkdir()
        self.generated = self.root / 'generated'
        self.transpiler = (self.repository / os.environ.get('B', 'build') / 'agirutc').resolve()
        self.apps = self.root / 'apps.json'
        self.apps.write_text(json.dumps({'apps': [{'name': 'fixture', 'source': 'source'}]}))
        (self.root / 'scope.json').write_text(json.dumps({'include': ['System', 'Microsoft'], 'exclude': []}))
        (self.source / 'app.json').write_text(json.dumps({'id': '118874ab-44bc-4ccb-9daf-59763539ab16',
            'name': 'Fixture', 'publisher': 'Tests', 'version': '1.0.0.0'}))
        (self.source / 'Population.Codeunit.al').write_text('namespace Microsoft.Fixture;\n'
            'codeunit 50231 "Native Report UT"\n{ Subtype = Test; [Test] procedure Kept() begin end; }')
        self.report = self.package / 'src/not-a-report-filename.aL'
        self.report.write_text('namespace System.Fixture;\n'
            'report 50230 "Native Fixture" { DefaultRenderingLayout = Original; dataset {} '
            'rendering { layout(Original) { Type = Word; LayoutFile = \'original.docx\'; } } }')
        self.manifest = self.package / 'NavxManifest.xml'
        self.manifest.write_text('<Package xmlns="http://schemas.microsoft.com/navx/2015/manifest">'
            '<App Id="85a884cd-20d8-4d18-91bd-e6c1baaa3a32" Name="Native &amp; Fixture" '
            'Publisher="Test Publisher" Version="1.2.3.4"/></Package>')
        (self.source / 'Layout.ReportExt.al').write_text('namespace Microsoft.Fixture;\n'
            'reportextension 50232 Extra extends "Native Fixture" { rendering { layout(Extra) '
            '{ Type = Word; LayoutFile = \'extra.docx\'; } } }')

    def run_compiler(self, output=True):
        arguments = [str(self.transpiler), str(self.root), str(self.apps)]
        if output:
            arguments.append(str(self.generated))
        arguments += ['--system-symbols', str(self.package)]
        return subprocess.run(arguments, text=True, capture_output=True, timeout=30)

    def definitions(self):
        return self.generated / 'platform/system/fixture/report/NativeFixture.def.cpp'

    def test_original_ast_and_extension_keep_separate_source_owned_modules(self):
        original_manifest = self.manifest.read_text()
        self.manifest.write_text(original_manifest.replace('Version="1.2.3.4"',
            'Version="1.2.3.4" Runtime="18.0"'))
        app_path = self.source / 'app.json'
        identity = json.loads(app_path.read_text())
        identity['runtime'] = '16.0'
        app_path.write_text(json.dumps(identity))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('1 report sources bound; 3 objects written into the platform app', result.stdout)
        self.assertIn('2 bound layouts, 2 immutable declarations emitted', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')
        definitions = self.definitions().read_text()
        self.assertIn('platform/src/not-a-report-filename.aL', definitions)
        self.assertIn('85a884cd-20d8-4d18-91bd-e6c1baaa3a32', definitions)
        self.assertIn('118874ab-44bc-4ccb-9daf-59763539ab16', definitions)
        module = (self.generated / 'platform/PlatformModule.h').read_text()
        self.assertIn('Native & Fixture', module)
        self.assertIn('1.2.3.4', module)
        self.assertIn('.minimumRuntime = "18.0"', module)
        ordinary_module = self.generated / 'fixture/FixtureModule.h'
        self.assertIn('.minimumRuntime = "16.0"', ordinary_module.read_text())
        arguments = [os.environ.get('CXX', 'clang++-19'), '-std=c++23', '-stdlib=libc++',
            '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-fsyntax-only',
            f'-I{self.repository / "include"}', f'-I{self.generated / "platform"}']
        sources = [str(path) for path in sorted((self.generated / 'platform').rglob('*.cpp'))]
        compiled = subprocess.run(arguments + sources, text=True, capture_output=True, timeout=30)
        self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
        contract = ('#include "PlatformModule.h"\n#include "FixtureModule.h"\n'
            'static_assert(agiru::app::Platform::kModule.minimumRuntime == "18.0");\n'
            'static_assert(agiru::app::Fixture::kModule.minimumRuntime == "16.0");\n'
            'static_assert(agiru::app::Platform::kModule.version == "1.2.3.4");\n'
            'static_assert(agiru::app::Fixture::kModule.version == "1.0.0.0");\n')
        consumer = arguments + [f'-I{self.generated / "fixture"}', '-x', 'c++', '-']
        compiled = subprocess.run(consumer, input=contract, text=True, capture_output=True, timeout=30)
        self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
        native_module = self.generated / 'platform/PlatformModule.h'
        control = module.replace('.minimumRuntime = "18.0"', '.minimumRuntime = "17.0"')
        self.assertNotEqual(module, control)
        native_module.write_text(control)
        compiled = subprocess.run(consumer, input=contract, text=True, capture_output=True, timeout=30)
        self.assertNotEqual(compiled.returncode, 0, 'wrong native runtime escaped the compiled contract')
        self.assertIn('static assertion failed', compiled.stderr)
        native_module.write_text(module)
        self.manifest.write_text(original_manifest)
        del identity['runtime']
        app_path.write_text(json.dumps(identity))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        omitted = contract.replace('"18.0"', '""').replace('"16.0"', '""')
        compiled = subprocess.run(consumer, input=omitted, text=True, capture_output=True, timeout=30)
        self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)

    def test_source_only_retains_layouts_without_claiming_emission(self):
        result = self.run_compiler(output=False)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('2 bound layouts, 0 immutable declarations emitted', result.stdout)
        self.assertFalse(self.generated.exists())

    def test_missing_or_malformed_native_identity_refuses_before_output(self):
        empty_runtime = self.manifest.read_text().replace('Version="1.2.3.4"',
            'Version="1.2.3.4" Runtime=""')
        for text in (None, '<Package>', '<Package><App/></Package>',
                     '<Package xmlns="http://schemas.microsoft.com/navx/2015/manifest"><App/></Package>',
                     empty_runtime):
            with self.subTest(text=text):
                if text is None:
                    self.manifest.unlink()
                else:
                    self.manifest.write_text(text)
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertFalse(self.generated.exists())

    def test_duplicate_app_manifest_and_dtd_refuse_before_output(self):
        original = self.manifest.read_text()
        app = re.search(r'<App .*/>', original).group()
        for text in (original.replace('</Package>', app + '</Package>'),
                     '<!DOCTYPE Package [<!ENTITY name "Unsafe">]>' + original):
            with self.subTest(text=text):
                self.manifest.write_text(text)
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertFalse(self.generated.exists())

    def test_manifest_symlink_and_invalid_guid_refuse_before_output(self):
        original = self.manifest.read_text()
        held = self.package / 'held.xml'
        held.write_text(original)
        self.manifest.unlink()
        self.manifest.symlink_to(held)
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('manifest is a symlink', result.stderr)
        self.assertFalse(self.generated.exists())
        self.manifest.unlink()
        self.manifest.write_text(original.replace('85a884cd-20d8-4d18-91bd-e6c1baaa3a32', 'not-a-guid'))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('invalid App Id', result.stderr)
        self.assertFalse(self.generated.exists())

    def test_duplicate_native_report_identity_refuses_before_output(self):
        (self.package / 'src/duplicate.al').write_text(self.report.read_text())
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('duplicates report identity', result.stderr)
        self.assertFalse(self.generated.exists())

    def test_an_application_cannot_replace_the_native_report_id(self):
        (self.source / 'Conflict.Report.al').write_text('namespace Microsoft.Fixture;\n'
            'report 50230 Conflict { dataset {} }')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('duplicates declared System report ID 50230', result.stderr)
        self.assertFalse(self.definitions().exists())

    def test_unclassified_sources_keep_translation_red_and_population_counted(self):
        (self.package / 'src/other.al').write_text('page 50233 Unactivated {}')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('1 other AL sources not activated', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')
        self.assertTrue(self.definitions().exists())

    def test_reserved_generated_platform_name_cannot_be_an_application(self):
        self.apps.write_text(json.dumps({'apps': [{'name': 'platform', 'source': 'source'}]}))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('platform is reserved', result.stderr)
        self.assertFalse(self.generated.exists())

    def test_native_report_parse_failure_retains_the_original_source(self):
        self.report.write_text('report 50230 "Native Fixture" { dataset {')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('native-refused src/not-a-report-filename.aL', result.stdout)
        self.assertIn('1 source refusals', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')
        self.assertFalse(self.definitions().exists())

    def test_comment_text_is_not_a_native_report_declaration(self):
        self.report.write_text('// report 50230 "Native Fixture" { dataset {} }\n'
            'page 50230 "Not A Report" {}')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('1 other AL sources not activated', result.stdout)
        self.assertNotIn('report sources bound', result.stdout)
        self.assertFalse(self.definitions().exists())


class NativeEnumSourceCompilerGate(unittest.TestCase):
    def setUp(self):
        self.repository = Path(__file__).resolve().parents[2]
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name) / 'input'
        shutil.copytree(self.repository / 'test/transpiler/native-enums', self.root)
        self.package = self.root / 'package'
        self.native = self.package / 'src/unusual-name.aL'
        self.generated = self.root / 'generated'
        self.transpiler = (self.repository / os.environ.get('B', 'build') / 'agirutc').resolve()

    def run_compiler(self):
        return subprocess.run([str(self.transpiler), str(self.root), str(self.root / 'apps.json'),
            str(self.generated), '--system-symbols', str(self.package)],
            text=True, capture_output=True, timeout=30)

    def test_original_declaration_and_extension_share_the_production_enum_binder(self):
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('native 1 enum sources bound; 1 enum objects written', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')
        header = self.generated / 'platform/system/fixture/enum/NativeSparse.h'
        self.assertIn('Extra = 70', header.read_text())
        self.assertIn('kObjectID = 50240', header.read_text())
        self.assertIn('kScope{"Cloud"}', header.read_text())
        self.assertIn('kExtensible = true', header.read_text())
        self.assertIn('Native Enum Fixture', (self.generated / 'platform/PlatformModule.h').read_text())
        consumer = (self.generated / 'fixture/fixture/codeunit/NativeConsumerUT.h').read_text()
        self.assertNotIn('Enum<>', consumer)
        self.assertIn('::agiru::System::Fixture::NativeSparse_Enum', consumer)

    def test_unactivated_sources_remain_red_without_losing_the_ut_population(self):
        (self.package / 'src/Other.al').write_text('page 50245 Unactivated {}')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('1 other AL sources not activated', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')
        self.assertIn('native 1 enum sources bound', result.stdout)

    def test_malformed_native_enum_keeps_its_source_refusal_and_ut_population(self):
        self.native.write_text('enum 50240 "Native Sparse" { value(0; None) {')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('native-refused src/unusual-name.aL', result.stdout)
        self.assertIn('1 source refusals', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')

    def test_missing_manifest_is_not_replaced_by_an_invented_owner(self):
        (self.package / 'NavxManifest.xml').unlink()
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertFalse(self.generated.exists())

    def test_duplicate_native_enum_id_and_qualified_name_refuse_before_output(self):
        original = self.native.read_text()
        for mutant in (original, original.replace('50240', '50249')):
            with self.subTest(mutant=mutant):
                (self.package / 'src/Duplicate.al').write_text(mutant)
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('duplicates enum identity', result.stderr)
                self.assertFalse(self.generated.exists())

    def test_application_enum_cannot_overwrite_a_native_id_or_name(self):
        for declaration in ('enum 50240 Conflict', 'enum 50249 "Native Sparse"'):
            with self.subTest(declaration=declaration):
                (self.root / 'source/Conflict.Enum.al').write_text('namespace System.Fixture;\n' +
                    declaration + ' { value(0; None) {} }')
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('duplicates declared System enum identity', result.stderr)
                self.assertFalse((self.generated / 'platform/system/fixture/enum/NativeSparse.h').exists())

    def test_non_extensible_enum_cannot_accept_an_extension(self):
        original = self.native.read_text()
        for mutant in (original.replace('Extensible = true;', 'Extensible = false;'),
                       original.replace('Extensible = true;', '')):
            with self.subTest(mutant=mutant):
                self.native.write_text(mutant)
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('Cannot extend non-extensible enum', result.stderr)

    def test_invalid_extensibility_refuses_instead_of_becoming_false(self):
        self.native.write_text(self.native.read_text().replace('Extensible = true;',
                                                               'Extensible = Invalid;'))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('Invalid Extensible property', result.stderr)

    def test_namespace_ambiguity_refuses_instead_of_overwriting_a_native_alias(self):
        (self.root / 'source/Conflict.Enum.al').write_text('namespace Microsoft.Other;\n'
            'enum 50249 "Native Sparse" { value(0; None) {} }')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('Ambiguous unqualified System enum name', result.stderr)

    def test_native_namespace_ambiguity_refuses_before_output(self):
        original = self.native.read_text()
        (self.package / 'src/Duplicate.al').write_text(
            original.replace('System.Fixture', 'System.Other').replace('50240', '50249'))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('ambiguous unqualified enum name', result.stderr)
        self.assertFalse(self.generated.exists())

    def test_missing_native_interface_remains_a_gap_not_a_successful_empty_binding(self):
        original = self.native.read_text()
        self.native.write_text(original.replace('enum 50240 "Native Sparse"',
                                                'enum 50240 "Native Sparse" implements Missing'))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('native-enum-interface-unbound 50240 Native Sparse: Missing', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')
        header = self.generated / 'platform/system/fixture/enum/NativeSparse.h'
        self.assertIn('kInterfaces{"Missing"}', header.read_text())

    def test_native_interfaces_keep_inheritance_and_typed_signatures(self):
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('native 2 interface sources bound; 2 interface objects written', result.stdout)
        base = self.generated / 'platform/system/fixture/interface/NativeBase.h'
        derived = self.generated / 'platform/system/fixture/interface/NativeContract.h'
        self.assertIn('NativeSparse_Enum>', base.read_text())
        self.assertIn('public virtual ::agiru::System::Fixture::NativeBase_Interface', derived.read_text())
        self.assertRegex(derived.read_text(), r'Echo\([^\n]+NativeSparse_Enum>\s*&')

    def test_runtime_context_signature_types_are_not_opaque_absent_carriers(self):
        (self.package / 'src/context-contract.aL').write_text('''namespace System.Fixture;
interface "Context Contract" {
    procedure Handler(Context: TestHandlerContext);
    procedure Source(Context: DataSourceContext);
}''')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        header = self.generated / 'platform/system/fixture/interface/ContextContract.h'
        self.assertIn('#include "type/TestHandlerContext.h"', header.read_text())
        self.assertIn('#include "type/DataSourceContext.h"', header.read_text())
        self.assertNotIn('absent::', header.read_text())
        self.assertNotIn('struct TestHandlerContext', (self.generated / 'absent/absent/Types.h').read_text())
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')

    def test_duplicate_native_interface_cannot_overwrite_the_original(self):
        original = (self.package / 'src/contract-derived.aL').read_text()
        (self.package / 'src/duplicate-contract.al').write_text(original)
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('duplicates interface identity', result.stderr)
        self.assertFalse(self.generated.exists())

    def test_application_interface_cannot_replace_a_native_contract(self):
        (self.root / 'source/Conflict.Interface.al').write_text(
            'namespace System.Fixture; interface "Native Contract" {}')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('duplicates declared System interface identity', result.stdout)

    def test_native_interface_missing_base_remains_a_counted_gap(self):
        path = self.package / 'src/contract-derived.aL'
        path.write_text(path.read_text().replace('extends "Native Base"', 'extends Missing'))
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('native-interface-base-unbound Native Contract: Missing', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \([^\n]*1 \[Test\] methods\)')

    def test_malformed_native_interface_keeps_its_source_refusal(self):
        (self.package / 'src/contract-derived.aL').write_text('interface "Native Contract" {')
        result = self.run_compiler()
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('native-refused src/contract-derived.aL', result.stdout)
        self.assertIn('1 source refusals', result.stdout)

    def test_duplicate_base_or_extension_value_cannot_be_silently_overwritten(self):
        original = self.native.read_text()
        extension = self.root / 'source/NativeSparse.EnumExt.al'
        original_extension = extension.read_text()
        for native, added in ((original.replace('value(10;', 'value(0;'), original_extension),
                              (original, original_extension.replace('value(70;', 'value(10;')),
                              (original, original_extension.replace('70; Extra', '70; Chosen'))):
            with self.subTest(native=native, added=added):
                self.native.write_text(native)
                extension.write_text(added)
                result = self.run_compiler()
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('Duplicate enum value', result.stderr)


class SymbolsPackageGate(unittest.TestCase):
    def test_transpile_wrapper_verifies_before_compiling_and_preserves_the_exit_status(self):
        root = Path(__file__).resolve().parents[2]
        with tempfile.TemporaryDirectory() as folder:
            work = Path(folder)
            package = symbols.publish(self.package(), '28.4.1.0', 'url', 'System.app', work)
            build = work / 'build'
            build.mkdir()
            compiler = build / 'agirutc'
            compiler.write_text('#!/bin/sh\nprintf "compiler-started\\n"\nexit 7\n')
            compiler.chmod(0o755)
            apps = work / 'apps.json'
            apps.write_text('{"apps":[]}')
            (work / 'scope.json').write_text('{"include":["System"],"exclude":[]}')
            environment = dict(os.environ, B=str(build), AGIRU_SYSTEM_SYMBOLS=str(package))
            environment.pop('AGIRU_HOST_RUNTIME', None)
            command = ['bash', 'scripts/transpile.sh', str(work), str(apps), str(work / 'generated')]
            result = subprocess.run(command, cwd=root, env=environment, text=True, capture_output=True)
            self.assertEqual(result.returncode, 7, result.stdout + result.stderr)
            receipt = Path((build / 'transpile.latest').read_text().strip())
            self.assertEqual((receipt / 'status').read_text().strip(), '7')
            invocation = json.loads((receipt / 'command.json').read_text())
            self.assertEqual(invocation[4:6], ['--host-runtime', '18.0'])
            self.assertEqual(invocation[-2:], ['--system-symbols', str(package)])
            self.assertEqual(json.loads((receipt / 'native-inventory.json').read_text())['summary']['objects'], 1)
            environment['AGIRU_HOST_RUNTIME'] = '17.0'
            result = subprocess.run(command, cwd=root, env=environment, text=True, capture_output=True)
            self.assertEqual(result.returncode, 7, result.stdout + result.stderr)
            receipt = Path((build / 'transpile.latest').read_text().strip())
            invocation = json.loads((receipt / 'command.json').read_text())
            self.assertEqual(invocation[4:6], ['--host-runtime', '17.0'])
            (package / 'src/Virtual Tables/Fixture.Table.al').write_text('changed')
            result = subprocess.run(command, cwd=root, env=environment, text=True, capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertNotIn('compiler-started', result.stdout)

    def test_native_consumers_build_the_compiler_before_qualification(self):
        root = Path(__file__).resolve().parents[2]
        environment = os.environ.copy()
        for name in ('MAKEFLAGS', 'MFLAGS', 'MAKEOVERRIDES'):
            environment.pop(name, None)
        with tempfile.TemporaryDirectory() as folder:
            result = subprocess.run(['make', '--no-print-directory', '-n', 'native-consumers',
                                     f'B={folder}', 'JOBS=2'], cwd=root, env=environment,
                                    text=True, capture_output=True, check=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertLess(result.stdout.index('--target agirutc'),
                        result.stdout.index('test/transpiler/native-consumers.sh'))

    def test_native_consumers_require_explicit_package_and_audit(self):
        root = Path(__file__).resolve().parents[2]
        environment = os.environ.copy()
        for name in ('AGIRU_SYSTEM_SYMBOLS', 'AGIRU_NATIVE_AUDIT'):
            environment.pop(name, None)
        with tempfile.TemporaryDirectory() as folder:
            environment['B'] = folder
            result = subprocess.run(['bash', 'test/transpiler/native-consumers.sh'], cwd=root,
                                    env=environment, text=True, capture_output=True, check=False)
        self.assertEqual(result.returncode, 2, result.stderr)
        self.assertIn('explicit AGIRU_SYSTEM_SYMBOLS and matching AGIRU_NATIVE_AUDIT', result.stderr)

    def test_native_consumer_denominator_rejects_missing_duplicate_and_changed_identity(self):
        root = Path(__file__).resolve().parents[2]
        entries = json.loads((root / 'test/transpiler/native-binding/consumers.json').read_text())
        command = ['bash', 'test/transpiler/native-consumers.sh', '--validate']
        result = subprocess.run(command, cwd=root, text=True, capture_output=True, check=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        changed = [dict(entry) for entry in entries]
        changed[0]['id'] = 1
        with tempfile.TemporaryDirectory() as folder:
            fixture = Path(folder) / 'consumers.json'
            for mutant in (entries[1:], entries + [entries[0]], changed):
                with self.subTest(mutant=mutant):
                    fixture.write_text(json.dumps(mutant))
                    result = subprocess.run(command + [str(fixture)], cwd=root,
                                            text=True, capture_output=True, check=False)
                    self.assertNotEqual(result.returncode, 0, result.stderr)

    def test_native_binding_audit_builds_runtime_and_compiler_before_the_fixture(self):
        root = Path(__file__).resolve().parents[2]
        environment = os.environ.copy()
        for name in ('MAKEFLAGS', 'MFLAGS', 'MAKEOVERRIDES'):
            environment.pop(name, None)
        with tempfile.TemporaryDirectory() as folder:
            result = subprocess.run(['make', '--no-print-directory', '-n', 'native-bindings',
                                     f'B={folder}', 'JOBS=2'], cwd=root, env=environment,
                                    text=True, capture_output=True, check=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        runner = result.stdout.index('test/transpiler/native-bindings.sh')
        self.assertLess(result.stdout.index('--target agiru_rt'), runner)
        self.assertLess(result.stdout.index('--target agirutc'), runner)

    def test_native_binding_audit_requires_an_explicit_package(self):
        root = Path(__file__).resolve().parents[2]
        environment = os.environ.copy()
        environment.pop('AGIRU_SYSTEM_SYMBOLS', None)
        with tempfile.TemporaryDirectory() as folder:
            environment['B'] = folder
            result = subprocess.run(['bash', 'test/transpiler/native-bindings.sh'], cwd=root,
                                    env=environment, text=True, capture_output=True, check=False)
        self.assertEqual(result.returncode, 2, result.stderr)
        self.assertIn('explicit AGIRU_SYSTEM_SYMBOLS is required', result.stderr)

    def test_native_binding_audit_refuses_changed_originals_before_compiling(self):
        root = Path(__file__).resolve().parents[2]
        with tempfile.TemporaryDirectory() as folder:
            package = symbols.publish(self.package(), '28.4.1.0', 'url', 'System.app', Path(folder))
            changed = package / 'src/Virtual Tables/Fixture.Table.al'
            changed.write_text('changed')
            environment = dict(os.environ, B=folder, AGIRU_SYSTEM_SYMBOLS=str(package),
                               CXX='must-not-start-a-compiler')
            result = subprocess.run(['bash', 'test/transpiler/native-bindings.sh'], cwd=root,
                                    env=environment, text=True, capture_output=True, check=False)
            self.assertNotEqual(result.returncode, 0)
            self.assertNotIn('must-not-start-a-compiler', result.stderr)
            self.assertEqual(changed.read_text(), 'changed')
            self.assertFalse((Path(folder) / 'native-bindings.latest').exists())

    def test_native_report_fixture_builds_runtime_without_an_existing_build(self):
        root = Path(__file__).resolve().parents[2]
        environment = os.environ.copy()
        for name in ('MAKEFLAGS', 'MFLAGS', 'MAKEOVERRIDES'):
            environment.pop(name, None)
        with tempfile.TemporaryDirectory() as folder:
            result = subprocess.run(['make', '--no-print-directory', '-n', 'native-report-layouts',
                                     f'B={folder}', 'JOBS=2'], cwd=root, env=environment,
                                    text=True, capture_output=True, check=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        runtime = result.stdout.index('--target agiru_rt')
        runner = result.stdout.index('test/reporting/native-report-layouts.sh')
        self.assertLess(runtime, runner)

    def test_verification_entrypoint_is_offline_and_read_only(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            package = symbols.publish(self.package(), '28.4.1.0', 'url', 'System.app', root)
            before = {str(path): (path.read_bytes(), path.stat().st_mtime_ns)
                      for path in package.rglob('*') if path.is_file()}
            output = io.StringIO()
            with patch.object(sys, 'argv', ['fetch_symbols.py', '--verify', str(package)]), \
                    patch.object(symbols, 'curl', side_effect=AssertionError('unexpected network')), \
                    redirect_stdout(output):
                symbols.main()
            self.assertEqual(json.loads(output.getvalue())['package_sha256'],
                             symbols.verify_package(package)['package_sha256'])
            self.assertEqual(before, {str(path): (path.read_bytes(), path.stat().st_mtime_ns)
                                     for path in package.rglob('*') if path.is_file()})
            changed = package / 'src/Virtual Tables/Fixture.Table.al'
            changed.write_text('changed')
            with patch.object(sys, 'argv', ['fetch_symbols.py', '--verify', str(package)]), \
                    redirect_stderr(io.StringIO()), self.assertRaises(SystemExit):
                symbols.main()
            self.assertEqual(changed.read_text(), 'changed')

    @staticmethod
    def package(extra=(), manifest=None, source=True):
        manifest = manifest if manifest is not None else (
            '<Package xmlns="http://schemas.microsoft.com/navx/2015/manifest">'
            '<App Id="fixture-id" Name="System" Publisher="Microsoft" '
            'Version="28.0.1.0" Runtime="17.0" Target="OnPrem"/></Package>')
        image = io.BytesIO()
        with zipfile.ZipFile(image, 'w', zipfile.ZIP_DEFLATED) as archive:
            archive.writestr('NavxManifest.xml', manifest)
            archive.writestr('SymbolReference.json', b'\xef\xbb\xbf{}')
            if source:
                archive.writestr('src/Virtual%2520Tables/Fixture.Table.al',
                                 'namespace System.Reflection; table 2000000001 Fixture {}')
            for name, data in extra:
                archive.writestr(name, data)
        return b'NAVX' + bytes(36) + image.getvalue()

    def test_original_navx_and_declared_identity_are_preserved(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            app = self.package()
            destination = symbols.publish(app, '28.4.1.0', 'fixture-url', 'System.app', root)
            ledger = symbols.verify_package(destination)
            self.assertEqual((destination / 'System.app').read_bytes(), app)
            self.assertEqual(ledger['package_sha256'], hashlib.sha256(app).hexdigest())
            self.assertEqual(destination, root / '28.4.1.0' / ledger['package_sha256'])
            self.assertEqual(ledger['package_bytes'], len(app))
            self.assertEqual(ledger['bc_version'], '28.4.1.0')
            self.assertEqual(ledger['identity']['Version'], '28.0.1.0')
            self.assertEqual(ledger['source_url'], 'fixture-url')
            self.assertEqual(ledger['source_entry'], 'System.app')
            self.assertEqual(len(ledger['files']), 4)
            self.assertTrue((destination / 'src/Virtual Tables/Fixture.Table.al').is_file())
            original_times = {path: path.stat().st_mtime_ns for path in destination.rglob('*')}
            self.assertEqual(symbols.publish(app, '28.4.1.0', 'fixture-url', 'System.app', root),
                             destination)
            self.assertEqual(original_times,
                             {path: path.stat().st_mtime_ns for path in destination.rglob('*')})

    def test_fetch_entrypoint_keeps_original_bytes_and_provenance(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'BC_VERSION').write_text('28.4.1.0\n')
            app = self.package()
            output = root / 'symbols'
            with patch.object(symbols, 'ROOT', root), patch.object(symbols, 'OUT', output), \
                    patch.object(sys, 'argv', ['fetch_symbols.py']), \
                    patch.object(symbols, 'size_of', return_value=1000), \
                    patch.object(symbols, 'central_directory', return_value=(1, b'directory')), \
                    patch.object(symbols, 'entry_named', return_value=('System.app', 40, len(app))), \
                    patch.object(symbols, 'member', return_value=app), redirect_stdout(io.StringIO()):
                symbols.main()
            originals = list(output.rglob('System.app'))
            self.assertEqual(len(originals), 1, 'fetch discarded the original NAVX package')
            self.assertEqual(originals[0].read_bytes(), app)
            ledger = json.loads((originals[0].parent / 'provenance.json').read_text())
            self.assertEqual(ledger['package_sha256'], hashlib.sha256(app).hexdigest())

    def test_explicit_platform_version_does_not_change_the_demo_pin(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            pin = root / 'BC_VERSION'
            pin.write_text('28.4.1.0\n')
            original = pin.read_bytes(), pin.stat().st_mtime_ns
            app = self.package()
            output = root / 'symbols'
            with patch.object(symbols, 'ROOT', root), patch.object(symbols, 'OUT', output), \
                    patch.object(sys, 'argv', ['fetch_symbols.py', '--version', '29.0.2.3']), \
                    patch.object(symbols, 'size_of', return_value=1000) as size, \
                    patch.object(symbols, 'central_directory', return_value=(1, b'directory')), \
                    patch.object(symbols, 'entry_named', return_value=('System.app', 40, len(app))), \
                    patch.object(symbols, 'member', return_value=app), redirect_stdout(io.StringIO()):
                symbols.main()
            size.assert_called_once_with(symbols.CDN + '/onprem/29.0.2.3/platform')
            packages = list(output.rglob('provenance.json'))
            self.assertEqual(len(packages), 1)
            ledger = json.loads(packages[0].read_text())
            self.assertEqual(ledger['bc_version'], '29.0.2.3')
            self.assertEqual(ledger['identity']['Version'], '28.0.1.0')
            self.assertEqual((pin.read_bytes(), pin.stat().st_mtime_ns), original)

    def test_explicit_version_refuses_paths_and_conflicting_verification_before_network(self):
        for arguments in (['--version', '../29.0.1.2'], ['--version', '29.0.1.2/platform'],
                          ['--version', '29.0'], ['--version', '29.0.1.2?x'],
                          ['--version', '29.0.1.2', '--verify', 'package']):
            with self.subTest(arguments=arguments), \
                    patch.object(sys, 'argv', ['fetch_symbols.py', *arguments]), \
                    patch.object(symbols, 'size_of', side_effect=AssertionError('unexpected network')), \
                    redirect_stderr(io.StringIO()), self.assertRaises(SystemExit) as refused:
                symbols.main()
            self.assertEqual(refused.exception.code, 2)

    def test_changed_package_is_refused_and_not_overwritten(self):
        for relative in ('System.app', 'src/Virtual Tables/Fixture.Table.al', 'NavxManifest.xml'):
            with self.subTest(relative=relative), tempfile.TemporaryDirectory() as folder:
                app = self.package()
                root = Path(folder)
                destination = symbols.publish(app, '28.4.1.0', 'url', 'System.app', root)
                changed = destination / relative
                changed.write_bytes(b'changed')
                with self.assertRaises(SystemExit), redirect_stderr(io.StringIO()):
                    symbols.publish(app, '28.4.1.0', 'url', 'System.app', root)
                self.assertEqual(changed.read_bytes(), b'changed')

    def test_missing_identity_or_source_cannot_publish_a_package(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            with self.assertRaises(SystemExit), redirect_stderr(io.StringIO()):
                symbols.publish(self.package(manifest='<Package/>'), '28.4.1.0',
                                'url', 'System.app', root)
            self.assertFalse(any(root.rglob('provenance.json')))
            with self.assertRaises(SystemExit), redirect_stderr(io.StringIO()):
                symbols.publish(self.package(source=False), '28.4.1.0',
                                'url', 'System.app', root)

    def test_frozen_input_preserves_original_and_provenance(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            app = self.package()
            package = symbols.publish(app, '28.4.1.0', 'url', 'System.app', root)
            frozen = root / 'frozen'
            digest = verify.freeze_input(package, frozen, 'System symbols')
            self.assertEqual(digest, verify.digest(package))
            self.assertEqual(symbols.verify_package(frozen), symbols.verify_package(package))
            self.assertEqual((frozen / 'System.app').read_bytes(), app)

    def test_encoded_paths_links_and_collisions_refuse_before_extraction(self):
        link = zipfile.ZipInfo('src/link')
        link.create_system = 3
        link.external_attr = (stat.S_IFLNK | 0o777) << 16
        for extra in (
                (('%252e%252e/escape', 'bad'),), (('/absolute', 'bad'),),
                (('src/a%252Fb.al', 'bad'),), (('src/a%255Cb.al', 'bad'),),
                (('System.app', 'bad'),), (('provenance.json', 'bad'),),
                ((link, '../outside'),),
                (('src/Virtual%20Tables/Fixture.Table.al', 'duplicate'),),
                (('src', 'file and directory'),)):
            with self.subTest(extra=extra), tempfile.TemporaryDirectory() as folder:
                destination = Path(folder) / 'unpacked'
                with self.assertRaises(SystemExit), redirect_stderr(io.StringIO()):
                    symbols.unpack(self.package(extra), destination)
                self.assertFalse(destination.exists())

    def test_existing_extraction_and_symlinks_are_not_replaced(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            existing = root / 'existing'
            existing.mkdir()
            sentinel = existing / 'sentinel'
            sentinel.write_text('user data')
            with self.assertRaises(FileExistsError):
                symbols.unpack(self.package(), existing)
            self.assertEqual(sentinel.read_text(), 'user data')
            destination = symbols.publish(self.package(), '28.4.1.0', 'url', 'System.app', root)
            (destination / 'alias').symlink_to(existing, target_is_directory=True)
            with self.assertRaises(SystemExit), redirect_stderr(io.StringIO()):
                symbols.verify_package(destination)

    def test_publication_refuses_conflicting_origin_and_version_paths(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            app = self.package()
            symbols.publish(app, '28.4.1.0', 'url', 'System.app', root)
            with self.assertRaises(SystemExit), redirect_stderr(io.StringIO()):
                symbols.publish(app, '28.4.1.0', 'other-url', 'System.app', root)
            with self.assertRaises(SystemExit), redirect_stderr(io.StringIO()):
                symbols.publish(app, '../escape', 'url', 'System.app', root)
            original = root / '28.4.1.0' / hashlib.sha256(app).hexdigest()
            other = symbols.publish(self.package(extra=(('src/Other.al', 'namespace System;'),)),
                                    '28.4.1.0', 'url', 'System.app', root)
            original.rename(root / 'saved')
            other.rename(original)
            with self.assertRaises(SystemExit), redirect_stderr(io.StringIO()):
                symbols.publish(app, '28.4.1.0', 'url', 'System.app', root)


@unittest.skipUnless(sys.platform.startswith('linux'), 'ELF foundation link controls')
class FoundationLinkGate(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = SCRIPT.parents[2]
        self.build = (self.root / Path(os.environ.get('B', 'build'))).resolve()
        self.temporary = Path(self.folder.name)
        cache = (self.build / 'CMakeCache.txt').read_text()
        compiler = re.search(r'^CMAKE_CXX_COMPILER:[^=]+=(.+)$', cache, re.M)
        self.assertIsNotNone(compiler, 'configured compiler is missing')
        self.compiler = compiler[1]
        self.foundations = []
        for reaches in sorted((self.root / 'src').glob('*/reaches')):
            dependencies = re.findall(r'^[a-z]+$', reaches.read_text(), re.M)
            if not dependencies:
                self.foundations.append(reaches.parent.name)
        self.assertTrue({'al', 'net', 'db'}.issubset(self.foundations),
                        'a required foundation acquired a tier dependency')

    def run_command(self, arguments):
        return subprocess.run(arguments, cwd=self.build, text=True,
                              capture_output=True, timeout=30)

    def test_each_foundation_links_independently(self):
        source = self.temporary / 'consumer.cpp'
        source.write_text('int main() { return 0; }\n')
        for tier in self.foundations:
            with self.subTest(tier=tier):
                dynamic = self.run_command(['readelf', '-d', f'libagiru_{tier}.so'])
                self.assertEqual(dynamic.returncode, 0, dynamic.stdout + dynamic.stderr)
                self.assertNotRegex(dynamic.stdout, r'Shared library: \[libagiru_',
                                    'a foundation depends on another agiru tier')
                output = self.temporary / tier
                result = self.run_command([
                    self.compiler, '-std=c++23', '-stdlib=libc++', '--rtlib=compiler-rt',
                    '--unwindlib=libunwind', '-fuse-ld=lld-19', str(source), '-Wl,--no-as-needed',
                    '-Wl,--no-allow-shlib-undefined', f'-L{self.build}',
                    f'-Wl,-rpath,{self.build}', f'-lagiru_{tier}', '-o', str(output)])
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertEqual(self.run_command([str(output)]).returncode, 0)

    def test_value_error_headers_do_not_import_transaction_boundaries(self):
        prefix = ('namespace agiru { class Boundaries {}; '
                  'namespace detail { class Scope {}; } }\n')
        source = self.temporary / 'value_error.cpp'
        source.write_text(prefix + '\n'.join(
            f'#include "{header}"' for header in (
                'runtime/ErrorValue.h', 'type/Decimal.h', 'dotnet/Refused.h', 'dotnet/Uri.h',
                'type/JsonToken.h', 'type/Stream.h', 'type/List.h',
                'type/Dictionary.h', 'type/Variant.h')) + '\nint main() { return 0; }\n')
        command = [self.compiler, '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-fsyntax-only', f'-I{self.root / "include"}', str(source)]
        result = self.run_command(command)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        source.write_text(prefix + '#include "runtime/Error.h"\n')
        control = self.run_command(command)
        self.assertNotEqual(control.returncode, 0, control.stdout + control.stderr)
        self.assertIn('redefinition', control.stdout + control.stderr)
        self.assertIn('Boundaries', control.stdout + control.stderr)

    def test_production_foundation_links_reject_an_injected_reverse_edge(self):
        source = self.temporary / 'injected.cpp'
        source.write_text('extern "C" void agiru_forbidden_runtime_edge();\n'
                          'void probe() { agiru_forbidden_runtime_edge(); }\n')
        injected = self.temporary / 'injected.o'
        compiled = self.run_command([self.compiler, '-fPIC', '-c', str(source),
                                     '-o', str(injected)])
        self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
        for tier in self.foundations:
            with self.subTest(tier=tier):
                commands = self.run_command(['ninja', '-t', 'commands', f'libagiru_{tier}.so'])
                self.assertEqual(commands.returncode, 0, commands.stdout + commands.stderr)
                wrapped = commands.stdout.splitlines()[-1]
                self.assertTrue(wrapped.startswith(': && ') and wrapped.endswith(' && :'), wrapped)
                arguments = shlex.split(wrapped[len(': && '):-len(' && :')])
                self.assertIn('--no-undefined', ' '.join(arguments))
                self.assertEqual(arguments.count('-o'), 1)
                output_index = arguments.index('-o') + 1
                self.assertEqual(arguments[output_index], f'libagiru_{tier}.so')
                arguments[output_index] = str(self.temporary / f'broken_{tier}.so')
                arguments = [re.sub(r'(?<=--dependency-file=).*',
                                    str(self.temporary / f'{tier}.link.d'), argument)
                             for argument in arguments]
                result = self.run_command(arguments + [str(injected)])
                self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertIn('agiru_forbidden_runtime_edge', result.stdout + result.stderr)
                unguarded = arguments.copy()
                flag = '--no-undefined' if '--no-undefined' in unguarded else '-Wl,--no-undefined'
                index = unguarded.index(flag)
                if flag == '--no-undefined':
                    self.assertEqual(unguarded[index - 1], '-Xlinker')
                    del unguarded[index - 1:index + 1]
                else:
                    del unguarded[index]
                unguarded[unguarded.index('-o') + 1] = str(self.temporary / f'unguarded_{tier}.so')
                control = self.run_command(unguarded + [str(injected)])
                self.assertEqual(control.returncode, 0, control.stdout + control.stderr)


class TranspilerAttributeCensusGate(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        (self.root / 'source').mkdir()
        (self.root / 'apps.json').write_text(json.dumps({
            'apps': [{'name': 'fixture', 'source': 'source', 'depends': []}]}))
        (self.root / 'scope.json').write_text(json.dumps({
            'include': ['Microsoft.Fixture'], 'exclude': []}))
        build = Path(os.environ.get('B', 'build'))
        self.transpiler = (SCRIPT.parents[2] / build / 'agirutc').resolve()
        self.assertTrue(self.transpiler.is_file(), f'missing transpiler: {self.transpiler}')

    def assert_census(self, procedures, expected, status=0):
        (self.root / 'source/Census.Codeunit.al').write_text(
            'namespace Microsoft.Fixture;\ncodeunit 50140 "Attribute Census"\n{\n'
            'Subtype = Test;\n'
            + procedures + '\n}\n')
        result = subprocess.run([str(self.transpiler), str(self.root),
                                 str(self.root / 'apps.json'), str(self.root / 'generated')],
                                text=True, capture_output=True, timeout=30)
        self.assertEqual(result.returncode, status, result.stdout + result.stderr)
        counts = re.search(r'attributes acted on (\d+) of (\d+) kind\(s\) declared, '
                           r'(\d+) declaration\(s\) acknowledged as no-ops; '
                           r'(\d+) declaration\(s\) of (\d+) kind\(s\) are read and dropped',
                           result.stdout)
        self.assertIsNotNone(counts, result.stdout + result.stderr)
        self.assertEqual(tuple(map(int, counts.groups())), expected)
        return result

    def test_empty_population_is_zero(self):
        self.assert_census('procedure A() begin end;', (0, 0, 0, 0, 0))

    def test_acknowledged_kind_counts_its_observed_declarations(self):
        self.assert_census('[Normal] procedure A() begin end;\n'
                           '[Normal] procedure B() begin end;', (0, 1, 2, 0, 0))

    def test_acted_kind_counts_once_despite_repeated_declarations(self):
        self.assert_census('[Normal] [TryFunction] procedure A() begin end;\n'
                           '[Normal] [TryFunction] procedure B() begin end;', (1, 2, 2, 0, 0))

    def test_known_mixture_partitions_observed_kinds(self):
        self.assert_census('[Normal] [TryFunction] procedure A() begin end;\n'
                           '[Normal] procedure B() begin end;', (1, 2, 2, 0, 0))

    def test_try_function_without_normal_remains_counted_and_refuses(self):
        result = self.assert_census('[TryFunction] procedure A() begin end;',
                                    (1, 1, 0, 0, 0), status=1)
        self.assertIn('[TryFunction] applies to [Normal] methods only', result.stdout)
        self.assertRegex(result.stdout, r'refused\s+1 property declaration')

    def test_try_function_on_test_remains_counted_and_refuses(self):
        result = self.assert_census('[Test] [TryFunction] procedure A() begin end;',
                                    (2, 2, 0, 0, 0), status=1)
        self.assertIn('[TryFunction] applies to [Normal] methods only', result.stdout)
        self.assertRegex(result.stdout, r'codeunits\s+1 of 1 parsed \(1 procedures, 1 \[Test\] methods\)')

    def test_unknown_kind_remains_counted_and_fails(self):
        result = self.assert_census('[FutureAttribute] procedure A() begin end;\n'
                                    '[FutureAttribute] procedure B() begin end;',
                                    (0, 1, 0, 2, 1), status=1)
        self.assertIn('ABORT     2 attribute declaration(s)', result.stdout)

    def test_unknown_mixture_cannot_disappear_into_known_catalogues(self):
        result = self.assert_census('[Normal] [TryFunction] procedure A() begin end;\n'
                                    '[Normal] procedure B() begin end;\n'
                                    '[FutureAttribute] procedure C() begin end;',
                                    (1, 3, 2, 1, 1), status=1)
        self.assertIn('ABORT     1 attribute declaration(s)', result.stdout)


@unittest.skipUnless(shutil.which('clang++-19'), 'tree gate requires clang++-19')
class TreeSyntaxGate(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        for name in ('scripts', 'cmake', 'include', 'apps/Fixture'):
            (self.root / name).mkdir(parents=True)
        repository = SCRIPT.parents[2]
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


@unittest.skipUnless(shutil.which('clang++-19'), 'gap gate requires clang++-19')
class FirstGapGate(unittest.TestCase):
    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        for name in ('scripts', 'cmake', 'include', 'apps/Fixture'):
            (self.root / name).mkdir(parents=True)
        repository = SCRIPT.parents[2]
        shutil.copy2(repository / 'Makefile', self.root / 'Makefile')
        for name in ('first_gap.sh', 'tree_syntax.sh', 'tree_keys.py'):
            shutil.copy2(repository / 'scripts' / name, self.root / 'scripts' / name)
        (self.root / 'cmake/Precompiled.h').write_text('// isolated gap gate fixture\n')
        (self.root / 'apps.json').write_text(json.dumps({'apps': [{'name': 'Fixture'}]}))

    def run_gap(self, source=False, sweep=False, make=False, path='apps', environment=None):
        env = dict(os.environ, SOURCE='1' if source else '', SWEEP='1' if sweep else '',
                   JOBS='2')
        for name in ('MAKEFLAGS', 'MFLAGS', 'MAKEOVERRIDES'):
            env.pop(name, None)
        if environment:
            env.update(environment)
        command = (['make', '--no-print-directory', '-o', 'db', 'gap'] if make else
                   ['sh', 'scripts/first_gap.sh', path])
        return subprocess.run(command, cwd=self.root, env=env, text=True,
                              capture_output=True, timeout=30)

    def write(self, relative, content):
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content)
        return path

    def test_make_source_skips_failing_header_census_and_counts_extra_bodies(self):
        self.write('apps/Fixture/Unused.h', '#error unused header\n')
        self.write('apps/Fixture/A.cpp', 'int a() { return 0; }\n')
        self.write('apps/Fixture/B.cpp', 'int b() { return 0; }\n')
        self.write('apps/shared/C.cpp', 'int c() { return 0; }\n')
        result = self.run_gap(source=True, make=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('all 3 generated bodies compile', result.stdout)
        self.assertEqual([str(Path(path).relative_to(self.root)) for path in
                          (self.root / 'build/first-gap/files').read_text().splitlines()],
                         ['apps/Fixture/A.cpp', 'apps/Fixture/B.cpp', 'apps/shared/C.cpp'])
        self.assertFalse((self.root / 'build/tree-syntax').exists())

    def test_make_header_sweep_does_not_build_a_census(self):
        self.write('apps/Fixture/A.h', '#pragma once\nstruct A {};\n')
        result = self.run_gap(sweep=True, make=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('all 1 generated headers compile', result.stdout)
        self.assertFalse((self.root / 'build/tree-syntax').exists())

    def test_ranked_make_still_builds_the_header_census(self):
        self.write('apps/Fixture/Invalid.h', '#error ranked preflight\n')
        result = self.run_gap(make=True)
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('ranked preflight', (self.root / 'build/tree-syntax/errors').read_text())
        self.assertFalse((self.root / 'build/first-gap/files').exists())

    def test_first_failed_body_keeps_the_complete_population(self):
        self.write('apps/Fixture/A.cpp', 'int a() { return 0; }\n')
        self.write('apps/Fixture/B.cpp', '#error broken body\n')
        self.write('apps/Fixture/C.cpp', 'int c() { return 0; }\n')
        result = self.run_gap(source=True, sweep=True)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('1 of 3 generated bodies compile, then apps/Fixture/B.cpp', result.stdout)
        self.assertIn('broken body', result.stderr)
        self.assertEqual(len((self.root / 'build/first-gap/files').read_text().splitlines()), 3)

    def test_warning_only_body_is_red(self):
        self.write('apps/Fixture/Warning.cpp',
                   '[[deprecated]] void old_api() {}\nvoid consumer() { old_api(); }\n')
        result = self.run_gap(source=True)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('-Werror', result.stderr)
        self.assertIn('0 of 1 generated bodies compile', result.stdout)

    def test_warning_only_header_is_red(self):
        self.write('apps/Fixture/Warning.h', '#pragma once\n'
                   '[[deprecated]] inline void old_api() {}\n'
                   'inline void consumer() { old_api(); }\n')
        result = self.run_gap(sweep=True)
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('-Werror', result.stderr)

    def test_failed_or_warning_only_pch_is_red(self):
        self.write('apps/Fixture/A.cpp', 'int a() { return 0; }\n')
        for content in ('#error broken PCH\n',
                        '[[deprecated]] inline void old_api() {}\n'
                        'inline void consumer() { old_api(); }\n'):
            with self.subTest(content=content):
                self.write('cmake/Precompiled.h', content)
                result = self.run_gap(source=True)
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('does not precompile', result.stderr)
                self.assertTrue((self.root / 'build/first-gap/pch.log').read_text())

    def test_empty_header_and_body_populations_are_red(self):
        for source in (False, True):
            with self.subTest(source=source):
                result = self.run_gap(source=source, sweep=True)
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn('no generated ' + ('bodies' if source else 'headers'), result.stderr)

    def test_missing_generated_tree_is_red(self):
        result = self.run_gap(source=True, path='missing-apps')
        self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
        self.assertIn('does not exist', result.stderr)

    def test_missing_declared_app_cannot_shrink_the_population(self):
        self.write('apps/Fixture/A.cpp', 'int a() { return 0; }\n')
        self.write('apps.json', json.dumps({'apps': [{'name': 'Fixture'}, {'name': 'Missing'}]}))
        result = self.run_gap(source=True)
        self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
        self.assertIn('declared app Missing does not exist', result.stderr)
        self.assertNotIn('compile.', result.stdout)

    def test_unreadable_malformed_and_empty_manifests_are_red(self):
        self.write('apps/Fixture/A.cpp', 'int a() { return 0; }\n')
        manifest = self.root / 'apps.json'
        manifest.unlink()
        for content in (None, '{', '{"apps": []}'):
            with self.subTest(content=content):
                if content is not None:
                    manifest.write_text(content)
                result = self.run_gap(source=True)
                self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
                self.assertNotIn('compile.', result.stdout)

    def test_header_sweep_includes_generated_support_directories(self):
        for name in ('Fixture', 'shared', 'absent'):
            self.write(f'apps/{name}/{name}.h', f'#pragma once\nstruct {name} {{}};\n')
        result = self.run_gap(sweep=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('all 3 generated headers compile', result.stdout)

    def test_compiler_failure_without_diagnostics_is_red(self):
        self.write('apps/Fixture/A.cpp', 'int a() { return 0; }\n')
        compiler = shlex.quote(shutil.which('clang++-19'))
        wrapper = self.write('bin/clang++-19', '#!/bin/sh\n'
                             'case " $* " in\n'
                             '  *" -x c++-header "*) exec ' + compiler + ' "$@";;\n'
                             'esac\nexit 37\n')
        wrapper.chmod(0o755)
        result = self.run_gap(source=True,
                              environment={'CXX': str(wrapper),
                                           'PATH': str(wrapper.parent) + os.pathsep + os.environ['PATH']})
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn('0 of 1 generated bodies compile', result.stdout)

    def test_ranked_selection_uses_the_largest_root(self):
        self.write('apps/Fixture/A.h', '#error small root\n')
        self.write('apps/Fixture/B.h', '#error largest root\n')
        self.write('build/tree-syntax/roots',
                   'one\tapps/Fixture/A.h\tsmall\n'
                   'two\tapps/Fixture/B.h\tlarge\n'
                   'three\tapps/Fixture/B.h\tlarge\n')
        result = self.run_gap(make=True)
        self.assertNotEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('apps/Fixture/B.h blocks 2 of 3 failing headers, 2 root(s)', result.stdout)
        self.assertIn('largest root', result.stderr)

    def test_missing_census_root_is_red(self):
        self.write('build/tree-syntax/roots', 'one\tapps/Fixture/Missing.h\tmissing\n')
        result = self.run_gap()
        self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
        self.assertIn('census root apps/Fixture/Missing.h does not exist', result.stderr)

    def test_spent_census_is_not_a_complete_tree_pass(self):
        self.write('apps/Fixture/A.h', '#pragma once\nstruct A {};\n')
        self.write('build/tree-syntax/roots', 'one\tapps/Fixture/A.h\told failure\n')
        result = self.run_gap()
        self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
        self.assertIn('census is spent', result.stdout)


class SeedTransferGate(unittest.TestCase):
    def test_large_sql_uses_stdin_not_operating_system_arguments(self):
        with tempfile.TemporaryDirectory() as folder:
            program = Path(folder) / 'podman'
            program.write_text('''#!/bin/sh
bytes=$(wc -c)
if [ "$bytes" -lt 262144 ]; then exit 17; fi
printf 'delivered\\n'
''')
            program.chmod(0o755)
            with patch.dict(os.environ, PATH=f'{folder}:{os.environ["PATH"]}'):
                result, why = seed.psql('fixture', ' ' * 262144 + 'SELECT 1;')
        self.assertEqual((result, why), ('delivered\n', ''))

    def readback(self, source, target):
        database = os.environ.get('AGIRU_TEST_DSN',
                                  'postgresql://agiru:agiru@localhost:5433/agiru_gate')
        with patch.object(seed, 'psql_command', return_value=[
                'psql', '-X', '-d', database, '-v', 'ON_ERROR_STOP=1']):
            return seed.verify_rows(source, target, 'source', 'target', None, None)

    def test_readback_preserves_typed_values_and_ignores_row_order_and_numeric_scale(self):
        source = ('\\copy (SELECT to_jsonb(ROW(trim_scale(number), text)) FROM '
                  "(VALUES (9007199254740993::numeric, 'Möbel'), (1.20, 'two')) "
                  'AS rows(number, text)) TO STDOUT')
        target = ('\\copy (SELECT to_jsonb(ROW(trim_scale(number), text)) FROM '
                  "(VALUES (1.20000000000000000000::numeric, 'two'), "
                  "(9007199254740993, 'Möbel')) AS rows(number, text)) TO STDOUT")
        self.assertEqual(self.readback(source, target), (0, 0, 'COPY 2\n'))
        changed = target.replace('9007199254740993', '9007199254740992')
        self.assertEqual(self.readback(source, changed),
                         (1, 1, 'ERROR: typed source/target rows differ'))
        self.assertEqual(self.readback(source, target.replace('Möbel', 'Mobel')),
                         (1, 1, 'ERROR: typed source/target rows differ'))

    def test_readback_refuses_missing_and_extra_rows_and_query_failures(self):
        one = '\\copy (SELECT to_jsonb(ROW(1))) TO STDOUT'
        empty = '\\copy (SELECT to_jsonb(ROW(1)) WHERE false) TO STDOUT'
        self.assertEqual(self.readback(empty, empty), (0, 0, 'COPY 0\n'))
        self.assertEqual(self.readback(one, empty),
                         (1, 1, 'ERROR: typed source/target rows differ'))
        self.assertEqual(self.readback(empty, one),
                         (1, 1, 'ERROR: typed source/target rows differ'))
        status, _, reason = self.readback('SELECT 1 / 0;', one)
        self.assertNotEqual(status, 0)
        self.assertIn('division by zero', reason)

    def test_time_readback_compares_time_not_the_sql_server_carrier_date(self):
        values = seed.canonical('value', 'time without time zone')
        source = (f'\\copy (SELECT to_jsonb(ROW({values})) FROM '
                  "(VALUES ('1754-01-01 12:40:06.917'::timestamp), "
                  "('1753-01-01 00:00:00'::timestamp)) AS rows(value)) TO STDOUT")
        target = (f'\\copy (SELECT to_jsonb(ROW({values})) FROM '
                  "(VALUES ('12:40:06.917'::time), ('00:00:00'::time)) AS rows(value)) TO STDOUT")
        self.assertEqual(self.readback(source, target), (0, 0, 'COPY 2\n'))
        self.assertEqual(self.readback(source, target.replace('06.917', '06.918')),
                         (1, 1, 'ERROR: typed source/target rows differ'))

    def test_datetime_readback_never_discards_the_date(self):
        values = seed.canonical('value', 'timestamp without time zone')
        source = (f'\\copy (SELECT to_jsonb(ROW({values})) FROM '
                  "(VALUES ('1754-01-01 12:40:06.917'::timestamp)) AS rows(value)) TO STDOUT")
        self.assertEqual(self.readback(source, source), (0, 0, 'COPY 1\n'))
        self.assertEqual(self.readback(source, source.replace('1754-01-01', '1754-01-02')),
                         (1, 1, 'ERROR: typed source/target rows differ'))

    def test_recovery_retains_nonce_and_refuses_changed_source_or_finished_seed(self):
        for status in ('building', 'failed'):
            details = {'id': 'new', 'artefact_sha256': 'original'}
            with patch.object(seed, 'psql', return_value=(
                    status + '\t{"id":"existing","artefact_sha256":"original"}\n', '')):
                seed.resume_seed('target', details, seed.Endpoint())
            self.assertEqual(details['id'], 'existing')
        for record in [
                'building\t{"id":"existing","artefact_sha256":"changed"}\n',
                'complete\t{"id":"existing","artefact_sha256":"original"}\n']:
            with self.subTest(record=record), patch.object(seed, 'psql', return_value=(record, '')), \
                    redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit):
                    seed.resume_seed('target', details, seed.Endpoint())

    def test_failed_seed_recovery_rechecks_every_table_and_never_copies(self):
        tables = {'A': ['id'], 'B': ['id']}
        for second, expected in [((0, 0, 'COPY 2\n'), 'complete'),
                                 ((1, 1, 'ERROR: typed source/target rows differ'), 'failed')]:
            with self.subTest(expected=expected), \
                    patch.object(seed, 'columns', side_effect=[tables, tables]), \
                    patch.object(seed, 'provenance', return_value={'id': 'new'}), \
                    patch.object(seed, 'psql', return_value=('failed\t{"id":"existing"}\n', '')), \
                    patch.object(seed, 'verify_rows', side_effect=[(0, 0, 'COPY 1\n'), second]) as read, \
                    patch.object(seed, 'transfer') as copy, \
                    patch.object(seed, 'begin_seed') as begin, \
                    patch.object(seed, 'reconcile_rowversions'), \
                    patch.object(seed, 'finish_seed') as finish, \
                    patch.object(sys, 'argv', ['seed_demo.py', '--verify']), \
                    redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
                if expected == 'failed':
                    with self.assertRaises(SystemExit):
                        seed.main()
                else:
                    seed.main()
            self.assertEqual(read.call_count, 2)
            copy.assert_not_called()
            begin.assert_not_called()
            self.assertEqual(finish.call_args.args[1]['id'], 'existing')
            self.assertTrue(finish.call_args.args[1]['typed_readback'])
            self.assertEqual(finish.call_args.kwargs['status'], expected)

    def test_independent_container_commands_preserve_identity_and_failure_handling(self):
        source = seed.Endpoint('source-container', 'source-user')
        target = seed.Endpoint('target-container', 'target-user')
        reading = seed.psql_command('source-db', source)
        writing = seed.psql_command('target-db', target, stdin=True)
        self.assertEqual(reading[:5], ['podman', 'exec', '--user', 'source-user',
                                      'source-container'])
        self.assertEqual(writing[:6], ['podman', 'exec', '-i', '--user', 'target-user',
                                      'target-container'])
        self.assertEqual(reading[-2:], ['-d', 'source-db'])
        self.assertEqual(writing[-2:], ['-d', 'target-db'])
        self.assertIn('ON_ERROR_STOP=1', reading)
        self.assertIn('ON_ERROR_STOP=1', writing)
        self.assertIn('-X', writing)

    def test_column_types_do_not_leak_between_equal_database_names(self):
        source = seed.Endpoint('source')
        target = seed.Endpoint('target')
        with patch.dict(seed.TYPES, clear=True), \
                patch.object(seed, 'psql', side_effect=[
                    ('A\ttimestamp\tbytea\n', ''), ('A\ttimestamp\tbigint\n', '')]):
            seed.columns('same-name', 'company', source)
            seed.columns('same-name', 'public', target)
            self.assertEqual(seed.column_kind('same-name', 'A', 'timestamp', source), 'bytea')
            self.assertEqual(seed.column_kind('same-name', 'A', 'timestamp', target), 'bigint')

    def rowversion_query(self, value):
        with patch.dict(seed.TYPES, {
                (seed.Endpoint(), 'source', 'A', 'timestamp'): 'bytea'}, clear=True):
            expression = seed.blanked('source', 'A', 'timestamp', 'bigint', 'b.')
        sql = f'SELECT ({expression})::bigint FROM (SELECT {value} AS timestamp) AS b'
        return subprocess.run(['psql', '-XAt', '-v', 'ON_ERROR_STOP=1', '-d',
                               os.environ.get('AGIRU_TEST_DSN',
                                   'postgresql://agiru:agiru@localhost:5433/agiru_gate'),
                               '-c', sql], capture_output=True, text=True, check=False)

    def test_original_binary_rowversions_preserve_full_integer_values(self):
        for encoded, expected in [('0000000000000000', 0), ('0000000000000001', 1),
                                  ('000000000001321b', 78363),
                                  ('0102030405060708', 72623859790382856),
                                  ('7fffffffffffffff', 9223372036854775807)]:
            with self.subTest(encoded=encoded):
                result = self.rowversion_query(f"decode('{encoded}', 'hex')")
                self.assertEqual(result.returncode, 0, result.stderr)
                self.assertEqual(result.stdout.strip(), str(expected))

    def test_malformed_or_out_of_range_binary_rowversions_refuse(self):
        for encoded in ['', '01', '000000000000000001', '8000000000000000',
                        'ffffffffffffffff']:
            with self.subTest(encoded=encoded):
                result = self.rowversion_query(f"decode('{encoded}', 'hex')")
                self.assertNotEqual(result.returncode, 0, result.stdout)
                self.assertIn('invalid input syntax for type bigint', result.stderr)

    def test_only_the_original_timestamp_has_a_binary_integer_adapter(self):
        with patch.dict(seed.TYPES, {
                (seed.Endpoint(), 'source', 'A', 'Payload'): 'bytea'}, clear=True):
            self.assertEqual(seed.blanked('source', 'A', 'Payload', 'bigint', 'b.'),
                             'b."Payload"')

    def test_imported_rowversions_advance_without_resetting_the_counter(self):
        target = seed.Endpoint('native', 'agiru')
        with patch.dict(seed.TYPES, {
                (target, 'target', 'A', 'timestamp'): 'bigint',
                (target, 'target', 'Blob', 'timestamp'): 'bytea'}, clear=True), \
                patch.object(seed, 'psql', return_value=('', '')) as run:
            seed.reconcile_rowversions('target', {'A': [], 'Blob': []}, target)
        query = run.call_args.args[1]
        self.assertIn('GREATEST(agiru_platform.last_rowversion_v1(), value)', query)
        self.assertIn('MAX("timestamp")', query)
        self.assertIn('FROM public."A"', query)
        self.assertNotIn('FROM public."Blob"', query)
        self.assertIn('WHERE value > 0', query)
        self.assertEqual(run.call_args.kwargs['endpoint'], target)

    def test_source_database_cannot_be_the_import_target(self):
        with patch.object(sys, 'argv', ['seed_demo.py', '--into', seed.SOURCE]), \
                patch.object(seed, 'columns') as read, redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                seed.main()
        read.assert_not_called()

    def test_ambiguous_table_folds_refuse_before_writes(self):
        with patch.object(sys, 'argv', ['seed_demo.py']), \
                patch.object(seed, 'columns', side_effect=[{'A_': ['id']},
                                                         {'A.': ['id'], 'A/': ['id']}]), \
                patch.object(seed, 'begin_seed') as begin, redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                seed.main()
        begin.assert_not_called()

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
                patch.object(seed, 'finish_seed') as finish, \
                patch.object(sys, 'argv', ['seed_demo.py']), \
                redirect_stdout(output), redirect_stderr(errors):
            with self.assertRaises(SystemExit) as refused:
                seed.main()
        self.assertEqual(refused.exception.code, 1)
        self.assertIn('1 table(s) carry 1 row(s)', output.getvalue())
        self.assertIn('the seed is incomplete', errors.getvalue())
        self.assertEqual(finish.call_args.kwargs['status'], 'failed')
        self.assertEqual(finish.call_args.args[1]['refused_tables'],
                         [('B', 'reader 17, writer 0: ERROR: source refused')])


class ReportRegistryHeaderGate(unittest.TestCase):
    def prove_contract(self, before, after):
        repository = Path(__file__).resolve().parents[2]
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            header = repository / 'include/runtime/ReportRegistry.h'
            source = root / 'contract.cpp.in'
            source.write_text('''#include "runtime/ReportRegistry.h"
#include <cstddef>
#include <string_view>
#include <type_traits>
struct PreviousEntry {
  agiru::ReportId id;
  std::string_view name;
  void (*run)(const agiru::ReportRequest&);
};
static_assert(std::is_same_v<decltype(agiru::ReportEntry::run), decltype(PreviousEntry::run)>);
static_assert(std::is_standard_layout_v<agiru::ReportEntry>);
static_assert(sizeof(agiru::ReportEntry) == sizeof(PreviousEntry));
static_assert(alignof(agiru::ReportEntry) == alignof(PreviousEntry));
static_assert(offsetof(agiru::ReportEntry, id) == offsetof(PreviousEntry, id));
static_assert(offsetof(agiru::ReportEntry, name) == offsetof(PreviousEntry, name));
static_assert(offsetof(agiru::ReportEntry, run) == offsetof(PreviousEntry, run));
''')
            arguments = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra',
                         '-Wpedantic', '-Werror', '-x', 'c++', '-fsyntax-only', str(source)]
            compiled = subprocess.run(arguments + ['-I' + str(repository / 'include')],
                                      capture_output=True, text=True, timeout=30)
            self.assertEqual(compiled.returncode, 0, compiled.stderr)
            original = header.read_text()
            self.assertEqual(original.count(before), 1)
            (root / 'runtime').mkdir()
            (root / 'runtime/ReportRegistry.h').write_text(original.replace(before, after))
            control = subprocess.run(arguments + ['-I' + str(root), '-I' + str(repository / 'include')],
                                     capture_output=True, text=True, timeout=30)
            self.assertNotEqual(control.returncode, 0)
            self.assertIn('static assertion failed', control.stderr)

    def test_entrypoint_keeps_the_original_const_request_signature(self):
        self.prove_contract('void (*run)(const ReportRequest &request);',
                            'void (*run)(ReportRequest &request);')

    def test_entry_keeps_original_field_offsets_and_layout(self):
        self.prove_contract('  ReportId id;', '  std::string_view extra;\n  ReportId id;')


class GeneratedRegistrationLinkGate(unittest.TestCase):
    def prove_registration(self, kind):
        repository = Path(__file__).resolve().parents[2]
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            shutil.copyfile(repository / 'cmake/GeneratedLink.cmake', root / 'GeneratedLink.cmake')
            (root / 'registry.cpp').write_text(
                'namespace { int stored = 0; }\n'
                'int Remember(int value) { stored = value; return value; }\n'
                'int Observed() { return stored; }\n')
            (root / 'generated.cpp').write_text(
                'int Remember(int);\n'
                'namespace { [[maybe_unused]] const int registration = Remember(42); }\n')
            (root / 'consumer.cpp').write_text(
                'int Observed();\nint main() { return Observed() == 42 ? 0 : 1; }\n')
            (root / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.28)
project(GeneratedRegistration CXX)
set(CMAKE_CXX_STANDARD 23)
add_compile_options(-stdlib=libc++ -Wall -Wextra -Wpedantic -Werror)
add_link_options(-stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
include(GeneratedLink.cmake)
add_library(registry SHARED registry.cpp)
add_library(generated ''' + kind + ''' generated.cpp)
target_link_libraries(generated PRIVATE registry)
add_library(dependency INTERFACE)
target_link_libraries(dependency INTERFACE generated)
add_executable(retained consumer.cpp)
target_link_options(retained PRIVATE "LINKER:--as-needed")
target_link_libraries(retained PRIVATE dependency registry)
agiru_link_generated(retained generated)
add_executable(dropped consumer.cpp)
target_link_options(dropped PRIVATE "LINKER:--as-needed")
target_link_libraries(dropped PRIVATE generated registry)
''')
            build = root / 'build'
            configured = subprocess.run(['cmake', '-S', str(root), '-B', str(build),
                                         '-G', 'Ninja', '-DCMAKE_CXX_COMPILER=clang++-19'],
                                        capture_output=True, text=True, timeout=60)
            self.assertEqual(configured.returncode, 0, configured.stdout + configured.stderr)
            compiled = subprocess.run(['cmake', '--build', str(build), '-j', '2'],
                                      capture_output=True, text=True, timeout=60)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            retained = subprocess.run([str(build / 'retained')], capture_output=True, timeout=10)
            self.assertEqual(retained.returncode, 0, retained.stderr)
            dropped = subprocess.run([str(build / 'dropped')], capture_output=True, timeout=10)
            self.assertEqual(dropped.returncode, 1, dropped.stderr)
            if kind == 'SHARED':
                for binary, expected in (('retained', True), ('dropped', False)):
                    needed = subprocess.run(['readelf', '-d', str(build / binary)],
                                            capture_output=True, text=True, check=True)
                    self.assertEqual('libgenerated.so' in needed.stdout, expected)

    def test_shared_registration_survives_as_needed_and_transitive_edges(self):
        self.prove_registration('SHARED')

    def test_static_registration_survives_archive_extraction(self):
        self.prove_registration('STATIC')


class UnlinkedProceduresGate(unittest.TestCase):
    def setUp(self):
        self.repository = Path(__file__).resolve().parents[2]
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.flags = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra',
                      '-Wpedantic', '-Werror']
        self.links = ['--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19']

    def shared(self, name, source):
        path = self.root / f'{name}.cpp'
        path.write_text(source)
        library = self.root / f'lib{name}.so'
        result = subprocess.run(self.flags + self.links + ['-fPIC', '-shared', str(path), '-o', str(library)],
                                capture_output=True, text=True, timeout=30)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return library

    def generate(self, library, definitions=()):
        self.output = self.root / 'unlinked.cpp'
        return subprocess.run([sys.executable, str(self.repository / 'scripts/unlinked.py'),
                               str(self.output), str(library), *map(str, definitions)],
                              capture_output=True, text=True, timeout=30)

    def test_no_missing_procedures_emit_no_unused_helper(self):
        library = self.shared('slice', 'namespace agiru { void Present() {} }\n')
        generated = self.generate(library)
        self.assertEqual(generated.returncode, 0, generated.stderr)
        self.assertIn('0 AL procedure(s)', generated.stdout)
        self.assertNotIn('void Unlinked', self.output.read_text())
        compiled = subprocess.run(self.flags + ['-c', str(self.output), '-o', str(self.root / 'empty.o')],
                                  capture_output=True, text=True, timeout=30)
        self.assertEqual(compiled.returncode, 0, compiled.stderr)
        self.output.write_text('namespace { void Unlinked() {} }\n')
        control = subprocess.run(self.flags + ['-c', str(self.output), '-o', str(self.root / 'control.o')],
                                 capture_output=True, text=True, timeout=30)
        self.assertNotEqual(control.returncode, 0)
        self.assertIn('-Wunused-function', control.stderr)

    def test_missing_procedure_raises_but_linked_definition_is_not_replaced(self):
        library = self.shared('slice', 'namespace agiru { void Missing(); void Call() { Missing(); } }\n')
        generated = self.generate(library)
        self.assertEqual(generated.returncode, 0, generated.stderr)
        self.assertIn('1 AL procedure(s)', generated.stdout)
        consumer = self.root / 'consumer.cpp'
        consumer.write_text('''#include "runtime/ErrorValue.h"
#include <string_view>
namespace agiru { void Call(); }
int main() {
  try { agiru::Call(); }
  catch (const agiru::Error& error) {
    return std::string_view(error.what()).find("agiru::Missing()") == std::string_view::npos;
  }
  return 2;
}
''')
        executable = self.root / 'consumer'
        compiled = subprocess.run(self.flags + self.links + ['-I' + str(self.repository / 'include'),
            str(consumer), str(self.output), str(library), '-Wl,--export-dynamic',
            '-Wl,-rpath,' + str(self.root), '-o', str(executable)],
            capture_output=True, text=True, timeout=30)
        self.assertEqual(compiled.returncode, 0, compiled.stderr)
        executed = subprocess.run([str(executable)], capture_output=True, timeout=10)
        self.assertEqual(executed.returncode, 0, executed.stderr)
        definition = self.shared('platform', 'namespace agiru { void Missing() {} }\n')
        resolved = self.generate(library, [definition])
        self.assertEqual(resolved.returncode, 0, resolved.stderr)
        self.assertIn('0 AL procedure(s)', resolved.stdout)
        self.assertNotIn('agiru_unlinked_', self.output.read_text())

    def test_missing_data_refuses_instead_of_emitting_a_function(self):
        library = self.shared('slice', 'namespace agiru { extern int MissingData; int Call() { return MissingData; } }\n')
        refused = self.generate(library)
        self.assertNotEqual(refused.returncode, 0)
        self.assertIn('agiru::MissingData is DATA', refused.stderr)
        self.assertFalse(self.output.exists())


class CompilerCacheGate(unittest.TestCase):
    def test_complete_app_build_does_not_require_the_diagnostic_slice(self):
        makefile = (SCRIPT.parents[2] / 'Makefile').read_text()
        recipe = re.search(r'^apps:.*?(?=^tree:)', makefile, re.M | re.S).group()
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            prefix = ('SHELL := /bin/bash\nSELF := ' + str(root) + '\n'
                      'B := ' + str(root / 'build') + '\nJOBS := 2\n'
                      '.PHONY: apps comments db all\ncomments db:\n\t@true\n'
                      'all:\n\t@echo diagnostic-slice-build; exit 97\n')
            path = root / 'Makefile'
            path.write_text(prefix + recipe)
            command = ['make', '--no-print-directory', '-n', '-f', str(path), 'apps']
            parent_build = root / 'parent-build'
            parent_log = root / 'parent-ut.log'
            with patch.dict(os.environ, {'MAKEFLAGS': f'-e -- B={parent_build}',
                                        'MFLAGS': '-e', 'MAKEOVERRIDES': f'B={parent_build}',
                                        'B': str(parent_build), 'UT_LOG': str(parent_log)}):
                environment = isolated_make_environment()
            result = subprocess.run(command, env=environment, capture_output=True,
                                    text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertNotIn('diagnostic-slice-build', result.stdout)
            self.assertIn('-DAGIRU_BUILD_APPS=ON', result.stdout)
            self.assertIn('cmake --build ' + str(root / 'build/apps'), result.stdout)
            path.write_text(prefix + re.sub(r'^apps:[^\n]*', 'apps: all', recipe, count=1))
            control = subprocess.run(command, env=environment, capture_output=True,
                                     text=True, timeout=10)
            self.assertEqual(control.returncode, 0, control.stdout + control.stderr)
            self.assertIn('diagnostic-slice-build', control.stdout)
            self.assertFalse(parent_build.exists())
            self.assertFalse(parent_log.exists())

    def test_missing_generated_slice_input_does_not_disable_handwritten_gates(self):
        cmake = (SCRIPT.parents[2] / 'CMakeLists.txt').read_text()
        properties = re.search(
            r'set_source_files_properties\("\$\{CMAKE_SOURCE_DIR\}/apps/\$\{slice_source\}" '
            r'PROPERTIES\s+[^)]+\)', cmake).group()
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'handwritten.cpp').write_text('int gate() { return 0; }\n')
            prefix = '''cmake_minimum_required(VERSION 3.28)
project(MissingGeneratedInput CXX)
set(slice_source missing.cpp)
set(slice_group root)
'''
            suffix = '''
add_library(slice OBJECT "${CMAKE_SOURCE_DIR}/apps/${slice_source}")
set_target_properties(slice PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE GROUP)
add_library(handwritten OBJECT handwritten.cpp)
'''
            (root / 'CMakeLists.txt').write_text(prefix + properties + suffix)
            build = root / 'build'
            configured = subprocess.run(['cmake', '-S', str(root), '-B', str(build),
                                         '-G', 'Ninja', '-DCMAKE_CXX_COMPILER=clang++-19'],
                                        capture_output=True, text=True)
            self.assertEqual(configured.returncode, 0, configured.stdout + configured.stderr)
            gate = subprocess.run(['cmake', '--build', str(build), '--target', 'handwritten'],
                                  capture_output=True, text=True)
            self.assertEqual(gate.returncode, 0, gate.stdout + gate.stderr)
            slice_run = subprocess.run(['cmake', '--build', str(build), '--target', 'slice'],
                                       capture_output=True, text=True)
            self.assertNotEqual(slice_run.returncode, 0)
            self.assertIn('missing.cpp', slice_run.stdout + slice_run.stderr)
            (root / 'CMakeLists.txt').write_text(
                prefix + properties.replace('GENERATED TRUE ', '') + suffix)
            control = subprocess.run(['cmake', '-S', str(root), '-B', str(root / 'control'),
                                      '-G', 'Ninja', '-DCMAKE_CXX_COMPILER=clang++-19'],
                                     capture_output=True, text=True)
            self.assertNotEqual(control.returncode, 0)
            self.assertIn('missing.cpp', control.stdout + control.stderr)

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
            script = Path(__file__).resolve().parents[2] / 'scripts/unity_groups.py'
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
        repo = Path(__file__).resolve().parents[2]
        build = repo / Path(os.environ.get('B', 'build'))
        commands = json.loads((build / 'compile_commands.json').read_text())
        pch = [row for row in commands
               if 'agiru_slice' in row['command'] and 'cmake_pch.hxx.cxx' in row['file']]
        self.assertEqual(len(pch), 1)
        self.assertIn('-Xclang -fno-pch-timestamp', pch[0]['command'])
        setting = subprocess.run(
            ['make', '-s', f'B={build}',
             '--eval=cache-env:; @printf %s "$$CCACHE_SLOPPINESS"', 'cache-env'],
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
target_compile_options(probe PRIVATE -stdlib=libc++)
target_link_options(probe PRIVATE -stdlib=libc++ --rtlib=compiler-rt --unwindlib=libunwind -fuse-ld=lld-19)
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
            shutil.copyfile(Path(__file__).resolve().parents[2] / 'Makefile', root / 'Makefile')
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
            shutil.copyfile(Path(__file__).resolve().parents[2] / 'Makefile', root / 'Makefile')
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
            shutil.copyfile(Path(__file__).resolve().parents[2] / 'Makefile', root / 'Makefile')
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


class ProductSourceGate(unittest.TestCase):
    def setUp(self):
        TranspilerAttributeCensusGate.setUp(self)
        self.policy = {'include': ['Microsoft.Fixture'], 'exclude': [],
                       'product_exclude': ['bc-licensing:source/Excluded.Codeunit.al']}
        (self.root / 'scope.json').write_text(json.dumps(self.policy))
        (self.root / 'source/Excluded.Codeunit.al').write_text(
            'codeunit 50142 "Excluded UT" { Subtype = Test; '
            '[Test][FutureAttribute] procedure Check() begin end; }')
        (self.root / 'source/Core.Codeunit.al').write_text(
            'namespace Microsoft.Fixture; codeunit 50143 "Core UT" { Subtype = Test; '
            '[Test] procedure Post() begin end; }')

    def run_transpiler(self):
        return subprocess.run([str(self.transpiler), str(self.root),
                               str(self.root / 'apps.json'), str(self.root / 'generated')],
                              capture_output=True, text=True, timeout=30)

    def test_actual_transpiler_excludes_exact_source_and_keeps_raw_ut_methods(self):
        result = self.run_transpiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn('product exclusion bc-licensing: source/Excluded.Codeunit.al', result.stdout)
        paths = {path.name for path in (self.root / 'generated').rglob('*.h')}
        self.assertIn('CoreUT.h', paths)
        self.assertNotIn('ExcludedUT.h', paths)
        raw = milestone.scan(self.root / 'source')
        self.assertEqual({entry['id'] for entry in raw}, {50142, 50143})
        report = scope_inventory.inventory(self.root, {
            'apps': [{'name': 'fixture', 'source': 'source'}]}, self.policy)
        self.assertEqual(report['summary']['raw_test_attributes'], 2)
        self.assertEqual(report['summary']['product_excluded_test_methods'], 1)
        self.assertEqual(report['summary']['product_required_test_methods'], 1)

    def test_rules_do_not_match_similar_filenames(self):
        (self.root / 'source/ExcludedExtra.Codeunit.al').write_text(
            'codeunit 50144 "Retained" { [FutureAttribute] procedure Check() begin end; }')
        result = self.run_transpiler()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('futureattribute', result.stdout.lower())
        self.assertTrue(list((self.root / 'generated').rglob('Retained.h')))

    def test_module_rules_use_a_directory_boundary(self):
        (self.root / 'scope.json').write_text(json.dumps(
            dict(self.policy, product_exclude=['microsoft-cloud:source/'])))
        result = self.run_transpiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertFalse(list((self.root / 'generated').rglob('*UT.h')))

    def test_test_area_rules_retain_libraries_and_raw_test_identities(self):
        folder = self.root / 'source/Graph'
        folder.mkdir()
        (folder / 'Excluded.Codeunit.al').write_text(
            'namespace Microsoft.Fixture; codeunit 50145 "Graph UT" { Subtype = Test; '
            '[Test] procedure Check() begin end; }')
        (folder / 'Unnamespaced.Codeunit.al').write_text(
            'codeunit 50148 "Other Graph UT" { Subtype = Test; [Test] procedure Check() begin end; }')
        (folder / 'Library.Codeunit.al').write_text('codeunit 50146 "Graph Library" {}')
        (folder / 'Data.Table.al').write_text('table 50147 "Graph Data" { fields { field(1; No; Integer) {} } '
                                             'keys { key(PK; No) {} } }')
        policy = dict(self.policy, area_exclude=['Graph'])
        (self.root / 'scope.json').write_text(json.dumps(policy))
        result = self.run_transpiler()
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        paths = {path.name for path in (self.root / 'generated').rglob('*.h')}
        self.assertNotIn('GraphUT.h', paths)
        self.assertNotIn('OtherGraphUT.h', paths)
        self.assertIn('GraphLibrary.h', paths)
        self.assertIn('GraphData.h', paths)
        raw = milestone.scan(self.root / 'source')
        kept, excluded = milestone.partition(raw, self.root, self.root / 'source', policy)
        self.assertEqual({entry['id'] for entry in raw}, {50142, 50143, 50145, 50148})
        self.assertEqual([entry['id'] for entry in kept], [50143])
        self.assertEqual([(entry['id'], entry['reason']) for entry in excluded],
                         [(50142, 'bc-licensing'), (50145, 'selection-area'), (50148, 'selection-area')])
        report = scope_inventory.inventory(self.root, {
            'apps': [{'name': 'fixture', 'source': 'source'}]}, policy)
        self.assertEqual(report['summary']['test_methods'], 4)
        self.assertEqual(report['summary']['selected_test_methods'], 1)
        self.assertEqual(report['summary']['product_excluded_test_methods'], 1)
        self.assertEqual(report['summary']['omitted_required_test_methods'], 2)
        self.assertTrue(next(item for item in report['objects'] if item['id'] == 50146)[
            'selection_selected'])

    def test_malformed_unknown_duplicate_and_unbounded_rules_refuse(self):
        for entries in (['no-reason'], ['unsupported:source/Excluded.Codeunit.al'],
                        ['bc-licensing:/outside'], ['bc-licensing:source/../outside'],
                        ['bc-licensing:source\\outside'], ['bc-licensing:'],
                        ['bc-licensing:source//Excluded.Codeunit.al'],
                        ['bc-licensing:source//'],
                        ['bc-licensing:source/Excluded.Codeunit.al'] * 2,
                        ['bc-licensing:source/Missing.Codeunit.al']):
            with self.subTest(entries=entries):
                (self.root / 'scope.json').write_text(json.dumps(
                    dict(self.policy, product_exclude=entries)))
                result = self.run_transpiler()
                self.assertNotEqual(result.returncode, 0)
                self.assertIn('scope.json:', result.stdout + result.stderr)


class SourceInventoryGate(unittest.TestCase):
    root = Path(__file__).resolve().parents[2]
    policy = {'include': ['Microsoft', 'System.Security', 'System.Email'],
              'exclude': ['Microsoft.Integration']}

    def test_lexical_inventory_retains_quoted_comment_markers_and_multiline_headers(self):
        namespace, objects = scope_inventory.declarations(
            '// codeunit 999 Lost {}\n/* table 998 Hidden {} */\n'
            'NaMeSpAcE Microsoft.Finance;\nCODEUNIT\n42\n"Invoice // /* ""UT"""\n'
            '{ Subtype = Test; var Caption: Label \'[Test] procedure Fake()\'; '
            '[Test] [HandlerFunctions(\'Answer\')] internal procedure "Post // Invoice"() '
            'begin Message(\'// [Test] /*\'); end; }')
        self.assertEqual(namespace, 'Microsoft.Finance')
        self.assertEqual(len(objects), 1)
        self.assertEqual((objects[0]['kind'], objects[0]['id'], objects[0]['name']),
                         ('codeunit', 42, 'Invoice // /* "UT"'))
        self.assertEqual(objects[0]['methods'], ['Post // Invoice'])
        self.assertEqual(objects[0]['test_attributes'], 1)
        self.assertTrue(objects[0]['test_subtype'])

    def test_extra_bom_characters_do_not_hide_declarations_or_tests(self):
        namespace, objects = scope_inventory.declarations(
            '\ufeff\ufeff// header\n\ufeffnamespace Microsoft.Finance;\n'
            'codeunit 1 "Visible UT" { Subtype = Test; [Test] procedure Post() begin end; }')
        self.assertEqual(namespace, 'Microsoft.Finance')
        self.assertEqual(len(objects), 1)
        self.assertEqual(objects[0]['methods'], ['Post'])
        self.assertEqual(scope_inventory.declarations('table 2 "Embedded\ufeffName" {}')[1][0]['name'],
                         'Embedded\ufeffName')

    def test_equal_conditional_brace_paths_merge_without_dropping_raw_methods(self):
        text = ('#region Fixtures\ncodeunit 1 X { Subtype = Test;\n'
                '#if FIRST\n[Test] procedure A() begin end;\n'
                '#else\n[Test] procedure B() begin end;\n#endif\n'
                'layout {\n#if OLD\ngroup(A) {\n#else\ngroup(B) {\n'
                '#endif\nfield(X; X) {} } } }\n#endregion\n')
        _, objects = scope_inventory.declarations(text)
        self.assertEqual(len(objects), 1)
        self.assertEqual(objects[0]['methods'], ['A', 'B'])
        self.assertEqual(objects[0]['test_attributes'], 2)
        indented = '\n'.join('    ' + line if line.startswith('#') else line
                             for line in text.splitlines())
        self.assertEqual(scope_inventory.declarations(indented), ('', objects))
        self.assertEqual(scope_inventory.declarations(
            'enum 1 Kind\n    #pragma warning restore AL0659\n{}')[1][0]['id'], 1)
        self.assertEqual(scope_inventory.declarations(
            '    # region X\ntable 1 X {}\n    # endregion;')[1][0]['id'], 1)

    def test_conditional_headers_are_retained_and_ambiguous_braces_are_unmeasured(self):
        _, variants = scope_inventory.declarations(
            '#if OLD\ntable 1 A\n#else\ntable 1 B\n#endif\n{}')
        self.assertEqual([item['name'] for item in variants], ['A', 'B'])
        self.assertTrue(all(item['shared_body'] for item in variants))
        for text in ('page 1 A {\n#if OLD\ngroup(X) {\n#else\ngroup(Y) {}\n'
                     '#endif\n} }',):
            with self.subTest(text=text), self.assertRaises(ValueError):
                scope_inventory.declarations(text)

    def test_every_named_object_kind_and_anonymous_dotnet_remain_counted(self):
        headers = ['dotnet {}' if kind == 'dotnet' else f'{kind} "Name" {{}}'
                   for kind in sorted(scope_inventory.KINDS)]
        _, objects = scope_inventory.declarations('\n'.join(headers))
        self.assertEqual({item['kind'] for item in objects}, scope_inventory.KINDS)
        self.assertEqual(len(objects), 20)
        self.assertTrue(all(item['id'] is None for item in objects))
        self.assertEqual(next(item for item in objects if item['kind'] == 'dotnet')['name'], '')
        self.assertEqual(scope_inventory.declarations('namespace System.Security;'),
                         ('System.Security', []))

    def test_correlated_conditional_braces_retain_every_source_method_once(self):
        text = ('report 1 R { dataset {\n#if not CLEAN\n'
                'dataitem(Outer; Integer) {\n#endif\ndataitem(Inner; Integer) {}\n'
                '#if not CLEAN\n}\n#endif\n} }\n'
                'codeunit 2 "Variant UT" { Subtype = Test;\n'
                '#if A and (not B or CLEAN)\n[Test] procedure Branch() begin end;\n'
                '#elif A or B\n[Test] procedure Branch() begin end;\n'
                '#else\n[Test] procedure Last() begin end;\n#endif\n'
                '[Test] procedure Common() begin end; }')
        with self.assertRaisesRegex(ValueError, 'variant-specific'):
            scope_inventory.raw_declarations(text)
        namespace, objects = scope_inventory.declarations(text)
        self.assertEqual(namespace, '')
        self.assertEqual([(item['kind'], item['id']) for item in objects],
                         [('report', 1), ('codeunit', 2)])
        self.assertEqual(objects[1]['methods'], ['Branch', 'Branch', 'Last', 'Common'])
        self.assertEqual(objects[1]['test_attributes'], 4)
        self.assertEqual(len(objects[0]['conditional_variants']), 8)
        self.assertEqual(len(objects[1]['conditional_variants']), 8)
        self.assertEqual({tuple(sorted(item.items()))
                          for item in objects[0]['conditional_variants']},
                         {tuple(zip(('A', 'B', 'CLEAN'), flags))
                          for flags in scope_inventory.product((False, True), repeat=3)})

    def test_conditional_fallback_honours_define_undef_and_nested_branches(self):
        text = ('#define LOCAL\nreport 1 R { dataset {\n#if LOCAL\n'
                'dataitem(Outer; Integer) {\n#endif\n'
                '#if FIRST\n#if SECOND\ndataitem(A; Integer) {}\n'
                '#elif not SECOND\ndataitem(B; Integer) {}\n#endif\n'
                '#else\ndataitem(C; Integer) {}\n#endif\n'
                '#if LOCAL\n}\n#endif\n} }\n#undef LOCAL\n'
                '#if not LOCAL\ncodeunit 2 "Kept UT" { Subtype = Test; '
                '[Test] procedure Keep() begin end; }\n#endif\n')
        _, objects = scope_inventory.declarations(text)
        self.assertEqual([item['name'] for item in objects], ['R', 'Kept UT'])
        self.assertEqual(objects[1]['methods'], ['Keep'])
        self.assertEqual(len(objects[1]['conditional_variants']), 8)

    def test_conditional_fallback_refuses_invalid_or_unmeasurable_populations(self):
        split = ('report 1 R { dataset {\n#if A\ndataitem(Outer; Integer) {\n'
                 '#endif\ndataitem(Inner; Integer) {}\n#if A\n}\n#endif\n} }\n')
        for extra in ('#if A and\n#endif\n', '#if A + B\n#endif\n',
                      '#else\n#endif\n', '#if A\n#else\n#elif B\n#endif\n',
                      '#if A\n', '#define LOCAL\n#if not LOCAL\n'
                      'codeunit 2 X { Subtype = Test; [Test] procedure Lost() begin end; }\n'
                      '#endif\n'):
            with self.subTest(extra=extra), self.assertRaises(ValueError):
                scope_inventory.declarations(split + extra)
        budget = '\n'.join(f'#if S{at}\n#endif' for at in range(8))
        with self.assertRaisesRegex(ValueError, 'work budget'):
            scope_inventory.declarations(split + budget)

    def test_refused_conditional_variants_never_erase_measured_source_identities(self):
        text = ('report 1 R { dataset {\n#if A\ndataitem(Outer; Integer) {\n'
                '#endif\ndataitem(Inner; Integer) {}\n#if A\n}\n#endif\n} }\n'
                '#if FUTURE\n}\n#endif\n'
                'codeunit 2 X { Subtype = Test; [Test] procedure Keep() begin end; }')
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'Fixture').mkdir()
            (root / 'Fixture/Source.al').write_text(text)
            report = scope_inventory.inventory(
                root, {'apps': [{'name': 'fixture', 'source': 'Fixture'}]}, self.policy)
            self.assertEqual(report['summary']['objects'], 2)
            self.assertEqual(report['summary']['test_methods'], 1)
            self.assertEqual(report['summary']['raw_test_attributes'], 1)
            self.assertEqual(report['summary']['unmeasured_files'], 0)
            self.assertEqual(report['objects'][1]['methods'], ['Keep'])
            self.assertEqual(len(report['errors']), 1)
            refusals = report['files'][0]['conditional_variant_refusals']
            self.assertEqual(len(refusals), 2)
            self.assertTrue(all(item['symbols']['FUTURE'] for item in refusals))
            self.assertEqual(report['errors'][0]['variants'], refusals)

    def test_conditional_duplicates_are_not_collapsed_into_a_runnable_population(self):
        text = ('#if OLD\ncodeunit 1 "Same UT" { Subtype = Test; '
                '[Test] procedure A() begin end; }\n#else\n'
                'codeunit 1 "Same UT" { Subtype = Test; '
                '[Test] procedure A() begin end; }\n#endif\n')
        _, objects = scope_inventory.declarations(text)
        self.assertEqual(len(objects), 2)
        self.assertEqual(sum(len(item['methods']) for item in objects), 2)

    def test_malformed_or_unknown_declarations_refuse_instead_of_disappearing(self):
        for text in ('codeunit 1 X {', 'alienobject 1 X {}', '/* never closed',
                     'codeunit 1 "never closed',
                     'codeunit 1 X { [Test]\n#if A\nprocedure A() begin end;\n'
                     '#else\nprocedure B() begin end;\n#endif\n}',
                     'codeunit 1 X { [Test] trigger OnRun() begin end; }',
                     'codeunit 1 X { [Test][Test] procedure Twice() begin end; }'):
            with self.subTest(text=text), self.assertRaises(ValueError):
                scope_inventory.declarations(text)

    def test_namespace_selection_is_case_insensitive_bounded_and_never_a_source_filter(self):
        decisions = {'Microsoft.Finance': True, 'microsoft.integration.Graph': False,
                     'MicrosoftOther': False, '': True, 'System.Security.AccessControl': True,
                     'System.Email': True}
        for name, selected in decisions.items():
            self.assertEqual(scope_inventory.namespace_selected(name, self.policy), selected)
        tie = {'include': ['Contoso', 'Contoso.Hidden.Public'],
               'exclude': ['CONTOSO', 'Contoso.Hidden']}
        self.assertFalse(scope_inventory.namespace_selected('Contoso', tie))
        self.assertTrue(scope_inventory.namespace_selected('Contoso.Hidden.Public', tie))

    def test_raw_inventory_retains_unconfigured_apps_excluded_namespaces_and_utf16(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'Configured').mkdir()
            (root / 'Unconfigured').mkdir()
            (root / 'Configured/app.json').write_text('{}')
            (root / 'Unconfigured/app.json').write_text('{}')
            (root / 'Configured/Core.al').write_text(
                'namespace Microsoft.Finance; table 1 Core {}')
            (root / 'Unconfigured/Cloud.AL').write_bytes(
                'namespace Microsoft.Integration; codeunit 2 Cloud {'
                'Subtype = Test; [Test] procedure Check() begin end; }'.encode('utf-16'))
            report = scope_inventory.inventory(
                root, {'apps': [{'name': 'base', 'source': 'Configured'}]}, self.policy)
            self.assertEqual(report['errors'], [])
            self.assertEqual(report['summary']['files'], 2)
            self.assertEqual(report['summary']['objects'], 2)
            self.assertEqual(report['summary']['test_methods'], 1)
            self.assertEqual(report['summary']['outside_configured_app_files'], 1)
            self.assertEqual(report['summary']['encodings']['utf-16'], 1)
            cloud = next(item for item in report['objects'] if item['name'] == 'Cloud')
            self.assertFalse(cloud['namespace_selected'])
            self.assertEqual(cloud['configured_apps'], [])
            self.assertEqual(cloud['app_root'], 'Unconfigured')
            self.assertEqual(cloud['methods'], ['Check'])

    def test_unmeasured_files_and_missing_roots_are_visible_and_zero_population_refuses(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            configuration = {'apps': [{'name': 'base', 'source': 'Missing'}]}
            empty = scope_inventory.inventory(root, configuration, self.policy)
            self.assertEqual(empty['summary']['files'], 0)
            self.assertEqual(len(empty['errors']), 2)
            (root / 'Broken.al').write_bytes(b'\xff\xff')
            broken = scope_inventory.inventory(root, configuration, self.policy)
            self.assertEqual(broken['summary']['files'], 1)
            self.assertEqual(broken['summary']['unmeasured_files'], 1)
            self.assertEqual(len(broken['errors']), 2)

    def test_make_inventory_has_no_build_or_database_dependency_and_preserves_exit(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            shutil.copyfile(self.root / 'Makefile', root / 'Makefile')
            runner = root / 'python3'
            runner.write_text('#!/bin/sh\nprintf "%s\\n" "$@"\nexit 23\n')
            runner.chmod(0o755)
            env = dict(os.environ, PATH=f'{root}:{os.environ["PATH"]}',
                       AGIRU_BC_SOURCE=str(root / 'raw-source'))
            result = subprocess.run(['make', '--no-print-directory', 'census',
                                     'B=build/inventory'], cwd=root, env=env,
                                    capture_output=True, text=True, timeout=10)
            self.assertEqual(result.stdout.splitlines(),
                             [str(root / 'scripts/scope_inventory.py'), str(root / 'raw-source'),
                              '--output', 'build/inventory/scope-inventory.json'])
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('Error 23', result.stderr)

    def test_canonical_default_scope_matches_transpiler_and_provenance(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'Base').mkdir()
            (root / 'Base/Core.al').write_text('namespace Microsoft.API; table 1 Core {}')
            (root / 'apps.json').write_text(json.dumps(
                {'apps': [{'name': 'base', 'source': 'Base'}]}))
            policy = json.loads((self.root / 'scope.json').read_text())
            for entry in policy.get('product_exclude', []):
                _, source = entry.split(':', 1)
                selected = root / source
                selected.parent.mkdir(parents=True, exist_ok=True)
                selected.write_text('codeunit 50100 Explicit {}')
            output = root / 'inventory.json'
            result = subprocess.run([sys.executable, scope_inventory.__file__, str(root),
                                     '--apps', str(root / 'apps.json'), '--output', str(output)],
                                    capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            report = json.loads(output.read_text())
            self.assertEqual(report['scope_sha256'],
                             hashlib.sha256((self.root / 'scope.json').read_bytes()).hexdigest())
            self.assertTrue(next(item for item in report['objects'] if item['name'] == 'Core')[
                'namespace_selected'])
            self.assertFalse((self.root / 'src/gen/scope.json').exists())

    def test_explicit_product_rules_are_bounded_and_never_replace_raw_counts(self):
        rules = scope_inventory.product_rules({'product_exclude': ['bc-licensing:Module/']})
        self.assertEqual(scope_inventory.product_reason('Module/Check.al', rules), 'bc-licensing')
        self.assertIsNone(scope_inventory.product_reason('ModuleExtra/Check.al', rules))
        self.assertIsNone(scope_inventory.product_reason('module/Check.al', rules))

    def test_source_domains_are_separate_without_shrinking_raw_populations(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'src').mkdir()
            (root / 'src/Commercial.al').write_text('table 1 Commercial {}')
            (root / 'src/User.al').write_text('table 2 User {}')
            configuration = {'apps': [{'name': 'native', 'source': 'src'}]}
            policy = {'include': ['System', 'Microsoft'], 'exclude': [],
                      'product_exclude': ['bc-licensing:system-symbols/src/Commercial.al']}
            native = scope_inventory.inventory(root, configuration, policy, 'system-symbols')
            regular = scope_inventory.inventory(root, configuration, policy)
            self.assertEqual(native['summary']['objects'], 2)
            self.assertEqual(regular['summary']['objects'], 2)
            self.assertEqual(native['summary']['product_excluded_objects'], 1)
            self.assertEqual(regular['summary']['product_excluded_objects'], 0)
            self.assertEqual(native['source_sha256'], regular['source_sha256'])
            self.assertFalse(native['errors'])
            self.assertFalse(regular['errors'])
            self.assertEqual(len(regular['other_domain_rules']), 1)
            with self.assertRaises(ValueError):
                scope_inventory.inventory(root, configuration, policy, 'unknown')
            with self.assertRaises(ValueError):
                scope_inventory.product_rules({'product_exclude': ['bc-licensing:system-symbols/']})
        for value in ('bc-licensing:/root', 'bc-licensing:../root', 'bc-licensing:Module/./Check.al',
                      'bc-licensing:Module//Check.al', 'bc-licensing:Module//',
                      'unknown:Module/', 'missing-reason',
                      'bc-licensing:Module\\Check.al'):
            with self.subTest(value=value), self.assertRaises(ValueError):
                scope_inventory.product_rules({'product_exclude': [value]})
        with self.assertRaises(ValueError):
            scope_inventory.product_rules({'product_exclude': ['bc-licensing:Module/'] * 2})

    def test_cli_retains_the_report_and_fails_on_inventory_errors(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'Base').mkdir()
            (root / 'Base/Broken.al').write_text('futureobject 1 Unknown {}')
            (root / 'apps.json').write_text(json.dumps(
                {'apps': [{'name': 'base', 'source': 'Base'}]}))
            (root / 'scope.json').write_text(json.dumps(self.policy))
            output = root / 'inventory.json'
            result = subprocess.run([sys.executable, scope_inventory.__file__, str(root),
                                     '--apps', str(root / 'apps.json'), '--scope', str(root / 'scope.json'),
                                     '--output', str(output)], capture_output=True, text=True, timeout=10)
            self.assertNotEqual(result.returncode, 0)
            report = json.loads(output.read_text())
            self.assertEqual(report['summary']['files'], 1)
            self.assertEqual(report['summary']['unmeasured_files'], 1)
            self.assertEqual(report['errors'][0]['source'], 'Base/Broken.al')
            self.assertEqual(report['apps_sha256'], hashlib.sha256(
                (root / 'apps.json').read_bytes()).hexdigest())


class ProvisioningScopeGate(unittest.TestCase):
    root = Path(__file__).resolve().parents[2]

    def test_provisioning_does_not_fabricate_a_commercial_license(self):
        source = (self.root / 'src/rt/Storage.cpp').read_text()
        for token in ('TenantLicenseState', 'kLicensedFromYear', 'kLicensedToYear'):
            self.assertFalse(token in source, f'provisioning still contains {token}')
        self.assertIn('InstalledTables()', source)
        self.assertIn('platform::Company', source)
        self.assertIn('InstalledProfiles()', source)


class NativeToolchainGate(unittest.TestCase):
    root = Path(__file__).resolve().parents[2]

    @staticmethod
    def surface_tool(script):
        specification = importlib.util.spec_from_file_location('surface_reference_probe', script)
        module = importlib.util.module_from_spec(specification)
        specification.loader.exec_module(module)
        return module

    def test_builtin_reference_root_is_explicit_and_checkout_relative(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            script = root / 'relocated/scripts/door.py'
            script.parent.mkdir(parents=True)
            shutil.copyfile(self.root / 'scripts/door.py', script)
            developer = root / 'developer'
            integer = developer / 'methods-auto/integer'
            integer.mkdir(parents=True)
            (integer / 'integer-data-type.md').write_text('# Integer Data Type\n')
            with patch.dict(os.environ, AGIRU_DEV_DOC_ROOT=str(developer)):
                surface = self.surface_tool(script)
                self.assertEqual(surface.ROOT, script.parent.parent)
                self.assertEqual(surface.DOC, developer / 'methods-auto')
                self.assertEqual(surface.canonical_names(), {'integer': 'Integer'})

    def test_builtin_reference_default_uses_the_local_developer_repository(self):
        with tempfile.TemporaryDirectory() as folder:
            home = Path(folder)
            developer = home / 'Git/dynamics365smb-devitpro-pb/dev-itpro/developer'
            integer = developer / 'methods-auto/integer'
            integer.mkdir(parents=True)
            (integer / 'integer-data-type.md').write_text('# Integer Data Type\n')
            with patch.dict(os.environ), patch('pathlib.Path.home', return_value=home):
                os.environ.pop('AGIRU_DEV_DOC_ROOT', None)
                surface = self.surface_tool(self.root / 'scripts/door.py')
                self.assertEqual(surface.DOC, developer / 'methods-auto')
                self.assertEqual(surface.canonical_names(), {'integer': 'Integer'})

    def test_missing_explicit_builtin_reference_never_falls_back(self):
        with tempfile.TemporaryDirectory() as folder:
            home = Path(folder)
            local = home / 'Git/dynamics365smb-devitpro-pb/dev-itpro/developer/methods-auto'
            local.mkdir(parents=True)
            missing = home / 'absent-developer'
            with patch.dict(os.environ, AGIRU_DEV_DOC_ROOT=str(missing)), \
                    patch('pathlib.Path.home', return_value=home):
                surface = self.surface_tool(self.root / 'scripts/door.py')
                with self.assertRaises(FileNotFoundError):
                    surface.canonical_names()

    def test_only_clang_builds_are_advertised(self):
        result = subprocess.run(['make', '--no-print-directory', 'help'], cwd=self.root,
                                capture_output=True, text=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertNotRegex(result.stdout, r'(?im)^gcc\s')
        for target in ('slice-check', 'include-cost', 'lint-one', 'verify-start', 'verify-status',
                       'native-consumers'):
            self.assertRegex(result.stdout, rf'(?m)^{target}\s', f'{target} is absent from make help')
        refused = subprocess.run(['make', '--no-print-directory', '-n', 'gcc'], cwd=self.root,
                                 capture_output=True, text=True, timeout=10)
        self.assertNotEqual(refused.returncode, 0, 'the retired compiler target still exists')
        self.assertIn("No rule to make target 'gcc'", refused.stderr)

    def test_verification_defaults_and_overrides_preserve_failures(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            shutil.copyfile(self.root / 'Makefile', root / 'Makefile')
            runner = root / 'python3'
            runner.write_text('#!/bin/sh\nprintf "%s\\n" "$@"\nexit 23\n')
            runner.chmod(0o755)
            env = dict(os.environ, PATH=f'{root}:{os.environ["PATH"]}')
            for name in ('MAKEFLAGS', 'MFLAGS', 'MAKEOVERRIDES', 'VERIFY_TARGETS'):
                env.pop(name, None)
            for target in ('verify', 'verify-start'):
                for override in (None, 'all test ut tree apps'):
                    with self.subTest(target=target, override=override):
                        command = ['make', '--no-print-directory', target, 'JOBS=2']
                        if override:
                            command.append(f'VERIFY_TARGETS={override}')
                        result = subprocess.run(command, cwd=root, env=env, capture_output=True,
                                                text=True, timeout=10)
                        arguments = result.stdout.splitlines()
                        prefix = [str(root / 'scripts/verify_snapshot.py'), 'start', '--reuse']
                        if target == 'verify-start':
                            prefix.append('--detach')
                        self.assertEqual(arguments, prefix + ['--jobs', '2'] +
                                         (override or 'all test').split())
                        self.assertNotEqual(result.returncode, 0, 'verification failure was lost')

    def test_cmake_selects_clang_and_rejects_other_compiler_ids(self):
        text = (self.root / 'CMakeLists.txt').read_text()
        policy = text[text.index('cmake_minimum_required'):text.index('set(CMAKE_CXX_STANDARD ')]
        self.assertEqual(policy.count('project(agiru CXX)'), 1)
        with tempfile.TemporaryDirectory() as folder:
            probe = Path(folder) / 'policy.cmake'
            for compiler_id in ('Clang', 'GNU'):
                with self.subTest(compiler_id=compiler_id):
                    probe.write_text(policy.replace('project(agiru CXX)',
                                                    f'set(CMAKE_CXX_COMPILER_ID {compiler_id})') +
                                     '\nget_filename_component(selected "${CMAKE_CXX_COMPILER}" NAME)\n'
                                     'if(NOT selected STREQUAL "clang++-19")\n'
                                     '  message(FATAL_ERROR "default compiler is not clang++-19")\n'
                                     'endif()\n')
                    result = subprocess.run(['cmake', '-P', str(probe)], capture_output=True,
                                            text=True, timeout=10)
                    if compiler_id == 'Clang':
                        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
                    else:
                        self.assertNotEqual(result.returncode, 0)
                        self.assertIn('agiru requires Clang', result.stderr)

    def test_installation_keeps_the_standard_library_not_a_second_compiler(self):
        installer = (self.root / 'scripts/install.sh').read_text()
        self.assertNotRegex(installer, r'\b(?:gcc|g\+\+)(?:-\d+)?\b')
        for package in ('clang-19', 'clang-format-19', 'clang-tidy-19', 'lld-19',
                        'libc++-19-dev', 'libc++abi-19-dev', 'libunwind-19-dev',
                        'libclang-rt-19-dev'):
            self.assertIn(package, installer.split())
        self.assertNotIn('libstdc++', installer)

    def test_native_configuration_and_syntax_probes_select_llvm_libraries(self):
        cmake = (self.root / 'CMakeLists.txt').read_text()
        self.assertIn('add_compile_options(-stdlib=libc++)', cmake)
        for flag in ('--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=${LLD}'):
            self.assertIn(flag, cmake)
        self.assertIn('AGIRU_LLVM_CXX23_WORKS', cmake)
        self.assertIn('LLVM C++23 libraries are unavailable', cmake)
        for name in ('first_gap.sh', 'tree_syntax.sh', 'include_cost.sh'):
            with self.subTest(script=name):
                script = (self.root / 'scripts' / name).read_text()
                self.assertIn('CXX=${CXX:-clang++-19}', script)
                self.assertIn('-stdlib=libc++', script)
                self.assertNotRegex(script, r'(?m)^\s*(?:if\s+)?clang\+\+\s')


class SnapshotGate(unittest.TestCase):
    def test_selected_build_configuration_is_pinned_and_explicit_roots_win(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'CMakeLists.txt').touch()
            selected = root / 'selected'
            selected.mkdir()
            old = root / 'old source'
            old.mkdir()
            new = root / 'new source'
            new.mkdir()
            (selected / 'CMakeCache.txt').write_text(
                'AGIRU_TEST_DSN:STRING=postgresql://fixture:fixture@127.0.0.1:5432/gate\n'
                'AGIRU_MASTER_DSN:STRING=postgresql://fixture:fixture@127.0.0.1:5432/master\n'
                f'AGIRU_AL_SOURCE:STRING={old}\nAGIRU_BC_SOURCE:STRING={old}\n'
                'AGIRU_BUILD_APPS:BOOL=OFF\nAGIRU_BUILD_SLICE:BOOL=ON\nCMAKE_BUILD_TYPE:STRING=\n')
            with patch.dict(os.environ, {'B': str(selected), 'AGIRU_AL_SOURCE': str(new),
                                         'AGIRU_BC_SOURCE': str(new)}, clear=True):
                settings = verify.build_configuration(root)
            self.assertEqual(settings['AGIRU_TEST_DSN'],
                             'postgresql://fixture:fixture@127.0.0.1:5432/gate')
            self.assertEqual(settings['AGIRU_AL_SOURCE'], str(new))
            self.assertEqual(settings['AGIRU_BC_SOURCE'], str(new))
            self.assertEqual(settings['AGIRU_BUILD_SLICE'], 'ON')
            self.assertEqual(settings['AGIRU_BUILD_APPS'], 'OFF')
            signature = verify.freeze_configuration(settings, root)
            private = root / 'cmake-configuration.json'
            self.assertEqual(stat.S_IMODE(private.stat().st_mode), 0o600)
            self.assertEqual(signature, hashlib.sha256(private.read_bytes()).hexdigest())
            with patch.dict(os.environ, {'B': str(selected), 'AGIRU_AL_SOURCE': str(root / 'missing')}, clear=True):
                with self.assertRaisesRegex(RuntimeError, 'AGIRU_AL_SOURCE'):
                    verify.build_configuration(root)

    def test_make_configuration_roundtrips_quoted_values_without_shell_or_make_expansion(self):
        repository = Path(__file__).resolve().parents[2]
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            fake = root / 'bin'
            fake.mkdir()
            executable = fake / 'cmake'
            executable.write_text('#!/usr/bin/env python3\nimport json, os, sys\n'
                                  'from pathlib import Path\n'
                                  'Path(os.environ["CONFIGURE_RECEIPT"]).write_text(json.dumps(sys.argv[1:]))\n')
            executable.chmod(0o755)
            receipt = root / 'args.json'
            values = {'AGIRU_TEST_DSN': "host=127.0.0.1 password='fixture-$quoted;$(false)'",
                      'AGIRU_AL_SOURCE': str(root / "quoted '$source directory")}
            environment = isolated_make_environment()
            environment.update(PATH=str(fake) + os.pathsep + environment['PATH'],
                               CONFIGURE_RECEIPT=str(receipt))
            result = subprocess.run(['make', '-f', str(repository / 'Makefile'), 'configure',
                                     f'SELF={root}', f'B={root / "build directory"}',
                                     f'CMAKE_ARGS={verify.cmake_arguments(values)}'],
                                    env=environment, capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            arguments = json.loads(receipt.read_text())
            self.assertEqual(arguments[:6], ['-S', str(root), '-B', str(root / 'build directory'), '-G', 'Ninja'])
            self.assertTrue(all(f'-D{name}={value}' in arguments for name, value in values.items()))

    def test_frozen_configuration_is_applied_before_targets_and_external_B_cannot_leak(self):
        with tempfile.TemporaryDirectory() as folder:
            run = Path(folder)
            source = run / 'source'
            source.mkdir()
            (source / 'Makefile').write_text('B := $(CURDIR)/build\nconfigure:\n'
                                           '\t@mkdir -p "$(B)"\n'
                                           '\t@printf "%s\\n" configured > "$(B)/configured"\n'
                                           'probe:\n\t@test -f "$(B)/configured"\n')
            signature = verify.freeze_configuration({name: 'fixture' for name in verify.CONFIGURATION_KEYS[:4]}, run)
            original = {'status': 'queued', 'targets': ['probe'], 'jobs': 1,
                        'source_sha256': verify.digest(source), 'cmake_configuration_sha256': signature}
            (run / 'result.json').write_text(json.dumps(original))
            with patch.dict(os.environ, B=str(run / 'external'), CMAKE_ARGS='unfrozen overrides'):
                self.assertEqual(verify.run_snapshot(run), 0)
            result = json.loads((run / 'result.json').read_text())
            self.assertEqual(result['target_exits'], {'configure': 0, 'probe': 0})
            self.assertEqual(result['post_cmake_configuration_sha256'], signature)
            self.assertTrue((source / 'build/configured').is_file())
            self.assertFalse((run / 'external').exists())
            (source / 'build/configured').unlink()
            (run / 'cmake-configuration.json').write_text('{}')
            (run / 'result.json').write_text(json.dumps(original))
            self.assertEqual(verify.run_snapshot(run), 1)
            result = json.loads((run / 'result.json').read_text())
            self.assertEqual(result['target_exits']['configure'], 2)
            self.assertEqual(result['target_exits']['probe'], 2)
            self.assertFalse((source / 'build/configured').exists())

    def test_failed_configuration_refuses_targets_instead_of_using_stale_outputs(self):
        with tempfile.TemporaryDirectory() as folder:
            run = Path(folder)
            source = run / 'source'
            source.mkdir()
            (source / 'Makefile').write_text('configure:\n\t@exit 7\nprobe:\n\t@touch ran\n')
            signature = verify.freeze_configuration({name: 'fixture' for name in verify.CONFIGURATION_KEYS[:4]}, run)
            (run / 'result.json').write_text(json.dumps({'status': 'queued', 'targets': ['probe'], 'jobs': 1,
                'source_sha256': verify.digest(source), 'cmake_configuration_sha256': signature}))
            self.assertEqual(verify.run_snapshot(run), 1)
            result = json.loads((run / 'result.json').read_text())
            self.assertNotEqual(result['target_exits']['configure'], 0)
            self.assertEqual(result['target_exits']['probe'], 2)
            self.assertFalse((source / 'ran').exists())

    def test_missing_runner_is_failed_not_a_permanent_running_lane(self):
        with tempfile.TemporaryDirectory() as folder:
            receipt = Path(folder) / 'result.json'
            receipt.write_text(json.dumps({'status': 'running', 'pid': 123,
                                           'targets': ['tc', 'native-enums']}))
            with patch.object(verify.os, 'kill', side_effect=ProcessLookupError):
                self.assertEqual(verify.receipt_state(receipt), 'failed')
            result = json.loads(receipt.read_text())
            self.assertEqual(result['exit_code'], 2)
            self.assertEqual(result['target_exits'], {'interrupted': 2})
            self.assertEqual(result['targets'], ['tc', 'native-enums'])

    def test_live_or_uninspectable_runner_remains_running(self):
        with tempfile.TemporaryDirectory() as folder:
            receipt = Path(folder) / 'result.json'
            receipt.write_text(json.dumps({'status': 'running', 'pid': 123}))
            for outcome in (None, PermissionError):
                with patch.object(verify.os, 'kill', side_effect=outcome):
                    self.assertEqual(verify.receipt_state(receipt), 'running')
            self.assertEqual(json.loads(receipt.read_text())['status'], 'running')

    def test_verification_root_is_external_and_repository_specific(self):
        root = Path(__file__).resolve().parents[2]
        current = verify.verification_root(root)
        self.assertTrue(current.is_relative_to('/tmp'))
        self.assertFalse(current.is_relative_to(root))
        self.assertEqual(current, verify.verification_root(root))
        self.assertNotEqual(current, verify.verification_root(root / 'other'))

    def test_deleted_tracked_caches_do_not_abort_freezing(self):
        for reuse in (False, True):
            for cache_removed in (False, True):
                with self.subTest(reuse=reuse, cache_removed=cache_removed), \
                        tempfile.TemporaryDirectory() as folder:
                    root = Path(folder)
                    cache = root / 'test/__pycache__/tracked.pyc'
                    cache.parent.mkdir(parents=True)
                    cache.write_bytes(b'cache')
                    retired = root / 'retired.cpp'
                    retired.write_text('retired source\n')
                    (root / 'Makefile').write_text('probe:\n\t@mkdir -p build\n'
                                                 '\t@touch build/executed\n')
                    for command in (['init', '-q'], ['add', '.'],
                                    ['-c', 'user.name=Gate', '-c', 'user.email=gate@example.invalid',
                                     'commit', '-qm', 'fixture']):
                        subprocess.run(['git', '-C', str(root), *command], check=True,
                                       capture_output=True)
                    retired.unlink()
                    if cache_removed:
                        cache.unlink()
                    expected = verify.digest(root)
                    arguments = SimpleNamespace(targets=['probe'], jobs=1, detach=False, reuse=reuse)
                    with patch.object(verify, 'ROOT', root), \
                            patch.dict(os.environ, {'AGIRU_SYSTEM_SYMBOLS': ''}):
                        self.assertEqual(verify.start(arguments), 0)
                    run = Path((verify.verification_root(root) / 'latest').read_text().strip())
                    result = json.loads((run / 'result.json').read_text())
                    build_source = Path(result.get('build_source', run / 'source'))
                    self.assertTrue((build_source / 'build/executed').is_file())
                    self.assertFalse((run / 'source/test/__pycache__').exists())
                    self.assertFalse((build_source / 'retired.cpp').exists())
                    self.assertEqual(result['source_sha256'], expected)
                    self.assertEqual(result['post_source_sha256'], expected)
                    self.assertEqual(result['target_exits'], {'probe': 0})

    def test_explicit_system_symbols_are_frozen_and_reach_the_runner(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            package = root / 'work/symbols'
            (package / 'src').mkdir(parents=True)
            (package / 'NavxManifest.xml').write_text('<Package/>')
            (package / 'SymbolReference.json').write_text('{}')
            (package / 'src/Fixture.al').write_text('fixture')
            (root / 'Makefile').write_text('probe:\n'
                                         '\t@mkdir -p build\n'
                                         '\t@printf "%s" "$$AGIRU_SYSTEM_SYMBOLS" > build/input\n')
            for arguments in (['init', '-q'], ['add', 'Makefile'],
                              ['-c', 'user.name=Gate', '-c', 'user.email=gate@example.invalid',
                               'commit', '-qm', 'fixture']):
                subprocess.run(['git', '-C', str(root), *arguments], check=True,
                               capture_output=True)
            arguments = SimpleNamespace(targets=['probe'], jobs=1, detach=False, reuse=False)
            with patch.object(verify, 'ROOT', root), \
                    patch.dict(os.environ, {
                        'AGIRU_SYSTEM_SYMBOLS': str(package),
                        'MAKEFLAGS': '-e -- AGIRU_SYSTEM_SYMBOLS=/unfrozen/make-override',
                        'MFLAGS': '-e',
                        'MAKEOVERRIDES': 'AGIRU_SYSTEM_SYMBOLS=/unfrozen/make-override'}):
                self.assertEqual(verify.start(arguments), 0)
            run = Path((verify.verification_root(root) / 'latest').read_text().strip())
            result = json.loads((run / 'result.json').read_text())
            self.assertEqual((run / 'source/build/input').read_text(), str(run / 'system_symbols'))
            self.assertEqual(result['system_symbols_sha256'], verify.digest(package))
            self.assertEqual(result['post_system_symbols_sha256'], verify.digest(package))
            (package / 'src/Fixture.al').write_text('later edit')
            self.assertEqual((run / 'system_symbols/src/Fixture.al').read_text(), 'fixture')

    def test_changed_frozen_system_symbols_refuse_before_execution(self):
        with tempfile.TemporaryDirectory() as folder:
            run = Path(folder)
            source = run / 'source'
            source.mkdir()
            (source / 'Makefile').write_text('probe:\n\t@touch ran\n')
            symbols = run / 'system_symbols'
            symbols.mkdir()
            (symbols / 'Fixture.al').write_text('original')
            (run / 'result.json').write_text(json.dumps({
                'status': 'queued', 'targets': ['probe'], 'jobs': 1,
                'source_sha256': verify.digest(source),
                'system_symbols_sha256': verify.digest(symbols)}))
            (symbols / 'Fixture.al').write_text('changed')
            self.assertEqual(verify.run_snapshot(run), 1)
            self.assertFalse((source / 'ran').exists())
            result = json.loads((run / 'result.json').read_text())
            self.assertEqual(result['target_exits']['probe'], 2)
            self.assertEqual(result['target_exits']['system_symbols_immutable'], 1)

    def test_dependency_freeze_refuses_changed_content_and_symlinks(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            source = root / 'input'
            source.mkdir()
            (source / 'Fixture.al').write_text('original')
            copy = verify.copy_source

            def change(original, destination):
                copy(original, destination)
                (original / 'Fixture.al').write_text('changed')

            with patch.object(verify, 'copy_source', side_effect=change):
                with self.assertRaisesRegex(RuntimeError, 'changed during snapshot'):
                    verify.freeze_input(source, root / 'copy', 'System symbols')
            (source / 'alias.al').symlink_to(root / 'external.al')
            with self.assertRaisesRegex(RuntimeError, 'unfrozen symlink'):
                verify.freeze_input(source, root / 'linked', 'System symbols')

    def test_symbols_changed_by_a_target_cannot_publish_success(self):
        with tempfile.TemporaryDirectory() as folder:
            run = Path(folder)
            source = run / 'source'
            source.mkdir()
            symbols = run / 'system_symbols'
            symbols.mkdir()
            fixture = symbols / 'Fixture.al'
            fixture.write_text('original')
            (source / 'Makefile').write_text('probe:\n'
                                           f'\t@printf changed > "{fixture}"\n')
            expected = verify.digest(symbols)
            (run / 'result.json').write_text(json.dumps({
                'status': 'queued', 'targets': ['probe'], 'jobs': 1,
                'source_sha256': verify.digest(source),
                'system_symbols_sha256': expected}))
            self.assertEqual(verify.run_snapshot(run), 1)
            result = json.loads((run / 'result.json').read_text())
            self.assertEqual(result['target_exits']['probe'], 0)
            self.assertEqual(result['target_exits']['system_symbols_immutable'], 1)
            self.assertNotEqual(result['post_system_symbols_sha256'], expected)

    def test_frozen_symbols_cannot_gain_unhashed_directory_links(self):
        with tempfile.TemporaryDirectory() as folder:
            run = Path(folder)
            source = run / 'source'
            source.mkdir()
            (source / 'Makefile').write_text('probe:\n\t@touch ran\n')
            symbols = run / 'system_symbols'
            symbols.mkdir()
            expected = verify.digest(symbols)
            external = run / 'external'
            external.mkdir()
            (external / 'Fixture.al').write_text('mutable')
            (symbols / 'alias').symlink_to(external, target_is_directory=True)
            self.assertEqual(verify.digest(symbols), expected,
                             'directory links are not ordinary digest file entries')
            (run / 'result.json').write_text(json.dumps({
                'status': 'queued', 'targets': ['probe'], 'jobs': 1,
                'source_sha256': verify.digest(source),
                'system_symbols_sha256': expected}))
            self.assertEqual(verify.run_snapshot(run), 1)
            self.assertFalse((source / 'ran').exists())

    def test_undeclared_symbols_environment_cannot_leak_into_a_snapshot(self):
        with tempfile.TemporaryDirectory() as folder:
            run = Path(folder)
            source = run / 'source'
            source.mkdir()
            (source / 'Makefile').write_text('probe:\n\t@test -z "$$AGIRU_SYSTEM_SYMBOLS"\n')
            (run / 'result.json').write_text(json.dumps({
                'status': 'queued', 'targets': ['probe'], 'jobs': 1,
                'source_sha256': verify.digest(source)}))
            with patch.dict(os.environ, {
                    'AGIRU_SYSTEM_SYMBOLS': '/unfrozen/live/input',
                    'MAKEFLAGS': '-e -- AGIRU_SYSTEM_SYMBOLS=/unfrozen/make-override',
                    'MFLAGS': '-e',
                    'MAKEOVERRIDES': 'AGIRU_SYSTEM_SYMBOLS=/unfrozen/make-override'}):
                self.assertEqual(verify.run_snapshot(run), 0)

    def test_raw_census_receipts_survive_reusing_the_same_build_lane(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            lane = root / 'lane'
            lane.mkdir()
            (lane / 'Makefile').write_text(
                'B := $(CURDIR)/build\ncensus:\n\t@mkdir -p "$(B)"\n'
                '\t@printf "%s\\n" "$$CENSUS_MARKER" > "$(B)/scope-inventory.json"\n')
            receipts = []
            for marker in ('first', 'second'):
                run = root / marker
                run.mkdir()
                (run / 'result.json').write_text(json.dumps({
                    'status': 'queued', 'targets': ['census'], 'jobs': 1,
                    'build_source': str(lane), 'source_sha256': verify.digest(lane)}))
                with patch.dict(os.environ, CENSUS_MARKER=marker):
                    self.assertEqual(verify.run_snapshot(run), 0)
                receipts.append(run / 'artifacts/census/scope-inventory.json')
            self.assertEqual([path.read_text().strip() for path in receipts], ['first', 'second'])
            self.assertFalse((lane / 'build/scope-inventory.json').exists())

    def test_bc_test_snapshot_requires_original_notice_before_dependency_copy(self):
        with tempfile.TemporaryDirectory() as folder:
            base = Path(folder)
            root = base / 'agiru'
            root.mkdir()
            (root / 'Makefile').write_text('test ut:\n'
                                           '\t@test -f "$$AGIRU_LAYOUT_SOURCE_NOTICE"\n'
                                           '\t@mkdir -p build\n'
                                           '\t@touch build/ran\n')
            bc = base / 'relocated-source'
            bc.mkdir()
            (bc / 'Fixture.al').write_text('fixture\n')
            subprocess.run(['git', 'init', '-q', str(root)], check=True)
            subprocess.run(['git', '-C', str(root), 'add', '.'], check=True)
            subprocess.run(['git', '-C', str(root), '-c', 'user.name=Fixture',
                            '-c', 'user.email=fixture@example.invalid', 'commit', '-qm',
                            'fixture'], check=True)
            arguments = SimpleNamespace(targets=['test', 'ut'], jobs=1, detach=False, reuse=False)
            with patch.object(verify, 'ROOT', root), \
                    patch.dict(os.environ, {'AGIRU_BC_SOURCE': str(bc)}):
                os.environ.pop('AGIRU_LAYOUT_SOURCE_NOTICE', None)
                with patch.object(verify, 'freeze_input', wraps=verify.freeze_input) as freeze:
                    with self.assertRaisesRegex(RuntimeError, 'original BC source notice'):
                        verify.start(arguments)
                    freeze.assert_not_called()
                self.assertFalse((root / 'build/ran').exists())
                self.assertFalse((verify.verification_root(root) / 'latest').exists())
                original = base / 'original-notice'
                original.write_text('Original third-party notice\n')
                os.environ['AGIRU_LAYOUT_SOURCE_NOTICE'] = str(original)
                self.assertEqual(verify.start(arguments), 0)
            archive = Path((verify.verification_root(root) / 'latest').read_text().strip())
            result = json.loads((archive / 'result.json').read_text())
            self.assertEqual(result['target_exits'], {'test': 0, 'ut': 0})
            self.assertTrue((archive / 'source/build/ran').is_file())
            self.assertEqual(result['layout_source_notice_sha256'], verify.input_digest(original))
            self.assertEqual((archive / 'layout_source_notice').read_text(), original.read_text())

    def test_frozen_bc_revision_reaches_the_ut_runner(self):
        with tempfile.TemporaryDirectory() as folder:
            base = Path(folder)
            root = base / 'project'
            root.mkdir()
            (root / 'Makefile').write_text('ut:\n'
                                           '\t@mkdir -p "$(dir $(UT_LOG))"\n'
                                           '\t@printf "%s" "$$AGIRU_BC_REVISION" > "$(UT_LOG)"\n'
                                           '\t@mkdir -p build\n'
                                           '\t@cp "$$AGIRU_LAYOUT_SOURCE_NOTICE" build/frozen-notice\n')
            bc = base / 'BCApps'
            (bc / 'src').mkdir(parents=True)
            (bc / 'src/Fixture.al').write_text('fixture\n')
            (bc / 'LICENSE').write_text('Original third-party notice\n')
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
                os.environ.pop('AGIRU_LAYOUT_SOURCE_NOTICE', None)
                run = verify.start(arguments)
            self.assertEqual(run, 0)
            archive = Path((verify.verification_root(root) / 'latest').read_text().strip())
            result = json.loads((archive / 'result.json').read_text())
            self.assertEqual(result['bc_source_revision'], revision)
            self.assertEqual((archive / 'artifacts/ut.log').read_text(), revision)
            self.assertEqual((archive / 'source/build/frozen-notice').read_text(),
                             'Original third-party notice\n')
            self.assertEqual(result['layout_source_notice_sha256'],
                             verify.input_digest(bc / 'LICENSE'))
            (bc / 'LICENSE').write_text('Later upstream edit\n')
            self.assertEqual((archive / 'layout_source_notice').read_text(),
                             'Original third-party notice\n')
            (archive / 'layout_source_notice').write_text('Mutated frozen input\n')
            self.assertEqual(verify.run_snapshot(archive), 1)
            result = json.loads((archive / 'result.json').read_text())
            self.assertEqual(result['target_exits']['layout_source_notice_immutable'], 1)
            self.assertEqual(result['target_exits']['ut'], 2)

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
            run = Path((verify.verification_root(root) / 'latest').read_text().strip())
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

    def test_compiled_fixture_receipt_adds_its_real_consumer(self):
        fixture = self.root / 'test/fixture/Runner.cpp'
        fixture.parent.mkdir(parents=True)
        fixture.write_text('int main() { return 0; }\n')
        receipts = self.root / 'build/fixture-commands'
        receipts.mkdir(parents=True)
        entry = {'directory': str(self.root), 'file': str(fixture),
                 'arguments': ['clang++', '-c', str(fixture), '-o', 'runner.o']}
        (receipts / 'fixture.json').write_text(json.dumps([entry]))
        selected, total = analysis.select_units(self.root, True)
        self.assertEqual(selected, sorted([self.root / 'src/a.cpp', fixture]))
        self.assertEqual(total, 2)
        fixture.unlink()
        with self.assertRaisesRegex(RuntimeError, 'missing source'):
            analysis.select_units(self.root, True)

    def test_untracked_fixture_without_a_receipt_refuses(self):
        fixture = self.root / 'test/fixture/Runner.cpp'
        fixture.parent.mkdir(parents=True)
        fixture.write_text('int main() { return 0; }\n')
        with self.assertRaisesRegex(RuntimeError, 'no compile command'):
            analysis.select_units(self.root, False)
        with self.assertRaisesRegex(RuntimeError, 'no compile command'):
            analysis.select_units(self.root, True)

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

    def test_relocated_database_uses_its_own_relative_dependency_graph(self):
        build = self.root / 'build/podman'
        build.mkdir(parents=True)
        database = self.root / 'compile_commands.json'
        database.rename(build / database.name)
        database.symlink_to(build / database.name)
        entries = [{'directory': str(build), 'file': str(self.root / 'src/a.cpp'),
                    'arguments': ['clang++', '-c', str(self.root / 'src/a.cpp'), '-o', 'a.o']}]
        (build / database.name).write_text(json.dumps(entries))
        header = self.root / 'src/indirect.h'
        header.write_text('#pragma once\n')
        original = subprocess.check_output

        def output(command, **kwargs):
            if command[0] == 'ninja':
                self.assertEqual(command, ['ninja', '-C', str(build), '-t', 'deps'])
                return 'a.o: #deps 2, deps mtime 1 (VALID)\n    ../../src/indirect.h\n'
            return original(command, **kwargs)

        with patch.object(subprocess, 'check_output', side_effect=output):
            self.assertEqual(analysis.select_units(self.root, False)[0], [self.root / 'src/a.cpp'])

        with patch.object(subprocess, 'check_output', side_effect=lambda command, **kwargs:
                          '' if command[0] == 'ninja' else original(command, **kwargs)):
            with self.assertRaisesRegex(RuntimeError, 'header has no compiled consumer'):
                analysis.select_units(self.root, False)

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

    def test_targeted_analysis_does_not_require_an_unrelated_fixture(self):
        fixture = self.root / 'test/fixture/Runner.cpp'
        fixture.parent.mkdir(parents=True)
        fixture.write_text('int main() { return 0; }\n')
        selected, total = analysis.select_units(self.root, True, Path('src/a.cpp'))
        self.assertEqual(selected, [self.root / 'src/a.cpp'])
        self.assertEqual(total, 1)
        with self.assertRaisesRegex(RuntimeError, 'hand-written source has no compile command'):
            analysis.select_units(self.root, True)

    def test_targeted_analysis_cannot_select_an_unconfigured_or_deleted_source(self):
        missing = self.root / 'src/missing.cpp'
        missing.write_text('int missing;\n')
        with self.assertRaisesRegex(RuntimeError, 'not a compiled handwritten source'):
            analysis.select_units(self.root, True, missing)
        (self.root / 'src/a.cpp').unlink()
        with self.assertRaisesRegex(RuntimeError, 'not a compiled handwritten source'):
            analysis.select_units(self.root, True, Path('src/a.cpp'))

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
            shutil.copyfile(SCRIPT.parents[2] / 'test/run.sh', root / 'test/run.sh')
            (root / 'test/gate/Fixture.cpp').touch()
            scripts = ('transpiler/builtins-reproduce.sh', 'tooling/one-definition.sh',
                       'tooling/function-size.sh',
                       'runtime/required-isolation.sh', 'tooling/header-dependencies.sh',
                       'tooling/slice-check.sh', 'transpiler/interface-defaults.sh',
                       'reporting/report-layouts.sh', 'reporting/layout-assets.sh',
                       'runtime/number-sequences.sh', 'transpiler/table-keys.sh',
                       'runtime/reflection-metadata.sh', 'runtime/catalogue.sh', 'transpiler/native-enums.sh',
                       'runtime/test-contexts.sh', 'runtime/text-positions.sh', 'runtime/xml-reader.sh',
                       'runtime/codeunit-record.sh', 'runtime/page-navigation.sh',
                       'runtime/boolean-expressions.sh', 'runtime/for-loops.sh', 'runtime/variant-text.sh', 'transpiler/control-extensions.sh',
                       'transpiler/native-table-ids.sh', 'transpiler/native-codeunits.sh',
                       'runtime/base64.sh', 'runtime/encoding.sh', 'runtime/hashing.sh', 'runtime/conversion.sh', 'runtime/record-order.sh',
                       'runtime/streams.sh', 'transpiler/system-profile.sh', 'runtime/xmlport-import.sh',
                       'ui/page-profile.sh', 'runtime/session-identity.sh', 'runtime/ui-host.sh',
                       'runtime/record-refresh.sh', 'runtime/table-permissions.sh',
                       'runtime/permission-sets.sh', 'runtime/native-permissions.sh')
            for name in scripts:
                script = root / 'test' / name
                script.parent.mkdir(parents=True, exist_ok=True)
                script.write_text('exit 0\n')
            (root / 'test/tooling/toolchain.py').write_text('raise SystemExit(0)\n')
            command = ['sh', str(root / 'test/run.sh')]
            env = dict(os.environ, B=str(root / 'build'))
            case_count = len(scripts) + 2
            result = subprocess.run(command, env=env, capture_output=True, text=True)
            self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
            self.assertIn(f'{case_count} case(s), 1 red', result.stdout)
            binary = root / 'build/gate_Fixture'
            binary.write_text('#!/bin/sh\nexit 0\n')
            binary.chmod(0o755)
            result = subprocess.run(command, env=env, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn(f'{case_count} case(s), 0 red', result.stdout)
            for name in scripts:
                script = root / 'test' / name
                script.rename(script.with_suffix('.saved'))
                result = subprocess.run(command, env=env, capture_output=True, text=True)
                self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
                self.assertIn(f'{case_count} case(s), 1 red', result.stdout)
                script.with_suffix('.saved').rename(script)


class ReproductionGate(unittest.TestCase):
    def test_stale_generated_file_is_red_without_mutating_the_source(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for directory in ('test/transpiler', 'scripts', 'include', 'src/rt/written'):
                (root / directory).mkdir(parents=True)
            shutil.copyfile(SCRIPT.parents[2] / 'test/transpiler/builtins-reproduce.sh', root / 'test/transpiler/builtins-reproduce.sh')
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
            command = ['sh', str(root / 'test/transpiler/builtins-reproduce.sh')]
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
            result = subprocess.run(['sh', str(SCRIPT.parents[2] / 'test/tooling/one-definition.sh')],
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
        repo = SCRIPT.parents[2]
        for name in ('ut-milestone.sh', 'ut_milestone.py', 'ut_manifest.py', 'ut_results.py',
                     'source_revision.py', 'scope_inventory.py'):
            shutil.copyfile(repo / 'scripts' / name, self.root / 'scripts' / name)
        (self.root / 'scope.json').write_text(json.dumps({'include': ['Microsoft'], 'exclude': []}))
        self.runner = self.root / 'build/agiru'
        self.psql = self.root / 'build/psql'
        self.psql.write_text('#!/bin/sh\nexit 0\n')
        self.psql.chmod(0o755)
        self.ninja = self.root / 'build/ninja'
        self.ninja.write_text('#!/bin/sh\necho "ninja: no work to do."\n')
        self.ninja.chmod(0o755)
        self.env = dict(isolated_make_environment(), AGIRU_BC_SOURCE=str(self.root / 'al'),
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

    def test_scope_receipts_keep_excluded_methods_without_invoking_them(self):
        tests = self.root / 'al/Layers/W1/Tests'
        (tests / 'Excluded.al').write_text('codeunit 50101 "Excluded UT" { Subtype = Test; '
                                         '[Test] procedure Removed() begin end; }')
        (self.root / 'scope.json').write_text(json.dumps({'include': ['Microsoft'], 'exclude': [],
            'product_exclude': ['bc-licensing:Layers/W1/Tests/Excluded.al']}))
        self.runner_output('2 of 2 passed')
        result = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        output = self.root / 'build/ut.log'
        raw = json.loads(Path(str(output) + '.raw-manifest.json').read_text())
        kept = json.loads(Path(str(output) + '.manifest.json').read_text())
        excluded = json.loads(Path(str(output) + '.excluded.json').read_text())
        self.assertEqual([entry['id'] for entry in raw], [50100, 50101])
        self.assertEqual([entry['id'] for entry in kept], [50100])
        self.assertEqual((excluded[0]['id'], excluded[0]['methods'], excluded[0]['reason']),
                         (50101, ['Removed'], 'bc-licensing'))
        receipt = json.loads(Path(str(output) + '.run.json').read_text())
        self.assertEqual((receipt['raw_methods'], receipt['selected_methods'], receipt['excluded_methods']),
                         (3, 2, 1))
        self.assertEqual(receipt['scope_sha256'],
                         hashlib.sha256((self.root / 'scope.json').read_bytes()).hexdigest())

    def test_zero_selected_population_refuses_and_retains_raw_receipts(self):
        (self.root / 'scope.json').write_text(json.dumps({'include': ['Microsoft'], 'exclude': [],
            'product_exclude': ['bc-licensing:Layers/W1/Tests/Example.al']}))
        self.runner_output('2 of 2 passed')
        result = subprocess.run(self.command, env=self.env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
        self.assertIn('scope selects no UT codeunits', result.stderr)
        self.assertNotIn('UT MILESTONE: 0 of 0', result.stdout)
        self.assertFalse((self.root / 'build/runner.pid').exists())
        raw = json.loads((self.root / 'build/ut.log.raw-manifest.json').read_text())
        self.assertEqual(raw[0]['methods'], ['First', 'Second'])

    def test_selected_build_controls_runner_freshness_hashes_and_nested_make(self):
        self.runner_output('2 of 2 passed')
        selected = self.root / 'build/native'
        selected.mkdir()
        shutil.copyfile(self.runner, selected / 'agiru')
        (selected / 'agiru').chmod(0o755)
        (selected / 'build.ninja').write_text('')
        (selected / 'libagiru_fixture.so').write_text('selected native library')
        self.runner.write_text('#!/bin/sh\necho wrong-root-image >&2\nexit 42\n')
        self.ninja.write_text('''#!/bin/sh
[ "$2" = "$EXPECTED_BUILD" ] || exit 43
printf 'ninja: no work to do.\\n'
''')
        make = self.root / 'build/make'
        make.write_text('''#!/bin/sh
[ "$2" = "B=$EXPECTED_BUILD" ] || exit 44
exit 0
''')
        make.chmod(0o755)
        env = dict(self.env, B='build/native', EXPECTED_BUILD=str(selected))
        result = subprocess.run(self.command + ['--build'], env=env,
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        metadata = json.loads((self.root / 'build/ut.log.run.json').read_text())
        self.assertEqual(metadata['build_directory'], str(selected))
        self.assertEqual(metadata['image_sha256']['agiru'], milestone.file_sha256(selected / 'agiru'))
        self.assertEqual(metadata['image_sha256']['libagiru_fixture.so'],
                         milestone.file_sha256(selected / 'libagiru_fixture.so'))
        result = subprocess.run(self.command, env=dict(env, B=str(selected)),
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_absent_selected_image_cannot_fall_back_to_the_root_image(self):
        self.runner_output('2 of 2 passed')
        selected = self.root / 'build/native'
        selected.mkdir()
        (selected / 'build.ninja').write_text('')
        result = subprocess.run(self.command, env=dict(self.env, B=str(selected)),
                                capture_output=True, text=True)
        self.assert_preflight_population(result, 'No such file')

    def test_make_forwards_explicit_build_and_master_without_changing_the_default(self):
        shutil.copyfile(SCRIPT.parents[2] / 'Makefile', self.root / 'Makefile')
        receipt = self.root / 'ut-arguments.json'
        (self.root / 'scripts/ut_milestone.py').write_text(
            'import json, os, pathlib, sys\n'
            + f'pathlib.Path({str(receipt)!r}).write_text(json.dumps('
            + '{"B":os.environ["B"],"arguments":sys.argv[1:]}))\n')
        master = 'postgresql://fixture.invalid/qualified_seed'
        selected = self.root / 'build/native'
        result = subprocess.run([shutil.which('make'), '-C', str(self.root), 'ut',
                                 f'B={selected}', f'UT_MASTER_DSN={master}', 'JOBS=2'],
                                env=self.env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        invocation = json.loads(receipt.read_text())
        self.assertEqual(invocation['B'], str(selected))
        self.assertEqual(invocation['arguments'], [str(selected / 'ut.log'), '2', master, '--build'])
        result = subprocess.run([shutil.which('make'), '-C', str(self.root), 'ut', 'JOBS=2'],
                                env=self.env, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertEqual(json.loads(receipt.read_text())['arguments'],
                         [str(self.root / 'build/ut.log'), '2', '--build'])

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
        shutil.copyfile(SCRIPT.parents[2] / 'Makefile', self.root / 'Makefile')
        result = subprocess.run([shutil.which('make'), '-C', str(self.root), 'ut', 'JOBS=1'],
                                env=self.env, capture_output=True, text=True)
        self.assert_preflight_population(result, 'build exited 19')

    def test_parent_make_output_directory_cannot_relocate_fixture_evidence(self):
        parent_build = self.root / 'parent-build'
        parent_log = self.root / 'parent-ut.log'
        with patch.dict(os.environ, {'MAKEFLAGS': f'-- B={parent_build}',
                                    'B': str(parent_build), 'UT_LOG': str(parent_log)}):
            fixture = MilestoneGate('test_failed_build_keeps_the_source_population')
            self.addCleanup(fixture.doCleanups)
            fixture.setUp()
            fixture.test_failed_build_keeps_the_source_population()
        self.assertFalse(parent_build.exists(), 'fixture wrote into the parent build')
        self.assertFalse(parent_log.exists(), 'fixture wrote into the parent UT log')

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


class SourceRevisionGate(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.project = self.root / 'project'
        self.project.mkdir()
        owner = patch.object(milestone, 'ROOT', self.root / 'image')
        owner.start()
        self.addCleanup(owner.stop)
        environment = dict(os.environ)
        environment.pop('AGIRU_BC_REVISION', None)
        inherited = patch.dict(os.environ, environment, clear=True)
        inherited.start()
        self.addCleanup(inherited.stop)

    def repository(self, sources):
        subprocess.run(['git', '-C', str(self.project), 'init', '-q'], check=True)
        for name in sources:
            path = self.project / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text('fixture')
        subprocess.run(['git', '-C', str(self.project), 'add', '.'], check=True)
        subprocess.run(['git', '-C', str(self.project), '-c', 'user.name=Fixture',
                        '-c', 'user.email=fixture@example.invalid', 'commit', '-qm', 'fixture'],
                       check=True)
        return subprocess.run(['git', '-C', str(self.project), 'rev-parse', 'HEAD'],
                              check=True, capture_output=True, text=True).stdout.strip()

    def test_untracked_frozen_al_cannot_borrow_an_outer_git_revision(self):
        self.repository(['source.cpp'])
        frozen = self.project / 'build/frozen-bc'
        frozen.mkdir(parents=True)
        (frozen / 'Example.al').write_text('codeunit 50100 Example {}')
        self.assertIsNone(milestone.bc_source_revision(frozen))

    def test_tracked_al_outside_the_source_root_is_not_its_provenance(self):
        self.repository(['other/Tracked.al'])
        frozen = self.project / 'frozen-bc'
        frozen.mkdir()
        (frozen / 'Example.al').write_text('codeunit 50100 Example {}')
        self.assertIsNone(milestone.bc_source_revision(frozen))

    def test_real_tracked_al_checkout_keeps_its_revision_in_both_suffix_cases(self):
        for suffix in ('al', 'AL'):
            with self.subTest(suffix=suffix):
                self.project = self.root / suffix
                self.project.mkdir()
                revision = self.repository(['src/nested/Example.' + suffix])
                self.assertEqual(milestone.bc_source_revision(self.project / 'src'), revision)

    def test_non_git_source_has_unknown_revision(self):
        (self.project / 'Example.al').write_text('codeunit 50100 Example {}')
        self.assertIsNone(milestone.bc_source_revision(self.project))

    def test_explicit_frozen_revision_stays_authoritative(self):
        self.repository(['src/Example.al'])
        with patch.dict(os.environ, AGIRU_BC_REVISION='frozen-input-revision'):
            self.assertEqual(milestone.bc_source_revision(self.project / 'src'),
                             'frozen-input-revision')

    def test_raw_inventory_and_runner_share_the_revision_contract(self):
        self.assertIs(scope_inventory.bc_revision, milestone.bc_revision)
        for tracked in (False, True):
            with self.subTest(tracked=tracked):
                if tracked:
                    expected = self.repository(['src/Example.al'])
                else:
                    expected = None
                    (self.project / 'src').mkdir()
                    (self.project / 'src/Example.al').write_text('table 1 Example {}')
                self.assertEqual(scope_inventory.bc_revision(self.project / 'src', milestone.ROOT),
                                 expected)
                with patch.dict(os.environ, AGIRU_BC_REVISION='frozen-source'):
                    self.assertEqual(scope_inventory.bc_revision(self.project / 'src', milestone.ROOT),
                                     'frozen-source')


class ManifestGate(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        spec = importlib.util.spec_from_file_location(
            'ut_manifest', SCRIPT.parents[2] / 'scripts/ut_manifest.py')
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

    def test_namespace_and_multiple_objects_on_one_line_preserve_ut_identities(self):
        self.write('Example.al', 'namespace Microsoft.Fixture; '
                   'codeunit 50100 "First UT" { Subtype = Test; '
                   '[Test] procedure A() begin end; } '
                   'codeunit 50101 "Second UT" { Subtype = Test; '
                   'var "codeunit 999 Fake UT": Boolean; '
                   '[Test] procedure B() begin end; }')
        entries = self.manifest.scan(self.root)
        self.assertEqual([(entry['id'], entry['name'], entry['methods']) for entry in entries],
                         [(50100, 'First UT', ['A']), (50101, 'Second UT', ['B'])])

    def test_bom_and_uppercase_suffix_cannot_hide_a_ut_codeunit(self):
        for filename in ('Example.AL', 'Example.al'):
            with self.subTest(filename=filename):
                path = self.root / filename
                path.write_text('\ufeff\ufeff// header\n\ufeffcodeunit 50100 "Example UT" {\n'
                                'Subtype = Test; [Test] procedure Case() begin end; }')
                try:
                    self.assertEqual(self.manifest.scan(self.root)[0]['methods'], ['Case'])
                finally:
                    path.unlink()

    def test_comment_markers_inside_names_preserve_object_and_method_identity(self):
        self.write('Example.al', 'codeunit 50100 "Comment // Marker UT" {\n'
                   'Subtype = Test; [Test] procedure "/* Real ""Case"" */"() begin end; }')
        entry = self.manifest.scan(self.root)[0]
        self.assertEqual(entry['name'], 'Comment // Marker UT')
        self.assertEqual(entry['methods'], ['/* Real "Case" */'])

    def test_quoted_identifier_contents_cannot_invent_test_attributes(self):
        self.write('Example.al', 'codeunit 50100 "Example UT" {\n'
                   'Subtype = Test; var "[Test] procedure Ghost()": Boolean;\n'
                   '[Test] procedure Real() begin end; }')
        self.assertEqual(self.manifest.scan(self.root)[0]['methods'], ['Real'])

    def test_escaped_boundary_quotes_remain_part_of_object_and_method_names(self):
        self.write('Example.al', 'codeunit 50100 """Boundary UT" {\n'
                   'Subtype = Test; [Test] procedure """Boundary"""() begin end; }')
        entry = self.manifest.scan(self.root)[0]
        self.assertEqual(entry['name'], '"Boundary UT')
        self.assertEqual(entry['methods'], ['"Boundary"'])

    def test_literal_trailing_quote_cannot_invent_a_ut_suffix(self):
        self.write('Example.al', 'codeunit 50100 "Boundary UT""" {\n'
                   'Subtype = Test; [Test] procedure Phantom() begin end; }\n'
                   'codeunit 50101 "Real UT" {\n'
                   'Subtype = Test; [Test] procedure Real() begin end; }')
        entries = self.manifest.scan(self.root)
        self.assertEqual([(entry['id'], entry['name'], entry['methods']) for entry in entries],
                         [(50101, 'Real UT', ['Real'])])

    def test_escaped_quotes_do_not_collapse_distinct_method_identities(self):
        self.write('Example.al', 'codeunit 50100 "Boundary UT" {\n'
                   'Subtype = Test; [Test] procedure Case() begin end;\n'
                   '[Test] procedure """Case"""() begin end; }')
        self.assertEqual(self.manifest.scan(self.root)[0]['methods'], ['Case', '"Case"'])

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

    def test_partition_never_converts_missing_rules_or_namespace_omissions_into_product_exclusions(self):
        self.write('Example.al', 'namespace Microsoft.Integration; '
                   'codeunit 50100 "Example UT" { Subtype = Test; [Test] procedure Case() begin end; }')
        raw = self.manifest.scan(self.root)
        policy = {'include': ['Microsoft'], 'exclude': ['Microsoft.Integration']}
        kept, excluded = self.manifest.partition(raw, self.root, self.root, policy)
        self.assertFalse(kept)
        self.assertEqual(excluded[0]['reason'], 'selection-namespace')
        self.assertIsNone(excluded[0]['product_exclusion_reason'])
        self.assertEqual(raw[0]['methods'], ['Case'])
        with self.assertRaisesRegex(ValueError, 'include list is empty'):
            self.manifest.partition(raw, self.root, self.root, dict(policy, include=[]))
        with self.assertRaisesRegex(ValueError, 'target is missing'):
            self.manifest.partition(raw, self.root, self.root, dict(policy,
                product_exclude=['microsoft-cloud:Missing.al']))


class ResultIdentityGate(unittest.TestCase):
    def setUp(self):
        spec = importlib.util.spec_from_file_location('ut_results', SCRIPT.parents[2] / 'scripts/ut_results.py')
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


class TableSourceBindingGate(unittest.TestCase):
    def setUp(self):
        self.root = SCRIPT.parents[2]
        self.build = (self.root / Path(os.environ.get('B', 'build'))).resolve()
        self.fixtures = self.root / 'test/transpiler/source-binding'
        cache = (self.build / 'CMakeCache.txt').read_text()
        compiler = re.search(r'^CMAKE_CXX_COMPILER:[^=]+=(.+)$', cache, re.M)
        self.assertIsNotNone(compiler, 'configured compiler is missing')
        self.compiler = compiler[1]
        database = re.search(r'^AGIRU_TEST_DSN:[^=]+=(.+)$', cache, re.M)
        self.assertIsNotNone(database, 'configured gate database is missing')
        self.database = database[1]

    def run_fixture(self, receiver_kind, numeric):
        with tempfile.TemporaryDirectory() as temp:
            fixture = Path(temp) / 'fixture'
            shutil.copytree(self.fixtures, fixture)
            caller = fixture / 'al/fixture/Caller.Codeunit.al'
            text = caller.read_text()
            if numeric:
                self.assertEqual(text.count('Record "Source Row"'), 3)
                text = text.replace('Record "Source Row"', 'Record 50170')
            consumer = fixture / 'Consumer.cpp'
            consumer_text = (fixture / 'Consumer.cpp.in').read_text()
            if receiver_kind == 'table':
                self.assertEqual(text.count('codeunit 50172 Caller\n{'), 1)
                text = text.replace('codeunit 50172 Caller\n{',
                                    'table 50172 Caller\n{\n'
                                    '    fields { field(1; ID; Integer) { } }')
                caller.unlink()
                caller = caller.with_name('Caller.Table.al')
                self.assertEqual(consumer_text.count('fixture/codeunit/Caller.h'), 1)
                self.assertEqual(consumer_text.count('Caller_Codeunit'), 1)
                consumer_text = (consumer_text
                    .replace('fixture/codeunit/Caller.h', 'fixture/table/Caller.h')
                    .replace('Caller_Codeunit', 'Caller_Table'))
            consumer.write_text(consumer_text)
            caller.write_text(text)
            output = Path(temp) / 'generated'
            generated = subprocess.run([
                str(self.build / 'agirutc'), str(fixture / 'al'),
                str(fixture / 'apps.json'), str(output)],
                cwd=self.root, capture_output=True, text=True, timeout=30)
            self.assertEqual(generated.returncode, 0, generated.stdout + generated.stderr)
            executable = Path(temp) / 'consumer'
            command = [self.compiler, '-std=c++23', '-stdlib=libc++',
                '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
                '-Wall', '-Wextra', '-Wpedantic', '-Werror', f'-I{self.root / "include"}',
                f'-I{self.root / "test/gate"}', f'-DAGIRU_TEST_DSN="{self.database}"',
                *(f'-I{path}' for path in (output, output / 'fixture', output / 'shared',
                                          output / 'absent')),
                *(str(path) for path in sorted(output.rglob('*.cpp'))),
                str(fixture / 'Consumer.cpp'), f'-L{self.build}', f'-Wl,-rpath,{self.build}',
                '-lagiru_rt', '-lagiru_al', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
            compiled = subprocess.run(command, cwd=self.root, capture_output=True,
                                      text=True, timeout=60)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            result = subprocess.run([str(executable)], cwd=self.root, capture_output=True,
                                    text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)

    def test_codeunit_calls_merged_table_signatures(self):
        for numeric in (False, True):
            with self.subTest(numeric=numeric):
                self.run_fixture('codeunit', numeric)

    def test_table_calls_merged_table_signatures(self):
        for numeric in (False, True):
            with self.subTest(numeric=numeric):
                self.run_fixture('table', numeric)


class PageRecordBindingGate(unittest.TestCase):
    def test_native_page_options_compile_and_execute_without_a_copied_ast(self):
        root = SCRIPT.parents[2]
        build = (root / Path(os.environ.get('B', 'build'))).resolve()
        cache = (build / 'CMakeCache.txt').read_text()
        compiler = re.search(r'^CMAKE_CXX_COMPILER:[^=]+=(.+)$', cache, re.M)
        self.assertIsNotNone(compiler, 'configured compiler is missing')
        for alias in ('Field', '2000000041'):
            with self.subTest(alias=alias), tempfile.TemporaryDirectory() as temp:
                fixture = Path(temp) / 'fixture'
                shutil.copytree(root / 'test/transpiler/page-record-binding', fixture)
                page = fixture / 'al/fixture/RecordBinding.Page.al'
                text = page.read_text()
                self.assertEqual(text.count('SourceTable = Field;'), 1)
                text = text.replace('SourceTable = Field;', f'SourceTable = {alias};')
                text = text.replace('Record Field temporary;', f'Record {alias} temporary;')
                if alias.isdecimal():
                    for name, number in (('Page Metadata', 2000000138), ('Table Metadata', 2000000136)):
                        self.assertEqual(text.count(f'Record "{name}" temporary;'), 1)
                        text = text.replace(f'Record "{name}" temporary;', f'Record {number} temporary;')
                    self.assertEqual(text.count('Record "Page Metadata";'), 1)
                    text = text.replace('Record "Page Metadata";', 'Record 2000000138;')
                page.write_text(text)
                output = Path(temp) / 'generated'
                generated = subprocess.run([
                    str(build / 'agirutc'), str(fixture / 'al'),
                    str(fixture / 'apps.json'), str(output)],
                    cwd=root, capture_output=True, text=True, timeout=30)
                self.assertEqual(generated.returncode, 0, generated.stdout + generated.stderr)
                body = next(output.rglob('RecordBinding.cpp')).read_text()
                self.assertNotIn('RefusedOption', body)
                self.assertIn('::agiru::platform::FieldClass::FlowFilter', body)
                for vocabulary in ('FieldDataType', 'FieldDataClassification',
                                   'FieldSQLDataType', 'FieldAccess', 'ObsoleteState',
                                   'PageMetadataPageType', 'TableMetadataTableType',
                                   'TableMetadataObsoleteState', 'TableMetadataCompressionType',
                                   'TableMetadataScope', 'TableMetadataAccess'):
                    self.assertIn(f'::agiru::platform::{vocabulary}::', body)
                executable = Path(temp) / 'consumer'
                command = [compiler[1], '-x', 'c++', '-std=c++23', '-stdlib=libc++',
                    '--rtlib=compiler-rt', '--unwindlib=libunwind', '-fuse-ld=lld-19',
                    '-Wall', '-Wextra', '-Wpedantic', '-Werror', f'-I{root / "include"}',
                    *(f'-I{path}' for path in (output, output / 'fixture', output / 'shared',
                                              output / 'absent')),
                    *(str(path) for path in sorted(output.rglob('*.cpp'))),
                    str(fixture / 'Consumer.cpp.in'), f'-L{build}', f'-Wl,-rpath,{build}',
                    '-lagiru_rt', '-lagiru_al', '-lagiru_net', '-lagiru_db', '-o', str(executable)]
                compiled = subprocess.run(command, cwd=root, capture_output=True,
                                          text=True, timeout=60)
                self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
                result = subprocess.run([str(executable)], cwd=root, capture_output=True,
                                        text=True, timeout=10)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
