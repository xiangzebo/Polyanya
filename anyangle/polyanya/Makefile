MAKEFLAGS += -r
PA_FOLDERS = structs helpers search
PA_SRC = $(foreach folder,$(PA_FOLDERS),$(wildcard $(folder)/*.cpp))
PA_OBJ = $(PA_SRC:.cpp=.o)
PA_INCLUDES = $(addprefix -I,$(PA_FOLDERS))

CXX = g++
CXXFLAGS = -std=c++11 -pedantic -Wall -Wno-strict-aliasing -Wno-long-long -Wno-deprecated -Wno-deprecated-declarations -Werror
FAST_CXXFLAGS = -O3 -DNDEBUG
DEV_CXXFLAGS = -g -ggdb -O0 -fno-omit-frame-pointer
PROFILE_CXXFLAGS = -g -ggdb -O0 -fno-omit-frame-pointer -DNDEBUG

ifeq ("$(findstring Darwin, "$(shell uname -s)")", "Darwin")
  CXXFLAGS += -DOS_MAC
else
  ifeq ("$(findstring Linux, "$(shell uname -s)")", "Linux")
    CXXFLAGS += -lrt
  endif
endif

TARGETS = test scenariorunner
BIN_TARGETS = $(addprefix bin/,$(TARGETS))

all: $(TARGETS)
fast: CXXFLAGS += $(FAST_CXXFLAGS)
dev: CXXFLAGS += $(DEV_CXXFLAGS)
prof: CXXFLAGS += $(PROFILE_CXXFLAGS)
fast dev prof: all

clean:
	rm -rf ./bin/*
	rm -f $(PA_OBJ:.o=.d)
	rm -f $(PA_OBJ)

.PHONY: $(TARGETS)
$(TARGETS): % : bin/%

$(BIN_TARGETS): bin/%: %.cpp $(PA_OBJ)
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) $(PA_INCLUDES) $(PA_OBJ) $(@:bin/%=%).cpp -o $(@)

-include $(PA_OBJ:.o=.d)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(PA_INCLUDES) -MM -MP -MT $@ -MF ${@:.o=.d} $<
	$(CXX) $(CXXFLAGS) $(PA_INCLUDES) $< -c -o $@
