MAKEFLAGS += -r
PU_FOLDERS = fadeutils
PU_SRC = $(foreach folder,$(PU_FOLDERS),$(wildcard $(folder)/*.cpp))
PU_OBJ = $(PU_SRC:.cpp=.o)
PU_INCLUDES = $(addprefix -I,$(PU_FOLDERS))

CXX = g++
CXXFLAGS = -std=c++11 -pedantic -Wall -Wno-strict-aliasing -Wno-long-long -Wno-deprecated -Wno-deprecated-declarations -Werror
FAST_CXXFLAGS = -O3 -DNDEBUG -Wno-error
DEV_CXXFLAGS = -g -ggdb -O0
FADE2DFLAGS = -Ifade2d -Llib/ubuntu16.10_x86_64 -lfade2d -Wl,-rpath=lib/ubuntu16.10_x86_64

TARGETS = visualiser poly2mesh
BIN_TARGETS = $(addprefix bin/,$(TARGETS))

all: $(TARGETS) nofade
nofade: gridmap2poly meshpacker meshunpacker meshmerger gridmap2rects
fast: CXXFLAGS += $(FAST_CXXFLAGS)
dev: CXXFLAGS += $(DEV_CXXFLAGS)
fast dev: all

clean:
	rm -rf ./bin/*
	rm -f $(PU_OBJ:.o=.d)
	rm -f $(PU_OBJ)

.PHONY: $(TARGETS) gridmap2poly
$(TARGETS) gridmap2poly meshpacker meshunpacker meshmerger gridmap2rects gridmap2grid: % : bin/%

$(BIN_TARGETS): bin/%: %.cpp $(PU_OBJ)
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) $(PU_INCLUDES) $(PU_OBJ) $(@:bin/%=%).cpp -o $(@) $(FADE2DFLAGS)

bin/gridmap2poly: gridmap2poly.cpp
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) gridmap2poly.cpp -o ./bin/gridmap2poly

bin/meshpacker: meshpacker.cpp
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) -O3 meshpacker.cpp -o ./bin/meshpacker

bin/meshunpacker: meshunpacker.cpp
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) -O3 meshunpacker.cpp -o ./bin/meshunpacker

bin/meshmerger: meshmerger.cpp
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) -O3 meshmerger.cpp -o ./bin/meshmerger

bin/gridmap2rects: gridmap2rects.cpp
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) -O3 gridmap2rects.cpp -o ./bin/gridmap2rects

bin/gridmap2grid: gridmap2grid.cpp
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) -O3 gridmap2grid.cpp -o ./bin/gridmap2grid

bin/gridmap2grid: gridmap2grid.cpp
	@mkdir -p ./bin
	$(CXX) $(CXXFLAGS) -O3 gridmap2grid.cpp -o ./bin/gridmap2grid

-include $(PU_OBJ:.o=.d)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(FADE2DFLAGS) $(INCLUDES) -MM -MP -MT $@ -MF ${@:.o=.d} $<
	$(CXX) $(CXXFLAGS) $(FADE2DFLAGS) $(INCLUDES) $< -c -o $@
