check online documentation on
http://icwiki.physik.uni-bonn.de/twiki/bin/view/Systems/UsbPix

# Quick installation for Ubuntu 16

Get the code:
```
git clone -b release_5.3_eudaq20 https://github.com/beam-telescopes/USBpix.git
```
Set the environments and setup:
```
cd USBpix/
source set_environments_ubuntu16.sh 
source setup.sh -gbip no -siusbman no
```
Compile and install:
```
make 
make install
```
Start the application:
```
./bin/STcontrol_eudaq
```
