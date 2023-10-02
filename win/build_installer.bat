@echo off
setlocal EnableDelayedExpansion

if not defined WIXPATH (
	for /f "delims=" %%a in ('dir /b "C:\Program Files (x86)\Wix Toolset*"') do set WIXPATH=%%a
	set WIXPATH=C:\Program Files ^(x86^)\!WIXPATH!\bin
)

set PATH=%PATH%;%WIXPATH%
del deploy /s /q
rmdir deploy /s /q
mkdir deploy

copy ..\LICENSE.md deploy\
xcopy ..\LICENSES deploy\LICENSES\

REM create RTF file with licenses' text
set LICENSEPATH=deploy\license.rtf
echo {\rtf1\ansi\deff0 {\fonttbl {\f0 Calibri;}} \f0\fs22>%LICENSEPATH%
for %%f in (..\LICENSE.md ..\LICENSES\MIT.txt ..\LICENSES\GPL-3.0.txt ..\LICENSES\LGPL-3.0-only.txt ..\LICENSES\AVC.txt) do (
  for /f "delims=" %%x in ('type %%f') do (
    echo %%x\line>>%LICENSEPATH%
  )
  echo \par >>%LICENSEPATH%
)
echo }>>%LICENSEPATH%

if "%~1"=="/q" (
    copy dialog_alt.bmp deploy\dialog.bmp
) else (
    copy dialog.bmp deploy\
)
if exist ..\builddir\release\jacktrip.exe (set JACKTRIP=..\builddir\release\jacktrip.exe) else (set JACKTRIP=..\builddir\jacktrip.exe)
copy %JACKTRIP% deploy\
cd deploy

set "WIXDEFINES="
for /f "tokens=*" %%a in ('objdump -p jacktrip.exe ^| findstr Qt5Core.dll') do (
	set DYNAMIC_QT=%%a
	set QTVERSION=5
)
if not defined DYNAMIC_QT (
	for /f "tokens=*" %%a in ('objdump -p jacktrip.exe ^| findstr Qt6Core.dll') do (
		set DYNAMIC_QT=%%a
		set QTVERSION=6
	)
)
if defined DYNAMIC_QT (
	echo Including Qt%QTVERSION% Files
	for /f "tokens=*" %%a in ('objdump -p jacktrip.exe ^| findstr Qt%QTVERSION%Qml.dll') do set VS=%%a
	if defined VS (
		windeployqt -release --qmldir ..\..\src\gui jacktrip.exe
		set WIXDEFINES=%WIXDEFINES% -dvs
	) else (
		windeployqt -release jacktrip.exe
	)
	set WIXDEFINES=!WIXDEFINES! -ddynamic -dqt%QTVERSION%
)

copy ..\jacktrip.wxs .\
copy ..\qt%QTVERSION%.wxs .\
.\jacktrip --test-gui
if %ERRORLEVEL% NEQ 0 (
	echo You need to build jacktrip with gui support to build the installer.
	exit /b 1
)
rem Get our version number
for /f "tokens=*" %%a in ('.\jacktrip -v ^| findstr VERSION') do for %%b in (%%~a) do set VERSION=%%b
for /f "tokens=1 delims=-" %%a in ("%VERSION%") do set VERSION=%%a
echo Version=%VERSION%
candle.exe -arch x64 -ext WixUIExtension -ext WixUtilExtension -dVersion=%VERSION%%WIXDEFINES% ..\jacktrip.wxs ..\qt%QTVERSION%.wxs
light.exe -ext WixUIExtension -ext WixUtilExtension -o JackTrip.msi jacktrip.wixobj qt%QTVERSION%.wixobj
endlocal
