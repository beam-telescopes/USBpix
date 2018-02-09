# for version
# git checkout release_6.2_eudaq20

# for installation
export EUDAQ=/opt/beam-telescopes/eudaq
echo "Please check your EUDAQ path"
export QT5DIR=/usr/lib/x86_64-linux-gnu/qt5
export QWTDIR=/usr/lib/x86_64-linux-gnu

# setup USBpix installation
echo "cd USBpix && source setup.sh -gpib no && make -j4"

# for re-compiling
# make clean
# make distclean

# for running
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$EUDAQ/lib
# or copy eudaq lib 
echo "./USBpix/bin/STcontrol_eudaq"
