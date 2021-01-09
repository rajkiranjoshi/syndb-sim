# MKFILE_PATH = $(abspath $(lastword $(MAKEFILE_LIST)))
# CURR_DIR = $(notdir $(patsubst %/,%,$(dir $(MKFILE_PATH))))

CURR_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

SOURCES = $(CURR_DIR)/syndb-sim.cpp \
		  $(wildcard $(CURR_DIR)/traffic/*.cpp) \
		  $(wildcard $(CURR_DIR)/topology/*.cpp) \
		  $(wildcard $(CURR_DIR)/utils/*.cpp)

OUTPUT_BINARY = syndb-sim

CC = gcc
CFLAGS  = -O3 \
		  -I$(CURR_DIR)


all: $(OUTPUT_BINARY)

$(OUTPUT_BINARY): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(OUTPUT_BINARY)

clean:
	rm $(OUTPUT_BINARY)
