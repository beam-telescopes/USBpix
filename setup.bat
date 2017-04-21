@echo off

IF /I "%~1"=="-restore" (
	echo Restoring last setup.bat call.
	build-config\su_cache.bat -skip %*
)

rem save this call, so it can be reused:
echo setup.bat %* %%* > build-config\su_cache.bat

rem Switch default values:
set useeudaq=no
set siusbman=auto
set buildtype=release
set stcontrol_console=no
set libusb=no
set buildflags=QT5_FIX_QDIALOG

rem automatically set Windows 8 to default of libusbs=yes until Silab driver works
systeminfo | find "OS Name" > %TEMP%\osname.txt
FOR /F "usebackq delims=: tokens=2" %%i IN (%TEMP%\osname.txt) DO set vers=%%i
echo %vers% | find "Windows 7" > nul
if %ERRORLEVEL% == 0 goto ver_7
echo %vers% | find "Windows 8" > nul
if %ERRORLEVEL% == 0 goto ver_8
goto ver_end
:ver_7
set WINVER=win7
set libusb=no
goto ver_end
:ver_8
set WINVER=win8
set libusb=yes
:ver_end

rem Loop through all arguments, detect known switches and set the corresponding
rem variables:
:paramloop
IF "%~1"=="" ( 
	GOTO cont
)ELSE IF /I "%~1"=="-useeudaq" (
	SET useeudaq=%~2
	SHIFT
)ELSE IF /I "%~1"=="-buildtype" (
	SET buildtype=%~2
	SHIFT
)ELSE IF /I "%~1"=="-siusbman" (
	SET siusbman=%~2
	SHIFT
)ELSE IF /I "%~1"=="-stcontrol_console" (
	SET stcontrol_console=%~2
	SHIFT
)ELSE IF /I "%~1"=="-libusb" (
	SET libusb=%~2
	SHIFT
)ELSE IF /I "%~1"=="-buildflag" (
	SET buildflags=%buildflags% %~2
	SHIFT
)ELSE IF /I "%~1"=="-skip" (
	SHIFT
)ELSE IF /I "%~1"=="-h" (
	GOTO showhelp 
)ELSE (
	goto paramerror
)
SHIFT & GOTO paramloop
goto cont
:paramerror
echo Unknown parameter %~1
:showhelp
echo Usage: 
echo setup.bat -restore
echo or
echo setup.bat [-useeudaq yes/no] [-buildtype release/debug] [-siusbman yes/no/auto] [-stcontrol_console yes/no] [-libusb yes/no]
goto eof
:cont 

echo Using: setup.bat -useeudaq %useeudaq% -buildtype %buildtype% -siusbman %siusbman% -stcontrol_console %stcontrol_console% -libusb %libusb%
echo Using flags: %buildflags%

set DAQ_BASE=%cd%
set USBCMN=%DAQ_BASE%
set SCTPIXEL_DAQ_ROOT=%DAQ_BASE%\RodDaq
set VME_INTERFACE=%DAQ_BASE%\VmeInterface
set ROD_DAQ=%DAQ_BASE%\RodDaq
set PIX_TB_IOM=%DAQ_BASE%\Applications\Pixel\pixTbIOM
set PIX_LIB=%DAQ_BASE%\Applications\Pixel\PixLib
set ROD_TYPE=PIXEL_ROD
set PIXEL_ROD=true
set CAN_CALIB_META=%DAQ_BASE%\Applications\Pixel\PixCalibDbCoral
set CAN=%DAQ_BASE%\Applications\Pixel\CalibrationAnalysis
set CALIB_CONSOLE=%DAQ_BASE%\Applications\Pixel\CalibrationConsole
set MODULE_ANALYSIS=%DAQ_BASE%\Applications\Pixel\ModuleAnalysis
set STC=%DAQ_BASE%\Applications\Pixel\STcontrol
set PIX_ANA=%DAQ_BASE%\Applications\Pixel\PixAnalysis
set DBEDT=%DAQ_BASE%\Applications\Pixel\DBeditor
set DATA_VIEWER=%DAQ_BASE%\Applications\Pixel\DataViewer
rem the following are just dummies to keep the Makefiles happy
set DAQ_INCL_DIR=.
set DAQ_LIB_DIR=.
set TDAQ_FLAG=-DNOTDAQ
set USB_INCL_DIR=%USBCMN%\inc
set USB_LIB_DIR=%USBCMN%\lib
set USBPIXI3DLL=%DAQ_BASE%\USBPixI3dll
set USBPIXI4DLL=%DAQ_BASE%\USBPixI4dll
set PIX_GPIB=%DAQ_BASE%\PixGPIB
set GPIBDIR=%PIX_GPIB%
set GPIB_FLAG=-DHAVE_GPIB
set PIX_RS232=%DAQ_BASE%\PixRS232
set PIX_SMTP=%DAQ_BASE%\SmtpClient-for-Qt
set LIBUSB_FLAG=
if %libusb% == yes goto :libusb
goto :nolibusb
:libusb
set LIBUSB_FLAG=USE_LIBUSB
:nolibusb
set EUDAQ_FLAG=
set EUDAQ=
if "%useeudaq%" == "yes" goto eudaq
goto noeudaq
:eudaq
set EUDAQ_FLAG=WITHEUDAQ
set EUDAQ=%DAQ_BASE%\eudaq
:noeudaq
set Path=%SystemRoot%\system32;%SystemRoot%;%ROOTSYS%\bin;%QT5DIR%\lib;%QT5DIR%\bin;%QWTDIR%\lib;%VCDIR%\bin;%DAQ_BASE%\bin
echo DAQ_BASE=%DAQ_BASE%
echo USBCMN=%USBCMN%
echo ROOTSYS=%ROOTSYS%
echo QT5DIR=%QT5DIR%
echo QTDIR=%QTDIR%  (not used)
echo VCDIR=%VCDIR%
echo USBPIXI3DLL=%USBPIXI3DLL%
echo USBPIXI4DLL=%USBPIXI4DLL%
echo PATH=%PATH%

call "%VCDIR%\vcvarsall.bat" x86

copy /y PixGPIB\ni488.h.windows PixGPIB\ni488.h

set BUILD_CONFIG_DIR=build-config
(
  echo # Created by setup.bat. Modify using setup.bat if possible.
  echo DAQ_BASE       = %DAQ_BASE%
  echo EUDAQ_FLAG     = %EUDAQ_FLAG%
  echo GPIB_FLAG      = %GPIB_FLAG%
  echo USE_GPIB_LINUX = %USE_GPIB_LINUX%
  echo BUILDTYPE      = %buildtype%
  echo BUILDFLAGS     = %buildflags%
  echo SIUSBMAN       = %siusbman%
  echo STCONTROL_CONSOLE = %stcontrol_console%
  echo LIBUSB_FLAG    = %LIBUSB_FLAG%
) > %BUILD_CONFIG_DIR%/setup.inc

qmake -r

echo Makefiles have been generated.
echo - If you just changed setup.bat parameters, consider calling
echo                         nmake distclean
echo   and then setup.bat again to make sure that all old binaries
echo   are removed.
echo - If this is your first build, just go ahead and call
echo                          nmake
echo   to build the complete project.
echo - The setup.bat parameters have been stored in build-config/su_cache.bat.
echo   Use setup.bat -restore to automatically call setup.bat with the previous
echo   parameter set.

:eof
