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

REM Download Visual C++ 2015-2022 Redistributable (x64)
echo Downloading Visual C++ Redistributable...
curl -L -s -o deploy\vc_redist.x64.exe https://aka.ms/vs/17/release/vc_redist.x64.exe
if not exist deploy\vc_redist.x64.exe (
    echo Failed to download Visual C++ Redistributable. Trying alternative URL...
    curl -L -s -o deploy\vc_redist.x64.exe https://download.microsoft.com/download/9/3/F/93FCF1E7-E6A9-4FE5-A5A4-E310F8480645/vc_redist.x64.exe
    if not exist deploy\vc_redist.x64.exe (
        echo ERROR: Failed to download Visual C++ Redistributable. Please download manually.
        echo URL: https://aka.ms/vs/17/release/vc_redist.x64.exe
        exit /b 1
    )
)
echo Visual C++ Redistributable downloaded successfully.

copy ..\LICENSE.md deploy\
xcopy ..\LICENSES deploy\LICENSES\

REM create RTF file with licenses' text
set LICENSEPATH=deploy\license.rtf
pandoc -s -f markdown -t rtf -o deploy\license.rtf ..\LICENSE.md

if "%~1"=="/q" (
    copy dialog_alt.bmp deploy\dialog.bmp
	copy jacktrip_alt.ico deploy\jacktrip.ico
) else (
    copy dialog.bmp deploy\
	copy jacktrip.ico deploy\jacktrip.ico
)
copy ..\src\images\icon_128.png deploy\icon_128.png

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

rem Build the Burn bundle
echo Building JackTrip Bundle...
candle.exe -arch x64 -ext WixBalExtension -dVersion=%VERSION% ..\jacktrip-bundle.wxs
light.exe -ext WixBalExtension -o JackTrip-Setup.exe jacktrip-bundle.wixobj

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to build bundle executable.
    exit /b 1
)

echo Build complete! JackTrip-Setup.exe contains the Visual C++ redistributable and JackTrip installer.
echo The installer will automatically install the Visual C++ redistributable if needed.
endlocal
