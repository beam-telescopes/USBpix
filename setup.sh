#!/bin/sh
#

BUILD_CONFIG_DIR=build-config

usbpix_print_help()
{
  echo "usage: source setup.sh [-h] [-tdaq <yes|no>] [-eudaq <auto|yes|no>] [-gpib <auto|yes|no>] [-spec <default|linux-g++|...>] [-buildtype <release|debug|full-debug>]"
  echo
  echo "help:  source setup.sh -h"
  echo
  echo "  -tdaq:"
  echo "    yes:         Build with TDAQ support"
  echo "  * no:          Build without TDAQ support"
  echo
  echo "  -eudaq:"
  echo "  * auto:        Build with EUDAQ support if \$EUDAQ is set."
  echo "    yes:         Build with EUDAQ support"
  echo "    no:          Build without EUDAQ support"
  echo
  echo "  -siusbman:"
  echo "  * auto:        Automatically detect if siusbman can be built"
  echo "    yes:         Build with SiUSBman suppport"
  echo "    no:          Build without SiUSBman support"
  echo
  echo "  -gpib:"
  echo "  * auto:        Automatically determine whether to use GPIB or not. "
  echo "                 Uses NI if available, then checks for linux-gpib."
  echo "    yes:         Force build with NI GPIB support."
  echo "    no:          Force build without any GPIB support"
  echo
  echo "  -spec:"
  echo "  * default:     Use default default qmake spec."
  echo "    ...:         Check your specs directory for other qmake specs."
  echo "                 'linux-clang' should enable building with clang,"
  echo "                 'linux-g++' with gcc."
  echo
  echo "  -buildtype:"
  echo "    release:     Remove all debugging symbols, use many optimization options."
  echo "  * debug:       Use the standard qmake debug settings."
  echo "    debug-full:  Use the standard qmake debug settings and some additional ones."
  echo
  echo "  -nq:"
  echo "  <no options>   Do not run qmake after running setup.sh. Thus, updates to .pro"
  echo "                 files are not consistently used and any environment change may"
  echo "                 not be reflected. Use this with caution."
  echo
  echo "  -restore:"
  echo "  <no options>   Use the same options you used last time (not recommended on linux)."
  echo
  echo " * = default "
}

export DAQ_BASE="$(pwd)"

origparams="$*"
#
# default: no TDAQ, no EUDAQ, GPIB identified automatically
eudaq=auto
tdaq=no
gpib=auto
spec=default
buildtype=debug
siusbman=auto
run_qmake=1
in_restore=0
#
# user options: -tdaq <yes|no> -eudaq <yes|no> -gpib <auto|yes|no>
while echo $1 | grep '^-' > /dev/null; do
    case $1 in
        "-nq")
            if [[ "$in_restore" == "0" ]]
            then
                run_qmake=0
            fi
            shift
        ;;
        "-restore")
            if [[ "$in_restore" == "0" ]]
            then
                set -- $(< ${BUILD_CONFIG_DIR}/sush_cache.txt)
                in_restore=1
            else
                echo "Warning - recursive restore detected."
            fi
        ;;
        "-h"|"-help"|"--h"|"--help"|"-?"|"/?"|"//?")
            usbpix_print_help
            return
        ;;
        "-eudaq"|"-tdaq"|"-siusbman"|"-gpib"|"-spec"|"-buildtype")
            eval $( echo $1 | sed 's/-//g' | tr -d '\012')=$2
            echo "Setting $1 := $2"
            shift
            shift
        ;;
        *)
            echo "Unkown option $1"
            usbpix_print_help
            return
        ;;
    esac
done

if [[ "$in_restore" == "0" ]]
then
    echo "$origparams" > ${BUILD_CONFIG_DIR}/sush_cache.txt
fi

echo "Using: setup.sh -tdaq $tdaq -eudaq $eudaq -gpib $gpib -spec $spec -buildtype $buildtype"
#
export USBCMN=${DAQ_BASE}
qtdtmp=${QT5DIR}
rstmp=${ROOTSYS}
export SCTPIXEL_DAQ_ROOT=${DAQ_BASE}/RodDaq
export VME_INTERFACE=${DAQ_BASE}/VmeInterface
export ROD_DAQ=${DAQ_BASE}/RodDaq
export PIX_TB_IOM=${DAQ_BASE}/Applications/Pixel/pixTbIOM
export PIX_LIB=${DAQ_BASE}/Applications/Pixel/PixLib
export ROD_TYPE=PIXEL_ROD
export PIXEL_ROD=true
export CAN_CALIB_META=${DAQ_BASE}/Applications/Pixel/PixCalibDbCoral
export CAN=${DAQ_BASE}/Applications/Pixel/CalibrationAnalysis
export CALIB_CONSOLE=${DAQ_BASE}/Applications/Pixel/CalibrationConsole
export MODULE_ANALYSIS=${DAQ_BASE}/Applications/Pixel/ModuleAnalysis
export STC=${DAQ_BASE}/Applications/Pixel/STcontrol
export PIX_ANA=${DAQ_BASE}/Applications/Pixel/PixAnalysis
export DBEDT=${DAQ_BASE}/Applications/Pixel/DBeditor
export DATA_VIEWER=${DAQ_BASE}/Applications/Pixel/DataViewer

export PIX_RS232="${DAQ_BASE}/PixRS232"
export PIX_SMTP="${DAQ_BASE}/SmtpClient-for-Qt"
export USB_INCL_DIR="${USBCMN}/inc"
export USB_LIB_DIR="${USBCMN}/lib"
export USB_DRIVER_LIB="${USBCMN}/libsiusb"
export USBPIXI3DLL="${DAQ_BASE}/USBpixI3dll"
export USBPIXI4DLL="${DAQ_BASE}/USBpixI4dll"

# libusb is now always used
export LIBUSB_FLAG=USE_LIBUSB

# TDAQ related settings
export DAQ_LIB_DIR=.
export DAQ_INCL_DIR=.
export DAQ_SW_DIR=.
export BOOSTINC=.
export BOOSTLIB=.
export TDAQ_FLAG=-DNOTDAQ
if [ "$tdaq" = "yes" ]; then
  export TDAQ_INST=/daq/slc6
  if [ ! -d "${TDAQ_INST}" -a -d /afs/cern.ch/atlas/project/tdaq/inst ]; then 
  # use the following instead of /daq/slc6 if no local TDAQ installation present; 
    echo "no local TDAQ found, will use afs; beware, compiling will be slow!"
    export TDAQ_INST=/afs/cern.ch/atlas/project/tdaq/inst
  fi
  # if TDAQ was found, proceed
  if [ -d "${TDAQ_INST}" ]; then 
    if [ "`uname -p`" = "x86_64" ] ; then
      export GCCCONFIG="x86_64"
      export CMTCONFIG=x86_64-slc6-gcc47-opt
    else
      export GCCCONFIG="i686"
      export CMTCONFIG=i686-slc6-gcc47-opt
    fi
    export GENCCFLAG="-mcmodel=large"
    export ARCH=${CMTCONFIG}
    if [ -d "${TDAQ_INST}"/tdaq/tdaq-05-04-00/installed/"${CMTCONFIG}" ]; then 
	export LCG_INST=${TDAQ_INST}/sw/lcg
	source ${LCG_INST}/contrib/gcc/4.7.2/${GCCCONFIG}-slc6-gcc47-opt/setup.sh
	export CMTROOT=${TDAQ_INST}/CMT/v1r25/
	export CMTPATH=${TDAQ_INST}/tdaq/tdaq-05-04-00:${TDAQ_INST}/tdaq-common/tdaq-common-01-28-00:${TDAQ_INST}/dqm-common/dqm-common-00-26-02:${TDAQ_INST}/LCGCMT/LCGCMT_67b
	source ${CMTROOT}/mgr/setup.sh
	source ${TDAQ_INST}/tdaq/tdaq-05-04-00/installed/setup.sh
	export TDAQ_BOOST=${LCG_INST}/external/Boost/1.53.0_python2.7
	export BOOSTVER=1_53
	export BOOST=${TDAQ_BOOST}/${ARCH}
	export BOOSTINC=${BOOST}/include/boost-${BOOSTVER}
	export BOOSTLIB=${BOOST}/lib
	export CMTPATH=${CMTPATH}:${DAQ_BASE}/Applications/Pixel
	export DAQ_LIB_DIR=${TDAQ_INST_PATH}/${CMTCONFIG}/lib
	export DAQ_INCL_DIR=${TDAQ_INST_PATH}/include
	export DAQ_SW_DIR=${TDAQ_INST_PATH}
	export TDAQ_FLAG=""
    else
	echo "TDAQ was requested, but installation wasn't found in "${TDAQ_INST}/tdaq/tdaq-05-04-00/installed/${CMTCONFIG}
    fi
  else
    echo "TDAQ was requested, but neither local TDAQ under /daq/slc6 nor afs-version could be accesssed"
  fi
fi # "$tdaq"=="yes"

# restore QT and ROOT since TDAQ scripts overwrite these
export QT5DIR=$qtdtmp
export ROOTSYS=$rstmp

# export CMTPATH=${CMTPATH}:${DAQ_BASE}/Applications/Pixel
if [ \( -n "$EUDAQ" -a "$eudaq" = "auto" \) -o "$eudaq" = "yes" ] ; then
  export EUDAQ_FLAG=WITHEUDAQ
  echo "using EUDAQ"
else
  export EUDAQ_FLAG=""
fi
#
# check if NI GPIB driver is running and compile with NI-GPIB if so
export nidrv=`ps aux | grep nipalps | grep -v grep | wc -l`
# alternatively, consider linux-gpib
if [ "$gpib" = "no" ]
then 
  export GPIB_FLAG=""
  export USE_GPIB_LINUX=""
  export PIX_GPIB=""
  echo "GPIB is disabled"
else
  if [ \( $nidrv -eq 1 -a "$gpib" = "auto" \) -o "$gpib" = "yes" ] 
  then
    export GPIB_FLAG="-DHAVE_GPIB"
    export USE_GPIB_LINUX=""
    export PIX_GPIB="${DAQ_BASE}/PixGPIB"
    echo "GPIB is enabled and using NI drivers"
  elif which gpib_config >/dev/null 2>&1
  then
    export GPIB_FLAG="-DHAVE_GPIB"
    export USE_GPIB_LINUX="yes"
    export PIX_GPIB="${DAQ_BASE}/PixGPIB"
    echo "GPIB is enabled and using linux-gpib"
  else
    export GPIB_FLAG=""
    export USE_GPIB_LINUX=""
    export PIX_GPIB=""
    echo "GPIB is disabled"
  fi
fi

if [[ "$ROOTSYS" != "" ]]
then
  export LD_LIBRARY_PATH="${ROOTSYS}/lib:${LD_LIBRARY_PATH}"
  export PATH="${ROOTSYS}/bin:${PATH}"
fi

if [[ "$QT5DIR" != "" ]]
then
  export LD_LIBRARY_PATH="${QT5DIR}/lib:${LD_LIBRARY_PATH}"
  export PATH="${QT5DIR}/bin:${PATH}"
fi

if [[ "$QWTDIR" != "" ]]
then
  export LD_LIBRARY_PATH="${QWTDIR}/lib:${LD_LIBRARY_PATH}"
fi

export PATH="${DAQ_BASE}/bin:${PATH}"

if [[ "$spec" != "default" ]]
then
  export QMAKESPEC="$spec"
else
  export QMAKESPEC=
fi

if ! which uic >/dev/null 2>&1
then
  echo "Could not find uic binary. It seems that Qt is not installed."
else
  if uic -version 2>&1 | grep "User Interface Compiler for Qt version 3" 2>/dev/null >/dev/null
  then
    which tput 2>/dev/null >/dev/null && tput smso 2>/dev/null
    echo "It seems you are trying to compile with QT3, as uic -version outputs:"
    uic -version
    echo "Please install and select QT >= 5 before continuing."
    which tput 2>/dev/null >/dev/null && tput rmso 2>/dev/null
    return
  fi
  QT_VERSION=$(uic -v 2>&1 | awk '{
    for(i = 0; i <= NF; i++) 
    { 
      if ($i ~ /^[345]\.[0-9](\.[0-9])*$/)
      {
        print $i
        break
      }
    } 
  }')
  QT_VERSION_MAJOR=$(echo $QT_VERSION | awk -F '.' '{print $1}')
  QT_VERSION_MINOR=$(echo $QT_VERSION | awk -F '.' '{print $2}')

  QT_COMPAT=unknown
  case "$QT_VERSION_MAJOR"
  in
    4)
      QT_COMPAT=obsolete
      if [[ "$QT_VERSION_MINOR" -lt 6 ]]
      then
        QT_COMPAT=no
      fi
      ;;
    5)
      QT_COMPAT=yes
      if [[ "$QT_VERSION_MINOR" -gt 2 ]]
      then
        QT_COMPAT=unknown
      fi
      ;;
  esac
  
  which tput 2>/dev/null >/dev/null && tput smso 2>/dev/null
  case "${QT_COMPAT}" in
    "unknown")
      echo "Warning: QT Version $QT_VERSION has not been tested. Use at your own luck."
      ;;
    "testing")
      echo "Warning: QT Version $QT_VERSION is still being evaluated and may not be fully supported. Use at your own luck."
      ;;
    "obsolete")
      echo "Warning: QT Version $QT_VERSION won't be supported any longer. Use at your own luck."
      ;;
    "no")
      echo "Warning: QT Version $QT_VERSION is not supported. This build will probably fail."
      ;;
  esac
  which tput 2>/dev/null >/dev/null && tput rmso 2>/dev/null
fi

if ! which root-config 2>/dev/null >/dev/null
then
  which tput 2>/dev/null >/dev/null && tput smso 2>/dev/null
  echo "Could not find root-config, which means that ROOTSYS is set incorrectly or root is not installed or not already in your PATH."
  which tput 2>/dev/null >/dev/null && tput rmso 2>/dev/null
  return
fi

echo "ROOTSYS      = ${ROOTSYS}"
echo "QT5DIR       = ${QT5DIR}"
echo "USB_INCL_DIR = ${USB_INCL_DIR}"
echo "USB_LIB_DIR  = ${USB_LIB_DIR}"
echo "USBPIXI3DLL  = ${USBPIXI3DLL}"
echo "USBPIXI4DLL  = ${USBPIXI4DLL}"

(
  echo "# Created by $0 on `date`. Modify using $0 if possible."
  echo "DAQ_BASE       = \"${DAQ_BASE}\"" 
  echo "EUDAQ_FLAG     = ${EUDAQ_FLAG}"
  echo "GPIB_FLAG      = ${GPIB_FLAG}"
  echo "USE_GPIB_LINUX = ${USE_GPIB_LINUX}"
  echo "BUILDTYPE      = ${buildtype}"
  echo "SIUSBMAN       = ${siusbman}"
  echo "LIBUSB_FLAG    = ${LIBUSB_FLAG}"
) > "${BUILD_CONFIG_DIR}/setup.inc"

if [[ "$run_qmake" == "1" ]]
then
  rm Makefile
  qmake -r
else
  echo "Skipping qmake run. Updates on .pro files and switches to this script may be ignored, but your environment is up to date."
fi
