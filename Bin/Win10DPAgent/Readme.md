#### Win10dpa (list, deploy, attach, uninstall, stop)
###### Only devices with anniversary update (>= 10.0.14393) are supported.
###### On some Win 10 builds (probably between 10.0.15043 - 10.0.16193) there is a bug in Microsoft system API that may effect attach mode.
####### details: https://wpdev.uservoice.com/forums/110705-universal-windows-platform/suggestions/18591439-loggingchannel-not-showing-string-message-content
###### At the end of this file you can find python script 'example.py' that shows how to run basic deploy & attach scenario.  
<br>

#### To install requirements run:
```
pip install lxml requests requests_toolbelt websocket_client
```

---

<br>

#### To install device portal on a device:
Both on desktop and on mobile device you need to open "settings -> update and security -> for developers" and enable device portal. 
If device portal switch turning off right after toggle, try to install latest system updates. If it doesn't help, contact your system administrator.  
Authorization is not supported, so turn it off.  

On this screen you can find IP address and port to connect to as well.

---

<br>

#### Common:   
**'-url'** is common optional flag. Default is **127.0.0.1:10080** (default ip for usb connected single device)  
**'-timeout'** is common optional flag. Default is **10.0** seconds. **10.0** should be enough.

---

<br>

#### List:
```
python win10_dpa.py list installed(running) -url=10.1.3.37 -timeout=15.0
```

Output is JSON. 
For **'installed'**: 
```
{"InstalledPackages" : [{"Name":"App name", "Version": "[1,0,0,0]", "PackageFullName":"alotofsymbolshere"}, ...]}
```
For **'running'**: 
```
{"Processes" : [{"Name":"App name", "Version": "[1,0,0,0]", "PackageFullName":"alotofsymbolshere"}, ...]}
```
**'running'** will output only apps that are currently on a device screen. 

---
<br>

#### Deploy:
```
python win10_dpa.py deploy "c:/path/to/appx/or/appxbundle/file.appx" -deps_paths="c:/path/to/specific/arch/dep/ARM/file.appx","c:/path/to/specific/arch/deps/dir/ARM" -cer_path="c:/path/to/cer/file.cer" -force -url=10.1.3.37 -timeout=15.0
```

First argument is a path to .appx or .appxbundle file to be deployed.  
**'-deps_paths'** is optional flag, specify paths to right arch (x86, x64 or ARM) dependencies. Dirs are accepted as well, all top level files in dir will be uploaded to device.  
**'-cer_path'** is optional flag, specify path to .cer file. May be useful for IoT or Windows Desktop devices.  
**'-force'** is optional flag, specify it if the same or higher app version is installed on a device.  

---

<br>

#### Attach:
```
python win10_dpa.py attach -start(-monitor) "package full name here" -guids="4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a","3bd1826e-53a1-4ba9-bf63-92b73ea1bc4b" -channels="DAVALogProvider","Test-log-service","YouNameIt" -log_levels="1,2,3,4,5" -no_timestamp -wait_time=21.0 -url=10.1.3.37 -timeout=15.0
```
  
**'-start'** is optional flag, specify PackageFullName (you can get it by app name from 'list' option) to start. Log session will be closed when app process stopped. App will be stopped and restarted if it's already running.  
**'-monitor'** is optional flag, specify PackageFullName to monitor. Use this option if app is already running (or you want to run it manualy right after log session will start) and you want to stop log session when app process stopped. After 'wait_time' (since log session start) app activity will be checked and if app process is not running, session will be closed.  
**'-guid'** is optional flag, you can specify logs provider's guid to listen to. Default is "4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a" for Microsoft-Windows-Diagnostics-LoggingChannel.  
**'-channels'** is optional flag, you can specify log channels to be shown in logs, "DAVALogProvider" and "Test-log-service" are default.  
**'-log_levels'** is optional flag, you can specify log levels to be shown in logs, "1,2,3,4,5" are set by default.  
**'-no_timestamp'** is optional flag, specify it if you don't want timestamp to be shown.   
**'-wait_time'** is optional flag, specify time after which app activity will be checked. 20.0 seconds set by default.  
  
You can attach without **'-start'** or **'-monitor'**, then you need to stop session manualy after you are done. Use ctrl+c to send interrupt signal (you may need to press it several times).  

---

<br>

#### Stop:
```
python win10_dpa.py stop "package full name here" -url=10.1.3.37 -timeout=15.0
```
App with given package full name will be stopped (app process may stay active)  

---

<br>

#### Uninstall
```
python win10_dpa.py uninstall "package full name here" -url=10.1.3.37 -timeout=15.0
```
App with given package full name will be uninstalled.  

---

<br>

#### example.py
cmd line: 
```
python example.py "app_test_etw" "C:\example\Documents\Visual Studio 2015\Projects\app_test_etw\AppPackages\app_test_etw\app_test_etw_1.0.29.0_Debug_Test\app_test_etw_1.0.29.0_x86_x64_arm_Debug.appxbundle" "C:\example\Documents\Visual Studio 2015\Projects\app_test_etw\AppPackages\app_test_etw\app_test_etw_1.0.29.0_Debug_Test\Dependencies\ARM\Microsoft.VCLibs.ARM.Debug.14.00.appx"
```

```python
import sys
import os
from json import loads
import subprocess

def main():
    package_name = sys.argv[1]
    url = "http://10.1.3.37"
    exec_string = "python win10_dpa.py {} {} -url=" + url

    env = os.environ
    env['PYTHONIOENCODING'] = 'utf-8'

    subprocess.call(exec_string.format("deploy", "\"{}\" -deps_paths=\"{}\" -force".format(sys.argv[2], sys.argv[3])), env=env)
    
    proc = subprocess.check_output(exec_string.format("list", "installed"), env=env)

    package_full_name = None
    installed_packages = loads(proc.decode("utf-8"))["InstalledPackages"]
    for package in installed_packages:
        if package["Name"] == package_name:
            if package_full_name is None:
                package_full_name = package["PackageFullName"]
            else:
                # This is just an example, you can use version check as well
                raise ValueError("Name {} is not unqiue.".format(package_name)) 

    print "Package full name for {}:{}".format(package_name, package_full_name)
    subprocess.call(exec_string.format("attach", "-start {}".format(package_full_name)), env=env)

if __name__ == "__main__":
    main()
```
