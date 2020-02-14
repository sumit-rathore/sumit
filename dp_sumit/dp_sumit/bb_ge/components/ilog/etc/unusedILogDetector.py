#!/usr/bin/python3

# This program deletes ilogs that are not referenced by any source code.  This breaks backwards
# compatibility of ilog numbers, but is necessary when the number of ilogs overflows the 256 ilog
# limit per component.

import sys
import os.path
import re

def processArgs():
    """Reads the command line arguments and validates them.

    Returnins a tuple of the ilog_file_path and component_directory_path if the arguments are
    satisfactory or exits execution if they are insufficient.
    """

    program_name = sys.argv[0]
    if len(sys.argv) != 4:
        print(
            "usage: {} ilog_file_path component_directory_path ilog_header_file_path".format(sys.argv[0]),
            file=sys.stderr)
        sys.exit(1)

    ilog_file_path = sys.argv[1]
    component_directory_path = sys.argv[2]
    ilog_header_file_path = sys.argv[3]

    if not(os.path.isfile(ilog_file_path)):
        print(
            "The ilog file path ({}) either is not a file or does not exist.".format(
                ilog_file_path),
            file=sys.stderr)
        sys.exit(1)

    if not(os.path.isdir(component_directory_path)):
        print(
            "The component directory path ({}) either is not a directory or does not exist.".format(
                component_directory_path),
            file=sys.stderr)
        sys.exit(1)

    return (ilog_file_path, component_directory_path, ilog_header_file_path)


def readILogFile(file_path):
    """Gets the names of all the ILogs from the given path"""
    ilogs = []
    with open(file_path, 'r') as f:
        exp = re.compile('L:(?P<ilog_name>\w+) S:.+')
        for line in f:
            m = exp.match(line)
            if m is not None:
                ilogs.append(m.group('ilog_name'))
    return ilogs


def find_eligible_files(search_dir, include_path_regexp, exclude_path_regexp):
    """Gives the relative path to all of the files within search_dir where the relative path
    matches the given regular expression."""
    include_exp = re.compile(include_path_regexp)
    exclude_exp = re.compile(exclude_path_regexp)
    for (dirpath, dirnames, filenames) in os.walk(search_dir):
        for f in filenames:
            p = os.path.join(dirpath, f)
            if include_exp.match(p) is not None and exclude_exp.match(p) is None:
                yield p


def search_for_ilog_usage(ilogs, search_dir, include_path_regexp, exclude_path_regexp):
    # Take a copy of ilogs so we don't manipulate the argument
    targets = [il for il in ilogs]

    for path in find_eligible_files(search_dir, include_path_regexp, exclude_path_regexp):
        with open(path, 'r') as f:
            line_num = 1
            try:
                for line in f:
                    (matched, unmatched) = check_for_matched_ilogs(targets, line)
                    for ilog in matched:
                        print("{}:{} matched {} \"{}\"".format(path, line_num, ilog, line[:-1]))
                    targets = unmatched
                    line_num += 1
            except UnicodeDecodeError:
                print("Failed to decode file: {}, line: {}".format(path, line_num))
                sys.exit(1)
    return targets


def check_for_matched_ilogs(ilogs, line):
    matched = []
    unmatched = []
    for ilog in ilogs:
        # Regular expression matches:
        # (start_of_line) or (anything + non_identifier_character) ilog_symbol (end_of_line) or (non_identifier_character + anything)
        if re.match('((^)|(.*[^_a-zA-Z0-9])){}(($)|([^_a-zA-Z0-9].*))'.format(ilog), line):
            matched.append(ilog)
        else:
            unmatched.append(ilog)
    return (matched, unmatched)


def delete_unused_ilogs(unused_ilogs, ilog_header_file_path):
    with open(ilog_header_file_path, 'r') as old_header:
        with open(ilog_header_file_path + '.new', 'w') as new_header:
            for line in old_header:
                matched = False
                for ilog in unused_ilogs:
                    if re.match('((^)|(.*[^_a-zA-Z0-9])){}(($)|([^_a-zA-Z0-9].*))'.format(ilog), line):
                        matched = True
                        break
                if not matched:
                    new_header.write(line)
    os.rename(ilog_header_file_path, ilog_header_file_path + '.orig')
    os.rename(ilog_header_file_path + '.new', ilog_header_file_path)
    os.remove(ilog_header_file_path + '.orig')



if __name__ == '__main__':
    (ilog_file_path, component_directory_path, ilog_header_file_path) = processArgs()

    ilogs = readILogFile(ilog_file_path)
    unused_ilogs = search_for_ilog_usage(
            ilogs, component_directory_path, '[\w /\.]*\w+\.[hc]$', ilog_header_file_path)

    if len(unused_ilogs) > 0:
        print("-----------------------------")
        print("The unused ilog messages are:")
        print("-----------------------------")
        for ilog in unused_ilogs:
            print(ilog)

        print("-------------------------")
        print("Deleting the unused ilogs")
        print("-------------------------")
        delete_unused_ilogs(unused_ilogs, ilog_header_file_path)
    else:
        print("All ilog message are used")

    sys.exit(0)
