import subprocess
import os
import sys

print("current OS is ", os.name)

if os.name == "posix":
    subprocess.Popen(["python", sys.argv[1]], shell=False, cwd=sys.argv[2])
else:
    subprocess.Popen(["python", sys.argv[1]], shell=True, cwd=sys.argv[2])
