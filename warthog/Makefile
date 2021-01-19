WARTHOG_SRC = $(wildcard domains/*.cpp) $(wildcard util/*.cpp) $(wildcard search/*.cpp) \
$(wildcard experimental/*.cpp) $(wildcard heuristics/*.cpp) $(wildcard jps/*.cpp) 

WARTHOG_OBJ = $(WARTHOG_SRC:.cpp=.o)

D_WARTHOG_INCLUDES = -I./domains -I./util -I./search -I./experimental -I./heuristics -I./jps
D_INCLUDES = -I/usr/include -I/opt/local/include $(D_WARTHOG_INCLUDES)
D_LIBS = -L/usr/local/lib -L./lib

CC = g++
CFLAGS = -std=c++0x -pedantic -Wall -Wno-strict-aliasing -Wno-long-long \
		 -Wno-deprecated -Wno-deprecated-declarations -Wno-unused-result -Werror
FAST_CFLAGS = -O3 -DNDEBUG
DEV_CFLAGS = -g -ggdb -O0
PROFILE_CFLAGS = -g -ggdb -O0 -DNDEBUG

ifeq ("$(findstring Darwin, "$(shell uname -s)")", "Darwin")
  CFLAGS += -DOS_MAC
else
  ifeq ("$(findstring Linux, "$(shell uname -s)")", "Linux")
    D_LIBS += -lrt
  endif
endif


.PHONY: all
all: dev

.PHONY: fast
fast: CFLAGS += $(FAST_CFLAGS) $(D_INCLUDES) 
fast: main

.PHONY: dev
dev: CFLAGS += $(DEV_CFLAGS) $(D_INCLUDES) 
dev: main

.PHONY: profile
profile: CFLAGS += $(PROFILE_CFLAGS) $(D_INCLUDES) 
profile: main

.PHONY: tags
tags:
	ctags -R .

.PHONY: clean
clean:
	@rm -rf ./bin/*
	@-$(RM) ./lib/*
	@-$(RM) $(WARTHOG_OBJ:.o=.d)
	@-$(RM) $(WARTHOG_OBJ)

.PHONY: main
main: makedirs warthog 
	@echo "linking..."
	$(CC) warthog.cpp -o ./bin/warthog -lwarthog $(CFLAGS) $(D_LIBS) $(D_INCLUDES)
	$(CC) tests.cpp -o ./bin/tests -lwarthog $(CFLAGS) $(D_LIBS) $(D_INCLUDES)

.PHONY: makedirs
makedirs:
	@echo "### Creating output directories ###"
	@$(shell mkdir ./bin)
	@$(shell mkdir ./lib)

.PHONY: warthog
warthog: $(WARTHOG_OBJ)
	@echo "###  Archiving object files ###"
	@echo "sources "$(WARTHOG_SRC)
	ar -crs lib/lib$(@).a $(WARTHOG_OBJ)

-include $(WARTHOG_OBJ:.o=.d)

$(WARTHOG_OBJ): 
	@echo "compiling..."
	$(CC) -c $(@:.o=.cpp) -MM -MP -MT $(dir $(@))$(notdir $(@)) -MF $(@:.o=.d) $(CFLAGS) 
	$(CC) -c $(@:.o=.cpp) -o $(@) $(CFLAGS)

