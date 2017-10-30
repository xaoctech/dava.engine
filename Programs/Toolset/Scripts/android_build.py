import argparse
import shutil
import sys  
import glob, os
import re
import argparse
import subprocess
import multiprocessing


DavaRootDir = os.path.realpath(os.path.join( os.path.dirname (__file__), '../../../'))
DavaProgramsDir = os.path.join( DavaRootDir, 'Programs' )

ProgramsList = [ 'UnitTests', 
                 'TestBed',
                 'SceneViewer'  
               ]


def build( program ):
    program_dir = os.path.join( DavaProgramsDir, program, 'Platforms', 'Android') 

    os.chdir(program_dir)

    os.system( 'gradle wrapper' )
    os.system( './gradlew {}:assembleFatRelease'.format( program ) )


def main():
    multiprocessing.freeze_support()

    parser = argparse.ArgumentParser()
    parser.add_argument( '--sdk_dir' )
    parser.add_argument( '--ndk_dir' )

    options = parser.parse_args()

    exists_sdk_dir = os.path.exists( options.sdk_dir ) 
    exists_ndk_dir = os.path.exists( options.ndk_dir ) 

    assert exists_sdk_dir == True
    assert exists_ndk_dir == True

    for program in ProgramsList:
        program_dir = os.path.join( DavaProgramsDir, program, 'Platforms', 'Android') 

        os.chdir(program_dir)

        local_properties_file = open( 'local.properties', 'w' )
        local_properties_file.write( 'sdk.dir={}\n'.format( options.sdk_dir) )
        local_properties_file.write( 'ndk.dir={}\n'.format( options.ndk_dir) )
        local_properties_file.close()


    pool = multiprocessing.Pool()
    pool.map(build, ProgramsList )



if __name__ == '__main__':
    main()

