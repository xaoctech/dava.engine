import os;
import os.path;
import subprocess;


currentDir = os.getcwd();
reseditorDir = os.path.realpath(currentDir + "/ResourceEditor/")
toolDir = os.path.realpath(currentDir + "/../../Tools/Bin/")
result = os.path.realpath(currentDir + "/Results/")

print "Convert DDS files:"
subprocess.call(reseditorDir + "/ResourceEditorQtVS2010.exe -mipmap 1 -forceclose -extract -path " + result, shell=True)

for gpu in os.listdir(result):
    for test in os.listdir(result + "/" + gpu):
        print test
        files = filter(lambda x: x[-3:] == "pvr", os.listdir(result + "/" + gpu + "/" + test))
        
        for file in files:
            print "Convert in:" + file
            pvr = os.path.realpath(result + "/" + gpu + "/" + test + "/" + file)
            print "CALL: " + toolDir + "/PVRTexToolCL.exe -d -f r8g8b8a8 -i " + pvr
            subprocess.call(toolDir + "/PVRTexToolCL.exe -d -f r8g8b8a8 -i " + pvr, shell=True)
            print "RENAME: " + os.path.realpath(pvr[:-3] + "Out.png") + " to " + os.path.realpath(pvr[:-3] + "png")
            os.rename(os.path.realpath(pvr[:-3] + "Out.png"), os.path.realpath(pvr[:-3] + "png"))