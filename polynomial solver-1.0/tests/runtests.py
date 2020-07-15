#!/usr/bin/env python3

from __future__ import print_function

import sys
from os.path import dirname, join
from subprocess import Popen, PIPE
from pprint import pprint
from optparse import OptionParser
from tempfile import mkdtemp
from shutil import rmtree

TEST_DIR = dirname(__file__)
TEST_FILE = join(TEST_DIR, 'tests.txt')

def main(args):
    parser = OptionParser()
    parser.add_option('-q', '--quiet', dest='verbose', default=True,
            action='store_false', help='Show more information')
    parser.add_option('-k', '--keep-going', dest='keep_going', default=False,
            action='store_true', help='Continue running all tests')
    options, args = parser.parse_args(args)

    tmpdir = mkdtemp()
    try:
        with open(TEST_FILE) as testfile:
            tests = list(collect_tests(testfile))
            run_all_tests(tests, options, tmpdir)
    finally:
        rmtree(tmpdir)

def run_all_tests(tests, options, tmpdir):
    '''Run all tests (don't stop on failure)'''
    total = 0
    failed = []
    for test in tests:
        if options.verbose:
            print('Run test:', test['name'], '...', end=' ')

        # Actually run the test:
        result = run_test(test, tmpdir)
        success = result['status'] == 'PASS'

        total += 1
        if options.verbose:
            print(result['status'])
        if not success:
            failed.append(test['name'])
            if options.verbose:
                print(result['msg'])
            # Exit if not continue
            if not options.keep_going:
                print('Passed %d tests' % (total-1,))
                print('Failed on: %s' % test['name'])
                return

        # Limited output in quiet mode
        if not options.verbose:
            print('.' if success else 'X', end='')
    if not options.verbose:
        print()

    # List failures if any
    if failed:
        print('')
        print('Failed tests:')
        print('-------------')
        for name in failed:
            print(name)
        print()

    # Always print this:
    print('Passed %d/%d tests' % (total - len(failed), total))
    print('%d fails' % (len(failed),))

def run_test(test, tmpdir):
    if test['type'] == 'cli':
        return run_test_cli(test, tmpdir)
    else:
        raise NotImplementedError

def collect_tests(testfile):
    test_header_lines = []
    test_lines = []
    for line in testfile:
        if line.startswith('#'):
            continue
        elif not line.strip():
            continue
        elif line.startswith('..'):
            test_prefix = '.. test:'
            if not line.startswith(test_prefix):
                raise NotImplementedError(line)
            name = line.lstrip(test_prefix).strip()
            for line in testfile:
                if line.strip():
                    break

            whitespace = ''
            for c in line:
                if c.isspace():
                    whitespace += c
                else:
                    break

            test_lines = [line.lstrip(whitespace)]
            for line in testfile:
                if not line.strip():
                    break
                elif not line.startswith(whitespace):
                    raise ValueError(line)
                test_lines.append(line.lstrip(whitespace))

            for test in parse_tests(name, test_lines):
                yield test
        else:
            raise ValueError(line)

def parse_tests(name, test_lines):
    lines = []
    count = 1
    for line in test_lines:
        if line.startswith('$'):
            if lines:
                yield parse_test({'name': name + ' ' + str(count)}, lines)
                count += 1
            lines = [line]
        else:
            lines.append(line)
    if lines:
        yield parse_test({'name': name + ' ' + str(count)}, lines)

def parse_test(test_props, test_body_lines):
    if not 'type' in test_props:
        test_props['type'] = 'cli'

    cmdline, output = test_body_lines[0], test_body_lines[1:]
    test_props['raw_cmdline'] = cmdline
    test_props['cmdline'] = parse_cmdline(cmdline)

    outlines = []
    errlines = []
    ret = 0
    for line in output:
        if line.startswith('(return):'):
            ret = int(line.split()[1])
        elif line.startswith('(stderr):'):
            errlines.append(line.split(' ', 1)[1])
        else:
            outlines.append(line)

    decode = lambda s: s.decode('ascii') if hasattr(s, 'decode') else s

    test_props['expected_output'] = {
        'stdout': decode(''.join(outlines)),
        'stderr': decode(''.join(errlines)),
        'return': ret,
    }
    return test_props

def parse_cmdline(cmdline):
    return cmdline.strip().lstrip('$').strip().split()

def is_test_header_line(line):
    return line.startswith('*-')

def strip_test_header_line(line):
    line = line[1:].lstrip('-')
    return line.strip()

def run_test_cli(test, tmpdir):
    output = get_cli_output(test['cmdline'], tmpdir)
    if output == test['expected_output']:
        status = 'PASS'
        msg = 'stdout, stderr, and return are correct'
    else:
        status = 'FAIL'
        expected = pretty_format_terminal_session(test['raw_cmdline'], test['expected_output'])
        found = pretty_format_terminal_session(test['raw_cmdline'], output)
        msg = ('\n------------------ Expected output:\n')
        msg += expected
        msg += '\n------------------ Actual output:\n'
        msg += found
        msg += '\n------------------ Difference:\n'
        msg += 'Line %d\n%r\n%r\n' % first_differing_lines(expected, found)

    if status == 'PASS' and 'compare_files' in test:
        path1, path2 = test['compare_files'].split()
        path1_tmp = path1.replace('$TMP', tmpdir)
        path2_tmp = path2.replace('$TMP', tmpdir)
        try:
            with open(path1_tmp, 'rb') as f1:
                f1text = f1.read()
            with open(path2_tmp, 'rb') as f2:
                f2text = f2.read()
        except IOError:
            status = 'FAIL'
            msg = ('\n------------------ Missing output file:\n')
            msg += 'I ran:\n    %sand the outputfile was not created' % (test['raw_cmdline'],)
            msg += ('\n----------------------------------\n')
        else:
            if f1text != f2text:
                status = 'FAIL'
                msg = ('\n------------------ Unequal output:\n')
                msg += 'I ran:\n    %sand %s differs from %s afterwards' % (test['raw_cmdline'], path1, path2)
                msg += ('\n----------------------------------\n')

    return {'status': status, 'msg': msg}

def pretty_format_terminal_session(raw_cmdline, output):
    outlines = ('(stdout): %s\n' % line for line in output['stdout'].split('\n'))
    errlines = ('(stderr): %s\n' % line for line in output['stderr'].split('\n'))
    session = '%s' % raw_cmdline
    if output['stdout']:
        session += ''.join(outlines)
    if output['stderr']:
        session += ''.join(errlines)
    session += '(return): %d\n' % output['return']
    return session

def first_differing_lines(s1, s2):
    assert s1 != s2
    lines1 = s1.split('\n')
    lines2 = s2.split('\n')
    for n in range(min(len(lines1), len(lines2))):
        if lines1[n] != lines2[n]:
            return n, lines1[n], lines2[n]
    if len(lines1) < len(lines2):
        n = len(lines1)
        return n, '', lines2[n]
    elif len(lines1) > len(lines2):
        n = len(lines2)
        return n, lines1[n], ''

def get_cli_output(cmdline, tmpdir):
    '''Execute command line and capture outputs.'''

    cmdline = [arg.replace('$TMP', tmpdir) for arg in cmdline]

    stdintext = ''
    if '<' in cmdline:
        n = cmdline.index('<')
        cmdline, (stdinfile,) = cmdline[:n], cmdline[n+1:]
        with open(stdinfile, 'rb') as fin:
            stdintext = fin.read()

    try:
        p = Popen(cmdline, stdin=PIPE, stdout=PIPE, stderr=PIPE)
    except OSError as e:
        # File not found (executable doesn't exist)
        if e.errno == 2:
            return no_such_file(cmdline[0])
        else:
            raise

    stdout, stderr = p.communicate(input=stdintext)
    ret = p.returncode

    stdout = stdout.decode('latin1')
    stderr = stderr.decode('latin1')

    stdout = stdout.replace('\r\n', '\n')
    stderr = stderr.replace('\r\n', '\n')

    return {'stdout': stdout, 'stderr': stderr, 'return': ret}


def no_such_file(cmd):
    '''Mimic bash shell for non-existent executable'''
    return {
        'stderr': '-bash: %s: No such file or directory\n' % (cmd,),
        'stdout': '',
        'return': 127,
    }

if __name__ == "__main__":
    main(sys.argv)
