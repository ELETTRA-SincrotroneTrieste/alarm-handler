NAME_SRV = alarm-srv



CXXFLAGS += `mysql_config --include`


LDFLAGS += `mysql_config --libs_r` -lboost_thread

include ./.makefiles/Make-9.2.2.in
