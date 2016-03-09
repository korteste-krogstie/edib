TGT_CXXFLAGS := -std=c++1z -O3 -Wfatal-errors -Werror
TGT_INCDIRS := ../../cog ../../ung ../../jeh ../../seq ../../jeh .
TGT_LDFLAGS := -lrt 
TARGET := cartodb
TARGET_DIR := bin
SOURCES := cartodb.cpp utc.cpp
