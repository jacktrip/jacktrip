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

REM get opengl32sw.dll mesa3d llvm build, required by opengl software backend
curl -L -s -o opengl32sw.zip https://files.jacktrip.org/contrib/opengl32sw.zip
unzip opengl32sw.zip
del opengl32sw.zip
move opengl32sw.dll deploy

copy ..\LICENSE.md deploy\
xcopy ..\LICENSES deploy\LICENSES\

REM create RTF file with licenses' text
set LICENSEPATH=deploy\license.rtf
pandoc -s -f markdown -t rtf -o deploy\license.rtf ..\LICENSE.md

if "%~1"=="/q" (
	copy dialog_alt.bmp deploy\dialog.bmp
) else (
	copy dialog.bmp deploy\
)

if exist ..\builddir\release\jacktrip.exe (set JACKTRIP=..\builddir\release\jacktrip.exe) else (set JACKTRIP=..\builddir\jacktrip.exe)
copy %JACKTRIP% deploy\
if exist ..\buildstatic\src\vst3\JackTrip.vst3 (
	echo Including JackTrip.vst3
	mkdir deploy\JackTrip.vst3
	mkdir deploy\JackTrip.vst3\Contents
	xcopy /E ..\src\vst3\resources deploy\JackTrip.vst3\Contents\Resources\
	copy ..\src\images\background.png deploy\JackTrip.vst3\Contents\Resources\
	copy ..\src\images\background_2x.png deploy\JackTrip.vst3\Contents\Resources\
	copy ..\src\images\Sercan_Moog_Knob.png deploy\JackTrip.vst3\Contents\Resources\
	copy ..\src\images\Dual_LED.png deploy\JackTrip.vst3\Contents\Resources\
	copy ..\LICENSE.md deploy\JackTrip.vst3\Contents\Resources\LICENSE.md
	xcopy /E ..\LICENSES deploy\JackTrip.vst3\Contents\Resources\LICENSES\
	mkdir deploy\JackTrip.vst3\Contents\x86_64-win
	copy ..\buildstatic\src\vst3\JackTrip.vst3 deploy\JackTrip.vst3\Contents\x86_64-win\JackTrip.vst3
)
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
		echo Including QML files
		windeployqt -release --qmldir ..\..\src\vs jacktrip.exe
		set WIXDEFINES=%WIXDEFINES% -dvs
	) else (
		echo Not including QML files
		windeployqt -release jacktrip.exe
	)
	set WIXDEFINES=!WIXDEFINES! -ddynamic -dqt%QTVERSION%
)

.\jacktrip --test-gui
if %ERRORLEVEL% NEQ 0 (
	echo You need to build jacktrip with gui support to build the installer.
	exit /b 1
)

rem Get our version number
for /f "tokens=*" %%a in ('.\jacktrip -v ^| findstr VERSION') do for %%b in (%%~a) do set VERSION=%%b
for /f "tokens=1 delims=-" %%a in ("%VERSION%") do set VERSION=%%a
echo Version=%VERSION%

rem Build the MSI installer
if exist JackTrip.vst3 (
	powershell -Command "(gc JackTrip.vst3\Contents\Resources\moduleinfo.json) -replace '%%VERSION%%', '%VERSION%' | Out-File -encoding ASCII JackTrip.vst3\Contents\Resources\moduleinfo.json"
	candle.exe -arch x64 -ext WixUIExtension -ext WixUtilExtension -dvst=true -dVersion=%VERSION%%WIXDEFINES% ..\jacktrip.wxs ..\jacktrip-vst3.wxs ..\qt%QTVERSION%.wxs
	light.exe -ext WixUIExtension -ext WixUtilExtension -o JackTrip.msi jacktrip.wixobj jacktrip-vst3.wixobj qt%QTVERSION%.wixobj
) else (
	candle.exe -arch x64 -ext WixUIExtension -ext WixUtilExtension -dVersion=%VERSION%%WIXDEFINES% ..\jacktrip.wxs ..\qt%QTVERSION%.wxs
	light.exe -ext WixUIExtension -ext WixUtilExtension -o JackTrip.msi jacktrip.wixobj qt%QTVERSION%.wixobj
)

rem Compile the bundle but don't build it yet
candle.exe -arch x64 -ext WixBalExtension -dVersion=%VERSION% ..\jacktrip-bundle.wxs

endlocal
