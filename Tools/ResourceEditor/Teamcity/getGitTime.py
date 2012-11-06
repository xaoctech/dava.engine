import os
os.chdir('../../../')
os.system("git log -1 --format=\"%ci\" > gitTime.txt")
