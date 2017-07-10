#!/usr/bin/env python
import os
import sys
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('modules', nargs='*')
    parser.add_argument('-o', metavar='output', help='Output file', nargs=1)
    parser.add_argument('-r', metavar='root', help='Engine root path', nargs=1)
    
    try:
        args = parser.parse_args()
    except SystemExit as e:
        print 'Argument were:', str(sys.argv)
        raise

    output = 'out.cpp'
    if args.o is not None:
        output = args.o[0]

    rootpath = '.'
    if args.r is not None:
        rootpath = args.r[0]

    # open output file
    outfile = open(output, 'w')

    if (len(args.modules) > 0):
        outfile.write('#include "ModuleManager/ModuleManager.h"\n')
        outfile.write('#include "ModuleManager/IModule.h"\n')

        included_modules = []

        for module in args.modules:
            module_header = 'Modules/%s/Sources/%sModule.h' % (module, module)
            if os.path.isfile(rootpath + '/' + module_header):
                included_modules.append(module)
                outfile.write('#include "%s"\n' % module_header)

        outfile.write('\n')
        outfile.write('namespace DAVA\n')
        outfile.write('{\n')
        outfile.write('Vector<IModule*> CreateModuleInstances(Engine* engine)\n')
        outfile.write('{\n')
        outfile.write('  Vector<IModule*> modules;\n')

        for module in included_modules:
            outfile.write('  modules.emplace_back(new %sModule(engine));\n' % module)

        outfile.write('  return modules;\n')

        outfile.write('}\n')
        outfile.write('} // namespace DAVA\n')
        
    outfile.close()

if __name__ == '__main__':
    main()
