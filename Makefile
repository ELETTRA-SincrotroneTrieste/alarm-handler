NAME_SRV = alarmhandler-srv



CXXFLAGS += `mysql_config --include` -D_USE_ELETTRA_DB_RW


LDFLAGS += `mysql_config --libs_r` -lboost_thread

include ./.makefiles/Make-9.3.3.in
