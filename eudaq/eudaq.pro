TEMPLATE = aux

unix{
  myeudaq.target = $(EUDAQ)/lib/libEUDAQ.so
  myeudaq.commands = cd $(EUDAQ)/build; cmake ..; make install
  myeudaq.CONFIG  += target_predeps no_link
}
win32{
}

QMAKE_EXTRA_TARGETS += myeudaq
unix{
  PRE_TARGETDEPS += $(EUDAQ)/lib/libEUDAQ.so
}
win32{
}
