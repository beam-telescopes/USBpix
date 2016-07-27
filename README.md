This repository contains application software for the USBpix system based on the ATLAS PixLib package. This collection of C++ classes 
has originally been developed to provide access to the ATLAS Pixel detector RODs. The hardware specific layer of this package has been 
adapted to access the FE pixel chips via the USB/FPGA cards. The user interface to configure and operate the system is an adapted version 
of STcontrol which uses ROOT and Qt to implement a graphical user interface. 

The code available here was transferred from its original svn repo
http://icwiki.physik.uni-bonn.de/svn/USBpixI4/host/trunk
after having tagged the release 6.0, moving everything from Applications/Pixel to Applications and re-naming the USBpix* directories.
