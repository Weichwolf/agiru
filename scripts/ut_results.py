#!/usr/bin/env python3
"""Reconcile completed test identities with the independent AL source manifest."""
import json
from pathlib import Path
import re


def reconcile(entry, records, log, status):
    expected = set(entry['methods'])
    seen = {name: [] for name in expected}
    errors = []
    for number, line in enumerate(records.splitlines(), 1):
        try:
            row = json.loads(line)
        except json.JSONDecodeError:
            errors.append(f'invalid JSON result at line {number}')
            continue
        if (not isinstance(row, dict) or row.get('schema') != 1
                or type(row.get('codeunit_id')) is not int
                or row['codeunit_id'] != entry['id']
                or row.get('codeunit') != entry['name']
                or not isinstance(row.get('method'), str)
                or row['method'] not in expected
                or type(row.get('passed')) is not bool
                or not isinstance(row.get('error'), str)):
            errors.append(f'unexpected or malformed result at line {number}')
            continue
        seen[row['method']].append(row)
    methods = []
    for name in entry['methods']:
        matches = seen[name]
        if len(matches) != 1:
            state = 'missing' if not matches else 'duplicate'
            errors.append(f'{state} method {name}')
            methods.append({'codeunit_id': entry['id'], 'codeunit': entry['name'],
                            'method': name, 'passed': False, 'status': state})
        else:
            methods.append(dict(matches[0], status='passed' if matches[0]['passed'] else 'failed'))
    passed = sum(row['passed'] for row in methods)
    summaries = re.findall(r'^(\d+) of (\d+) passed$', log, re.M)
    if summaries != [(str(passed), str(len(expected)))]:
        errors.append(f'summary {summaries} does not match {passed}/{len(expected)} method results')
    expected_status = 0 if passed == len(expected) else 1
    if status != expected_status:
        errors.append(f'process exit {status}, expected {expected_status}')
    return methods, errors


def aggregate(manifest, parts, output, workers, elapsed):
    passed = 0
    incomplete = 0
    expected = sum(len(entry['methods']) for entry in manifest)
    with Path(output).open('w') as report, Path(str(output) + '.results.jsonl').open('w') as structured:
        for entry in manifest:
            stem = Path(parts) / str(entry['id'])
            log = stem.with_suffix('.log').read_text(errors='replace') if stem.with_suffix('.log').exists() else ''
            records = stem.with_suffix('.jsonl').read_text(errors='replace') if stem.with_suffix('.jsonl').exists() else ''
            try:
                status = int(stem.with_suffix('.status').read_text())
            except (OSError, ValueError):
                status = -1
            methods, errors = reconcile(entry, records, log, status)
            report.write(log)
            passed += sum(row['passed'] for row in methods)
            for row in methods:
                structured.write(json.dumps(row) + '\n')
            if errors:
                incomplete += 1
                report.write(f"INCOMPLETE {entry['name']}: " + '; '.join(errors) + '\n')
        summary = (f'UT MILESTONE: {passed} of {expected} over {len(manifest)} codeunits, '
                   f'{incomplete} incomplete ({workers} workers, {elapsed} s)')
        print(summary)
        report.write(summary + '\n')
    return 0 if passed == expected and incomplete == 0 else 1


if __name__ == '__main__':
    import sys
    import time
    manifest_path, parts_path, output_path, workers, start = sys.argv[1:]
    sys.exit(aggregate(json.loads(Path(manifest_path).read_text()), parts_path, output_path,
                       workers, int(time.time()) - int(start)))
