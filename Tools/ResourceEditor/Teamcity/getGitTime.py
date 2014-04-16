import os
import sys
os.chdir('../')
os.system("git log -1 --date=local --format=\"#define RESOURCE_EDITOR_VERSION %x22%cd | " + sys.argv[1] + "%x22\" > version.h")

os.chdir('../../')
os.system("git log -1 --date=local --format=\"%cd\" > gitTime.txt")
