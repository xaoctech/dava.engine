import os
os.chdir('../')
os.system("git log -1 --format=\"#define RESOURCE_EDITOR_VERSION %x22%ci%x22\" > version.h")
