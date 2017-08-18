#!/usr/bin/env python
import sys
import os
import argparse
import json
import time
import team_city_api
import stash_api
import common_tool

# The script runs the teamcity-build or executes the command if the
# changes in the pull-request affect the specified folder list

def __parser_args():
    arg_parser = argparse.ArgumentParser()

    stash_api.argparse_add_argument( arg_parser )
    team_city_api.argparse_add_argument( arg_parser )
##
    arg_parser.add_argument( '--framework_branch', required = True )
##
    #'ResourceEditor':'Sources/Internal;Modules;Sources/CMake;Programs/ResourceEditor','QuickEd':'Sources/Internal;Modules;Sources/CMake;Programs/QuickEd'
    arg_parser.add_argument( '--target_folders_list', required = True  )
##
    return arg_parser.parse_args()

def check_depends_of_targets_on_folders( args  ):
    stash = stash_api.ptr()

    pull_requests_number = common_tool.get_pull_requests_number( args.framework_branch )

    if pull_requests_number == None  :
        return 'ALL_BUILD'

    branch_info = ''
    branch_info = stash.get_pull_requests_info( pull_requests_number )

    merged_branch = branch_info['toRef']['id'].split('/').pop()

    if merged_branch != 'development' :
        return 'ALL_BUILD'

    #changes folders check
    branch_changes =  stash.get_pull_requests_changes( pull_requests_number )[ 'values' ]

    #load target_folders_list param
    args.target_folders_list = args.target_folders_list.replace( '+' , ';')
    args.target_folders_list = args.target_folders_list.replace( '\'' , '\"')
    args.target_folders_list = args.target_folders_list.replace( ',' , ',\n')
    args.target_folders_list = '{{\n{}\n}}'.format( args.target_folders_list )

    target_folders_list = json.loads(args.target_folders_list)

    build_target_list = []

    for target in target_folders_list:
        next_target = False

        depends_folders = target_folders_list[target].split(';')
        for path_dep_folder in depends_folders:

            if next_target == True:
                break;

            for path_branch_folder in branch_changes:
                path                = path_branch_folder['path']['parent']
                path                = os.path.realpath( path )
                realpath_dep_folder = os.path.realpath(path_dep_folder)

                if realpath_dep_folder in path:
                    next_target = True
                    build_target_list += [target]
                    break

    return ';'.join(build_target_list)

def main():

    args = __parser_args()

    stash = stash_api.init_args(args)
    teamcity = team_city_api.init_args(args)

    common_tool.flush_print( check_depends_of_targets_on_folders( args ) )

if __name__ == '__main__':
    main()
