This repository contains application software for the USBpix system based on the ATLAS PixLib package. This collection of C++ classes 
has originally been developed to provide access to the ATLAS Pixel detector RODs. The hardware specific layer of this package has been 
adapted to access the FE pixel chips via the USB/FPGA cards. The user interface to configure and operate the system is an adapted version 
of STcontrol which uses ROOT and Qt to implement a graphical user interface. 

The code available here was transferred from its original svn repo
http://icwiki.physik.uni-bonn.de/svn/USBpixI4/host/trunk
after having tagged the release 6.0, moving everything from Applications/Pixel to Applications and re-naming the USBpix* directories.

More information on building the code and usage is avaialable here: http://icwiki.physik.uni-bonn.de/twiki/bin/view/Systems/UsbPix

To retrieve the code, run:

git clone https://[your CERN username]@gitlab.cern.ch/jgrosse/USBpix.git

which delivers the master branch (trunk). For another branch, go to the cloned version and use

git checkout [branch name]

# Updates for EUDAQ2

## Installation on Ubuntu 16.04

After setting the specific environment variables
```
setup_installation_ubuntu16.sh
```
(On fhlrcduranta in TB22 in /opt/configs/setup_ubuntu16_paths_for_STControl.sh)

run the USBpix setup.sh

## Development

* Reading out the trigger words in STeudaq_producer_I4.cxx
