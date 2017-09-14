@echo off

set %distribeast_version=0.3.42.1
set %distribeast_package_code=4A27898E-2F67-4CE2-9C0F-CECF93843217

ver | %SystemRoot%\system32\find " 5." > nul
if %ERRORLEVEL% == 0 goto ver_xp

goto ver_vista_and_above

:ver_xp
set updatepath=%SystemDrive%\Documents and Settings\NetworkService\Local Settings\Application Data
goto install


:ver_vista_and_above
set updatepath=%SystemRoot%\ServiceProfiles\NetworkService\AppData\Local
goto install


:install
rem echo "%updatepath%\IlluminateLabs\Distribeast\updates\%distribeast_version%\windows\x86\"

mkdir "%updatepath%\IlluminateLabs\Distribeast\updates\%distribeast_version%\windows\x86\" >nul 2>&1
copy "%~d0%~p0distribeast.msi" "%updatepath%\IlluminateLabs\Distribeast\updates\%distribeast_version%\windows\x86\%distribeast_package_code%.msi" >nul

msiexec /i "%~d0%~p0distribeast.msi"

