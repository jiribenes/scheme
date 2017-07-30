#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from subprocess import Popen, PIPE
import os.path as P
import os
import re

DEBUG_PATTERN = re.compile(r'DEBUG.*')
RESULT_PATTERN = re.compile(r'Result: .*')
TRUE_PATTERN = re.compile(r'#t')
FALSE_PATTERN = re.compile(r'#f')


def run_test(test_path, expected):
    passed = 0
    proc = Popen(
        ['./scheme.out', test_path], stdin=PIPE, stdout=PIPE, stderr=PIPE)

    out, err = proc.communicate()

    out = out.decode('utf-8').replace('\r\n', '\n')

    test_no = 1
    failed = []

    for line in out.split('\n'):
        match = DEBUG_PATTERN.search(line)
        if match:
            continue

        match = RESULT_PATTERN.search(line)
        if match:
            continue

        match = TRUE_PATTERN.search(line)
        if match:
            passed += 1

        match = FALSE_PATTERN.search(line)
        if match:
            failed.append(test_no)

        test_no += 1

    assert passed + len(failed) == expected, """
    Something went wrong in {file}!
    Here is the output:
    {out}""".format(
        file=test_path, out=out)

    return failed


def count_tests(test_path):
    count = 0
    with open(test_path) as f:
        for line in f:
            count += line.count('(test')
    return count


def main(basepath):
    total = 0
    total_failed = 0

    # Recursively finds all scm test files in test/ folder
    tests = [
        P.join(dirpath, f)
        for (dirpath, dirnames, files) in os.walk(basepath) for f in files
        if f.endswith('.scm')
    ]

    for test in tests:
        expected = count_tests(test)
        failed = run_test(test, expected)

        total += expected
        total_failed += len(failed)

        for fail in failed:
            print('FAIL: test #{} in file: {} (absolute path: {})!'.format(
                fail, P.basename(test), test))

    print('Tests finished! {} succeeded and {} failed!'.format(
        total - total_failed, total_failed))


if __name__ == '__main__':
    basepath = P.dirname(P.realpath(__file__))

    main(basepath)
