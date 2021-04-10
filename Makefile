ifndef $(BUILD)
	BUILD := release
endif

ifndef $(CONFIG)
	CONFIG := default
endif

UNAME_S := $(shell uname -s)

CURR_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BASE_BUILDDIR:=$(CURR_DIR)/build
BUILDDIR=$(BASE_BUILDDIR)/$(BUILD)

SOURCES = $(wildcard $(CURR_DIR)/*.cpp) \
		  $(wildcard $(CURR_DIR)/traffic/*.cpp) \
		  $(wildcard $(CURR_DIR)/topology/*.cpp) \
		  $(wildcard $(CURR_DIR)/utils/*.cpp) \
		  $(wildcard $(CURR_DIR)/simulation/*.cpp) \
		  $(wildcard $(CURR_DIR)/devtests/*.cpp)

ANALYSIS_SOURCES = $(wildcard $(CURR_DIR)/data-analysis/*.cpp)

OBJECTS = $(patsubst $(CURR_DIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.o))
DEPENDS = $(patsubst $(CURR_DIR)/%,$(BUILDDIR)/%,$(SOURCES:.cpp=.d))

ANALYSIS_OBJECTS = $(patsubst $(CURR_DIR)/%,$(BUILDDIR)/%,$(ANALYSIS_SOURCES:.cpp=.o))
ANALYSIS_DEPENDS = $(patsubst $(CURR_DIR)/%,$(BUILDDIR)/%,$(ANALYSIS_SOURCES:.cpp=.d))

RELEASE_BINARY := syndb-sim
DEBUG_BINARY := syndb-sim-debug
PROFILE_BINARY := syndb-sim-prof

ANALYSIS_RELEASE_BINARY := syndb-analysis
ANALYSIS_DEBUG_BINARY := syndb-analysis-debug
ANALYSIS_PROFILE_BINARY := syndb-analysis-profile


CXXFLAGS  = -I$(CURR_DIR) \
			-std=c++11

LDLIBS = -lpthread \
		 -lfmt \
		 -lspdlog

ifeq ($(BUILD), release)
CXXFLAGS += -O3
OUTPUT_BINARY := $(RELEASE_BINARY)
ANALYSIS_BINARY := $(ANALYSIS_RELEASE_BINARY)
endif

ifeq ($(BUILD), debug)
CXXFLAGS += -O0 -g3 -DDEBUG
OUTPUT_BINARY := $(DEBUG_BINARY)
ANALYSIS_BINARY := $(ANALYSIS_DEBUG_BINARY)
endif

ifeq ($(CONFIG), default)
CXXFLAGS += -DCONFIG=0
endif

ifeq ($(CONFIG), validation)
CXXFLAGS += -DCONFIG=1
endif

ifeq ($(CONFIG), evaluation)
CXXFLAGS += -DCONFIG=2
endif

ifeq ($(CONFIG), profiling)
CXXFLAGS += -pg -DCONFIG=3
OUTPUT_BINARY := $(PROFILE_BINARY)
ANALYSIS_BINARY := $(ANALYSIS_PROFILE_BINARY)
endif

ifeq ($(CONFIG), test)
CXXFLAGS += -DCONFIG=4
endif

.PHONY: all clean cleaner

all: $(OUTPUT_BINARY)

analysis: $(ANALYSIS_BINARY)

clean:
	@$(RM) -rf $(BUILDDIR)
	@$(RM) $(OUTPUT_BINARY)
	@$(RM) $(ANALYSIS_BINARY)

cleaner:
	@$(RM) -rf $(BASE_BUILDDIR)
	@$(RM) $(RELEASE_BINARY) $(DEBUG_BINARY) $(PROFILE_BINARY)
	@$(RM) $(ANALYSIS_RELEASE_BINARY) $(ANALYSIS_DEBUG_BINARY) $(ANALYSIS_PROFILE_BINARY)


# Linking the executable from the object files
$(OUTPUT_BINARY): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

$(ANALYSIS_BINARY): $(ANALYSIS_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

-include $(DEPENDS)

# -MMD -MP are related to generating the .d depends file
$(BUILDDIR)/%.o: $(CURR_DIR)/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
