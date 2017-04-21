#!/bin/sh
#
cfgfile=$1
shift
./ModuleGroup_Wizard -n BaseConf.cfg
mv BaseConf.cfg.root $cfgfile
./ModuleGroup_Wizard -a $cfgfile "$cfgfile:/Test_0/application;1" Grp0_0 $@
