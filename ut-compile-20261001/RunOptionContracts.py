from pathlib import Path
import json
import subprocess

root = Path(__file__).resolve().parent
source = root / 'source'
folder = root / 'artifacts/option-contracts'
folder.mkdir(exist_ok=True)
cases = [('ordinary-dense', (0, 1), False, 0),
         ('ordinary-sparse', (4912, 31489), False, 1),
         ('native-coded', (4912, 31489), True, 0),
         ('native-unsorted', (31489, 4912), True, 1),
         ('native-duplicate', (4912, 4912), True, 1)]
results = []
for tag, cxx in (('clang', 'clang++-19'), ('gcc', 'g++-14')):
    for name, codes, coded, expected in cases:
        path = folder / (name + '.cpp')
        flag = 'static constexpr bool kCodedOrdinals = true;' if coded else ''
        path.write_text('''#include "type/Option.h"
#include <array>
#include <cstdint>
enum class Members : std::int32_t { First = %d, Second = %d };
template <> struct agiru::OptionTraits<Members> {
  %s
  static constexpr std::array<agiru::EnumValueDef, 2> kValues{{
      {.ordinal = %d, .name = "First", .caption = "First"},
      {.ordinal = %d, .name = "Second", .caption = "Second"},
  }};
};
static_assert(agiru::Option<Members>{Members::First}.Name() == "First");
''' % (codes[0], codes[1], flag, codes[0], codes[1]))
        command = [cxx, '-std=c++23', '-Wall', '-Wextra', '-Wpedantic', '-Werror',
                   '-fsyntax-only', '-I' + str(source / 'include'), str(path)]
        result = subprocess.run(command, text=True, capture_output=True, timeout=60)
        valid = result.returncode == expected
        if expected != 0:
            valid = valid and 'ordinary options require dense ordinals' in result.stderr
        results.append({'compiler': tag, 'case': name, 'command': command,
                        'expected_exit': expected, 'exit': result.returncode,
                        'diagnostics': result.stderr, 'valid': valid})
receipt = {'cases': results, 'success': all(row['valid'] for row in results)}
(root / 'artifacts/option-contracts.json').write_text(json.dumps(receipt, indent=2) + '\n')
print(json.dumps({'checks': len(results), 'failed_checks': sum(not row['valid'] for row in results)}), flush=True)
raise SystemExit(0 if receipt['success'] else 1)
