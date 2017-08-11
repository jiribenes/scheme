#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os.path as P
import os
import re
from subprocess import Popen, PIPE
from sys import exit
from threading import Timer

DEBUG_PATTERN = re.compile(r'DEBUG.*')
RESULT_PATTERN = re.compile(r'Result: .*')
ERROR_PATTERN = re.compile(r'ERROR', re.IGNORECASE)
TRUE_PATTERN = re.compile(r'#t')
FALSE_PATTERN = re.compile(r'#f')


def run_test(test_path, expected):
    passed = 0
    proc = Popen(
        ['./scheme.out', test_path], stdin=PIPE, stdout=PIPE, stderr=PIPE)

    timeout = [False]
    def callback(proc):
        timeout[0] = True
        proc.kill()

    timer = Timer(5, callback, [proc])

    try:
        timer.start()
        out, err = proc.communicate()

        if timeout[0]:
            return [], ["timed out"]
    finally:
        timer.cancel()

    out = out.decode('utf-8').replace('\r\n', '\n')
    err = err.decode('utf-8').replace('\r\n', '\n')

    test_no = 1
    line_no = 0
    failed = []
    errors = []

    for line in out.split('\n'):
        line_no += 1

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
            failed.append((line_no, test_no, line))

        test_no += 1

    for line in err.split('\n'):
        match = ERROR_PATTERN.search(line)
        if match:
            errors.append(line)

    if (passed + len(failed) != expected):
        print('Something went wrong in {}!'.format(test_path))

    return failed, errors


def count_tests(test_path):
    count = 0
    with open(test_path) as f:
        for line in f:
            count += line.count('(test')
    return count


def main(basepath):
    total = 0
    total_failed = 0
    total_errors = 0

    # Recursively finds all scm test files in test/ folder
    tests = [
        P.join(dirpath, f)
        for (dirpath, dirnames, files) in os.walk(basepath) for f in files
        if f.endswith('.scm')
    ]

    for test in tests:
        expected = count_tests(test)
        failed, errors = run_test(test, expected)

        total += expected
        total_failed += len(failed)
        total_errors += len(errors)

        for line_no, test_no, fail in failed:
            print('FAIL: file {} - line {}: test #{} (absolute path: {})!'.
                  format(P.basename(test), line_no, test_no, test))

        for err in errors:
            print('ERROR: file {}: {}'.format(P.basename(test), err))

    print('Tests finished! {} succeeded, {} failed, {} errors!'.format(
        total - total_failed, total_failed, total_errors))

    # If any test fail or error unexpectedly, return EXIT_FAILURE
    if (total_failed > 0 or total_errors > 0):
        exit(1)


if __name__ == '__main__':
    basepath = P.dirname(P.realpath(__file__))

    main(basepath)
