NAME_SRV = alarm-srv



CXXFLAGS += `mysql_config --include`


LDFLAGS += `mysql_config --libs_r` -lboost_thread

#include ./Make-8.1.2.c.in
include ./Make-9.2.2.in
