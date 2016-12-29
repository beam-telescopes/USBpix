TEMPLATE = aux

unix{
  myeudaq.target = $(EUDAQ)/lib/libEUDAQ.so
  myeudaq.commands = cd $(EUDAQ)/build; cmake ..; make install
  myeudaq.CONFIG  += target_predeps no_link
}
win32{
  myeudaq.target = $(EUDAQ)/lib/EUDAQ.lib
  myeudaq.commands = cd $(EUDAQ)\build & cmake .. & msbuild INSTALL.vcxproj /p:Configuration=Release & copy  $(EUDAQ)\bin\EUDAQ.dll $(DAQ_BASE)\bin\EUDAQ.dll
  myeudaq.CONFIG  += target_predeps no_link
}

QMAKE_EXTRA_TARGETS += myeudaq
unix{
  PRE_TARGETDEPS += $(EUDAQ)/lib/libEUDAQ.so
}
win32{
  PRE_TARGETDEPS += $(EUDAQ)/lib/EUDAQ.lib
}
