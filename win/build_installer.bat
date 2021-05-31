@echo off
setlocal EnableDelayedExpansion
del deploy /s /q
rmdir deploy /s /q
mkdir deploy
copy files.wxs deploy\
copy dialog.bmp deploy\
copy license.rtf deploy\
copy ..\builddir\jacktrip.exe deploy\
cd deploy
set PATH=%PATH%;C:\Program Files (x86)\WiX Toolset v3.11\bin;C:\Qt\5.15.2\mingw81_64\bin
windeployqt jacktrip.exe
copy "C:\Qt\Tools\mingw810_64\bin\libgcc_s_seh-1.dll" .\
copy "C:\Qt\Tools\mingw810_64\bin\libstdc++-6.dll" .\
copy "C:\Qt\Tools\mingw810_64\bin\libwinpthread-1.dll" .\
copy "C:\Qt\Tools\OpenSSL\Win_x64\bin\libcrypto-1_1-x64.dll" .\
copy "C:\Qt\Tools\OpenSSL\Win_x64\bin\libssl-1_1-x64.dll" .\
.\jacktrip --test-gui
if %ERRORLEVEL% NEQ 0 (
	echo "You need to build jacktrip with gui support to build the installer."
	exit /b 1
)
rem Get our version number
for /f "tokens=*" %%a in ('.\jacktrip -v ^| findstr VERSION') do for %%b in (%%~a) do set VERSION=%%b
echo Version=%VERSION%
for /f "tokens=* delims=" %%a in (..\jacktrip.wxs.template) do (
	set line=%%a
	set line=!line:$VERSION=%VERSION%!
	echo !line! >> jacktrip.wxs
)
candle.exe jacktrip.wxs files.wxs
light.exe -ext WixUIExtension -o JackTrip.msi jacktrip.wixobj files.wixobj
endlocal
