@echo off
setlocal EnableDelayedExpansion

set QTVERSION="5"

if not defined QTBINPATH (
	for /f "delims=" %%a in ('dir /b C:\Qt\%QTVERSION%.*') do set QTFULLVERSION=%%a
	for /f "delims=" %%a in ('dir /b C:\Qt\!QTFULLVERSION!\mingw*') do set QTBINPATH=%%a
	set QTBINPATH=C:\Qt\!QTFULLVERSION!\!QTBINPATH!\bin
)
echo Using Qt Version %QTFULLVERSION%
if not defined QTLIBPATH (
	for /f "delims=" %%a in ('dir /b C:\Qt\Tools\mingw*') do set QTLIBPATH=%%a
	set QTLIBPATH=C:\Qt\Tools\!QTLIBPATH!\bin
)
echo Using mingw libraries from %QTLIBPATH%
if not defined WIXPATH (
	for /f "delims=" %%a in ('dir /b "C:\Program Files (x86)\Wix Toolset*"') do set WIXPATH=%%a
	set WIXPATH=C:\Program Files ^(x86^)\!WIXPATH!\bin
)
if not defined SSLPATH (set SSLPATH=C:\Qt\Tools\OpenSSL\Win_x64\bin)
if not defined RTAUDIOLIB (set RTAUDIOLIB="C:\Program Files (x86)\RtAudio\bin\librtaudio.dll")

set PATH=%PATH%;%WIXPATH%
del deploy /s /q
rmdir deploy /s /q
mkdir deploy

copy ..\LICENSE.md deploy\
xcopy ..\LICENSES deploy\LICENSES\

REM create RTF file with licenses' text
set LICENSEPATH=deploy\license.rtf
echo {\rtf1\ansi\deff0 {\fonttbl {\f0 Calibri;}} \f0\fs22>%LICENSEPATH%
for %%f in (..\LICENSE.md ..\LICENSES\MIT.txt ..\LICENSES\GPL-3.0.txt ..\LICENSES\LGPL-3.0-only.txt) do (
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
for /f "tokens=*" %%a in ('%QTLIBPATH%\objdump -p jacktrip.exe ^| findstr Qt%QTVERSION%Core.dll') do set DYNAMIC_QT=%%a
if defined DYNAMIC_QT (
	echo Including Qt Files
	for /f "tokens=*" %%a in ('%QTLIBPATH%\objdump -p jacktrip.exe ^| findstr Qt%QTVERSION%Qml.dll') do set VS=%%a
	if defined VS (
		if %QTVERSION%=="6" (
			echo The installer is not designed to handle dynamic Virtual Studio builds using Qt6 yet.
			exit /b 1
		)
		%QTBINPATH%\windeployqt --qmldir ..\..\src\gui jacktrip.exe
		set WIXDEFINES=%WIXDEFINES% -dvs
	) else (
		%QTBINPATH%\windeployqt jacktrip.exe
	)
	copy "%QTLIBPATH%\libgcc_s_seh-1.dll" .\
	copy "%QTLIBPATH%\libstdc++-6.dll" .\
	copy "%QTLIBPATH%\libwinpthread-1.dll" .\
	copy "%SSLPATH%\libcrypto-1_1-x64.dll" .\
	copy "%SSLPATH%\libssl-1_1-x64.dll" .\
	set WIXDEFINES=!WIXDEFINES! -ddynamic
)
for /f "tokens=*" %%a in ('%QTLIBPATH%\objdump -p jacktrip.exe ^| findstr librtaudio.dll') do set RTAUDIO=%%a
if defined RTAUDIO (
	echo Including librtaudio
	copy %RTAUDIOLIB% .\
	set WIXDEFINES=%WIXDEFINES% -drtaudio
)
set WIXDEFINES=%WIXDEFINES% -dqt%QTVERSION%

copy ..\jacktrip.wxs .\
copy ..\files.wxs .\
.\jacktrip --test-gui
if %ERRORLEVEL% NEQ 0 (
	echo You need to build jacktrip with gui support to build the installer.
	exit /b 1
)
rem Get our version number
for /f "tokens=*" %%a in ('.\jacktrip -v ^| findstr VERSION') do for %%b in (%%~a) do set VERSION=%%b
for /f "tokens=1 delims=-" %%a in ("%VERSION%") do set VERSION=%%a
echo Version=%VERSION%
candle.exe -arch x64 -ext WixUIExtension -ext WixUtilExtension -dVersion=%VERSION%%WIXDEFINES% jacktrip.wxs files.wxs
light.exe -ext WixUIExtension -ext WixUtilExtension -o JackTrip.msi jacktrip.wixobj files.wixobj
endlocal
