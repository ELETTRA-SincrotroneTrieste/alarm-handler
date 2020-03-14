NAME_SRV = alarm-handler-srv



CXXFLAGS +=  -D_USE_ELETTRA_DB_RW


LDFLAGS +=  -lboost_thread

include ./.makefiles/Make-9.3.3.in
