TGT_CXXFLAGS := -std=c++1z -O3 -Wfatal-errors -Werror
TGT_INCDIRS := ../../tio ../../cog ../../cdf ../../ung ../../jeh ../../seq ../../jeh .
TGT_LDFLAGS := -lrt -lboost_system -lnetcdf
TARGET := slurp
TARGET_DIR := bin
SOURCES := slurp.cpp utc.cpp 
