TGT_CXXFLAGS := -std=c++1z -O3 -Wfatal-errors -Werror
TGT_INCDIRS := ../../tio ../../cog ../../ung ../../jeh ../../seq ../../jeh ../../cdf .
TGT_LDFLAGS := -lrt -lnetcdf
TARGET := lick
TARGET_DIR := bin
SOURCES := lick.cpp utc.cpp
