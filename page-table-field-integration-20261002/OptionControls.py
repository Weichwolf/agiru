import json
from pathlib import Path
import subprocess

task = Path(__file__).resolve().parent
before = task.parent.parent
after = task / 'source'
prefix = '''#include "meta/EnumDef.h"
#include "type/Option.h"
#include <array>
#include <cstdint>
enum class Fixture : std::int32_t { First, Second };
template <> struct agiru::OptionTraits<Fixture> {
'''
suffix = '''};
using Value = agiru::Option<Fixture>;
static_assert(Value{FIRST}.Name() == "First");
static_assert(Value{SECOND}.Caption() == "Second");
static_assert(Value{-1}.Name().empty());
static_assert(sizeof(Value) == sizeof(agiru::Option<>));
'''
cases = [
    ('dense', False, [0, 1], True, True),
    ('coded', True, [31488, 31489], False, True),
    ('unmarked-sparse', False, [31488, 31489], False, False),
    ('coded-unsorted', True, [31489, 31488], False, False),
    ('coded-duplicate', True, [31488, 31488], False, False),
    ('coded-negative', True, [-1, 31488], False, False),
]
results = []
for label, coded, ordinals, old_passes, new_passes in cases:
    values = ','.join('{.ordinal = ' + str(value) + ', .name = "' + name + '", .caption = "' + name + '"}'
                      for value, name in zip(ordinals, ['First', 'Second']))
    cpp = prefix + ('static constexpr bool kCodedOrdinals = true;\n' if coded else '')
    cpp += 'static constexpr std::array<agiru::EnumValueDef, 2> kValues{{' + values + '}};\n'
    cpp += suffix.replace('FIRST', str(ordinals[0])).replace('SECOND', str(ordinals[1]))
    for version, source, passes in [('before', before, old_passes), ('after', after, new_passes)]:
        command = ['clang++-19', '-std=c++23', '-stdlib=libc++', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-I' + str(source / 'include'), '-x', 'c++', '-fsyntax-only', '-']
        result = subprocess.run(command, input=cpp, capture_output=True, text=True)
        results.append(dict(label=label, version=version, exit=result.returncode, diagnostics=result.stderr))
        assert (result.returncode == 0) == passes, results[-1]
        if not passes:
            assert 'static assertion failed' in result.stderr
(task / 'artifacts/option-controls.json').write_text(json.dumps(results, indent=2) + '\n')
print(json.dumps({'controls': len(results), 'expected_results_proved': True}))
