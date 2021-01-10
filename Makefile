# MKFILE_PATH = $(abspath $(lastword $(MAKEFILE_LIST)))
# CURR_DIR = $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))

CURR_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SOURCES = $(wildcard $(CURR_DIR)/*.cpp) \
		  $(wildcard $(CURR_DIR)/traffic/*.cpp) \
		  $(wildcard $(CURR_DIR)/topology/*.cpp) \
		  $(wildcard $(CURR_DIR)/utils/*.cpp)

OUTPUT_BINARY = syndb-sim

# CC = gcc
CXXFLAGS  = -O0 \
			-DBOOST_LOG_DYN_LINK \
			-I$(CURR_DIR) \
			-std=c++11

LDLIBS = -lboost_log \
		 -lpthread \
		 -lfmt


all: $(OUTPUT_BINARY)

$(OUTPUT_BINARY): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(OUTPUT_BINARY) $(LDLIBS)

clean:
	rm $(OUTPUT_BINARY)
