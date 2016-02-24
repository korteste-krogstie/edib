TGT_CXXFLAGS := -std=c++1z -O3 -Wfatal-errors -Werror
TGT_INCDIRS := ../../tio ../../cog ../../cdf ../../ung ../../jeh ../../seq ../../jeh ../../xl .
TGT_LDFLAGS := -lrt -lboost_system -lnetcdf -lX11 -lXext
TARGET := swallow
TARGET_DIR := bin
SOURCES := swallow.cpp utc.cpp 
