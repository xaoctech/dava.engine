#!/usr/bin/env python
import sys
import os
import argparse

import subprocess

class Preparation:

    white_list_path = []
    delete_files = []

    def __init__(self, base_path, repo):
        self.repo = repo
        self.base_path = os.path.abspath(base_path)
        self.get_white_list()
        self.get_delete_list()
        self.remove_extra()

    def scan_paths(self):
        paths = []

        for root, dirs, files in os.walk(self.base_path):
            for file in files:
                paths.append(os.path.join(root, file))

        return paths

    def get_white_list(self):
        try:
            files_list = open('white_list.txt')
            for path in files_list:
                self.white_list_path.append(os.path.abspath('../../../../' + path.replace('\n', '')))
        except IOError:
            print "##teamcity[message text='Unable to open file white_list.txt' status='ERROR']"
            sys.exit(3)

    def get_delete_list(self):
        root_path = self.scan_paths()
        sys.stdout.write('Files to delete from repo:\n')
        sys.stdout.flush()

        for path in root_path:
            if not [True for white_path in self.white_list_path if white_path in path]:
                sys.stdout.write(path.replace(self.base_path, '') + '\n')
                sys.stdout.flush()
                self.delete_files.append(path)

        self.delete_files.append(os.path.abspath('white_list.txt'))

    def remove_extra(self):

        for path in self.delete_files:

            os.chdir(os.path.dirname(path))

            cmd = 'java -jar bfg-1.12.15.jar --delete-files %s  %s' % (os.path.basename(path), self.repo)

            sys.stdout.write(cmd)
            sys.stdout.flush()
            try:
                cmd_log = subprocess.check_output(cmd, shell=True)
                sys.stdout.write(cmd_log)
                sys.stdout.flush()
            except subprocess.CalledProcessError as cmd_except:
                print "##teamcity[message text='Error removing extra files' errorDetails='%s' status='ERROR']" % cmd_except.output
                sys.exit(3)

    def remove_big_files(self):
        cmd = 'java -jar bfg-1.12.15.jar --strip-blobs-bigger-than 99M %s' % self.repo
        sys.stdout.write(cmd)
        sys.stdout.flush()
        try:
            cmd_log = subprocess.check_output(cmd, shell=True)
            sys.stdout.write(cmd_log)
            sys.stdout.flush()
        except subprocess.CalledProcessError as cmd_except:
            print "##teamcity[message text='Error removing big files' errorDetails='%s' status='ERROR']" % cmd_except.output
            sys.exit(3)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Utility tool to delete files from the repository that are not in the whitelist.')
    parser.add_argument('--path', required=True, help='Example: --path /var/git/repo')
    parser.add_argument('--repo', required=True, help='Example: --repo my-repo.git')

    args = parser.parse_args()

    preparation = Preparation(args.path, args.repo)


