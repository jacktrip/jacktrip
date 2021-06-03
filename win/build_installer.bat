@echo off
setlocal EnableDelayedExpansion

if not defined QTBINPATH (
	for /f "delims=" %%a in ('dir /b C:\Qt\5*') do set QTVERSION=%%a
	for /f "delims=" %%a in ('dir /b C:\Qt\!QTVERSION!\min*') do set QTBINPATH=%%a
	set QTBINPATH=C:\Qt\!QTVERSION!\!QTBINPATH!\bin
)
if not defined QTLIBPATH (
	for /f "delims=" %%a in ('dir /b C:\Qt\Tools\min*') do set QTLIBPATH=%%a
	set QTLIBPATH=C:\Qt\Tools\!QTLIBPATH!\bin
)
if not defined WIXPATH (
	for /f "delims=" %%a in ('dir /b "C:\Program Files (x86)\Wix Toolset*"') do set WIXPATH=%%a
	set WIXPATH=C:\Program Files ^(x86^)\!WIXPATH!\bin
)
if not defined SSLPATH (set SSLPATH=C:\Qt\Tools\OpenSSL\Win_x64\bin)

set PATH=%PATH%;%WIXPATH%
del deploy /s /q
rmdir deploy /s /q
mkdir deploy
copy dialog.bmp deploy\
copy license.rtf deploy\
if exist ..\builddir\release\jacktrip.exe (set JACKTRIP=..\builddir\release\jacktrip.exe) else (set JACKTRIP=..\builddir\jacktrip.exe)
copy %JACKTRIP% deploy\
cd deploy
set WIXFILES=files_static
for /f "tokens=*" %%a in ('%QTLIBPATH%\objdump -p jacktrip.exe ^| findstr Qt5Core.dll') do set DYNAMIC_QT=%%a
if defined DYNAMIC_QT (
	%QTBINPATH%\windeployqt jacktrip.exe
	copy "%QTLIBPATH%\libgcc_s_seh-1.dll" .\
	copy "%QTLIBPATH%\libstdc++-6.dll" .\
	copy "%QTLIBPATH%\libwinpthread-1.dll" .\
	copy "%SSLPATH%\libcrypto-1_1-x64.dll" .\
	copy "%SSLPATH%\libssl-1_1-x64.dll" .\
	set WIXFILES=files
)
copy ..\%WIXFILES%.wxs .\
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
candle.exe jacktrip.wxs %WIXFILES%.wxs
light.exe -ext WixUIExtension -o JackTrip.msi jacktrip.wixobj %WIXFILES%.wixobj
endlocal
