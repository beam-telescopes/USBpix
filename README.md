check online documentation on
http://icwiki.physik.uni-bonn.de/twiki/bin/view/Systems/UsbPix

# Quick installation for Ubuntu 16/18

Prequisities: 
```
sudo apt install libusb-1.0-0-dev
```

ROOT dependencies: Successfully build with v6.10.02; v6.14 and v6.16 not working, since seg fault with:
```
Error in <UnknownClass::InitInterpreter()>: LLVM SYMBOLS ARE EXPOSED TO CLING! This will cause problems; please hide them or dlopen() them after the call to TROOT::InitInterpreter()!
```

Get the code:
```
git clone https://github.com/beam-telescopes/USBpix.git PATH
cd PATH
git checkout release_5.3_eudaq20
```
Set the environments and setup:
```
source set_environments_ubuntu16.sh (source set_environments_ubuntu18.sh) 
source setup.sh -gpib no -siusbman no
```
Compile and install:
```
make 
make install
```
Start the application:
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/PATH/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/eudaq2/lib
./bin/STcontrol_eudaq
```
