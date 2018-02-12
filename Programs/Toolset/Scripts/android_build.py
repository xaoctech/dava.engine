import argparse
import os
import subprocess
import sys


DavaRootDir = os.path.realpath(os.path.join( os.path.dirname (__file__), '../../../'))
DavaProgramsDir = os.path.join( DavaRootDir, 'Programs' )

ProgramsList = [ 'UnitTests', 
                 'TestBed',
                 'SceneViewer',
                 'UIViewer',
                 'PerformanceTests'
               ]


def build( program ):
    program_dir = os.path.join( DavaProgramsDir, program, 'Platforms', 'Android') 

    os.chdir(program_dir)

    print "===== Building % s =====" % (program)
    
    command = '{}:assembleFatRelease'.format(program);
    subprocess.check_call([program_dir+'/gradlew', command])

def main():
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
        
        build(program)

if __name__ == '__main__':
    main()

