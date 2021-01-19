VPATH = jump/:rsr/:hpa/:abstraction/:driver/:shared/:simulation/:util/:objs/:apps/libs:bin/

# source targets
ABSTRACTION_SRC = $(wildcard abstraction/*.cpp)
DRIVER_SRC = $(wildcard driver/*.cpp)
GLSTUB_SRC = $(wildcard driver/STUB/GL/*.cpp)
SHARED_SRC = $(wildcard shared/*.cpp)
SIMULATION_SRC = $(wildcard simulation/*.cpp)
UTIL_SRC = $(wildcard util/*.cpp)
EXTRAS_SRC = $(wildcard extras/*.cpp)
HPASTAR_SRC = $(wildcard hpa/*.cpp)
RSR_SRC = $(wildcard rsr/*.cpp)
JUMP_SRC = $(wildcard jump/*.cpp)
POLICIES_SRC = $(wildcard policies/*.cpp)
FILTERS_SRC = $(wildcard filters/*.cpp)
HEURISTICS_SRC = $(wildcard heuristics/*.cpp)

# object targets
DRIVER_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(DRIVER_SRC))))
GLSTUB_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(GLSTUB_SRC))))
ABSTRACTION_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(ABSTRACTION_SRC))))
SHARED_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(SHARED_SRC))))
SIMULATION_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(SIMULATION_SRC))))
UTIL_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(UTIL_SRC))))
EXTRAS_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(EXTRAS_SRC))))
HPASTAR_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(HPASTAR_SRC))))
RSR_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(RSR_SRC))))
JUMP_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(JUMP_SRC))))
POLICIES_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(POLICIES_SRC))))
FILTERS_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(FILTERS_SRC))))
HEURISTICS_OBJ = $(subst .cpp,.o,$(addprefix objs/, $(notdir $(HEURISTICS_SRC))))

HOGCORE_OBJ = $(UTIL_OBJ) $(SIMULATION_OBJ) $(ABSTRACTION_OBJ) \
		$(SHARED_OBJ) $(HPASTAR_OBJ) $(RSR_OBJ) $(JUMP_OBJ) $(POLICIES_OBJ) \
		$(FILTERS_OBJ) $(HEURISTICS_OBJ)

ifeq ("$(OPENGL)", "STUB") 
  HOGCORE_OBJ += $(GLSTUB_OBJ)
endif


# header file locations
HOGINCLUDES = -I./jump -I./hpa -I./rsr -I./abstraction -I./driver -I./shared -I./simulation -I./util \
			  -I./extras -I./policies -I./filters -I./heuristics

# compiler flags
CC = c++
FAST_CFLAGS = -O3 -ansi -DNDEBUG
DEV_CFLAGS = -Wall -Wno-long-long -Wno-deprecated -g -ggdb -ansi -pedantic

LIBFLAGS = -Lapps/libs -Llibs
STARTGROUPFLAG=-Wl,--start-group
ENDGROUPFLAG=-Wl,--end-group

# configure header and library paths for the current os (and opengl config)
ifeq ("$(findstring Darwin, "$(shell uname -s)")", "Darwin")
  SYS_CFLAGS = -DOS_MAC
  SYS_CFLAGS += -I/System/Library/Frameworks/Foundation.framework/Versions/A/Headers/
  SYS_CFLAGS += -I/System/Library/Frameworks/AppKit.framework/Versions/A/Headers/
  SYS_CFLAGS += -I/opt/local/include/ -I/usr/local/include/ 
  LIBFLAGS += -framework AppKit -framework Foundation
  STARTGROUPFLAG=
  ENDGROUPFLAG=

  ifeq ("$(OPENGL)", "STUB") 
    SYS_CFLAGS += -DNO_OPENGL
    SYS_CFLAGS += -I./driver/STUB/ -I./driver/STUB/GL/ 
  else
    SYS_CFLAGS += -I/System/Library/Frameworks/AGL.framework/Versions/A/Headers/
    SYS_CFLAGS += -I/System/Library/Frameworks/GLUT.framework/Versions/A/Headers/
    SYS_CFLAGS += -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/
    LIBFLAGS += -framework GLUT -framework OpenGL 
  endif

else # Cygwin/Windows
ifeq ("$(findstring CYGWIN, $(shell uname -s))", "CYGWIN")
  SYS_CFLAGS = -Dlinux
  LIBFLAGS += -L/lib/w32api

  ifeq ("$(OPENGL)", "STUB") 
    SYS_CFLAGS += -DNO_OPENGL
    SYS_CFLAGS += -I./driver/STUB/ -I./driver/STUB/GL/ 
  else
    SYS_CFLAGS += -I/usr/include/opengl
    LIBFLAGS += -lopengl32 -lglu32 -lglut32 
  endif

else # Linux et al 
 SYS_CFLAGS = -Dlinux 
 LIBFLAGS += -L/usr/X11R6/lib64 -L/usr/X11R6/lib -L/usr/lib -L$(HOME)/lib -L/opt/local/lib -L/usr/local/lib

  ifeq ("$(OPENGL)", "STUB") 
    SYS_CFLAGS += -DNO_OPENGL
    SYS_CFLAGS += -I./driver/STUB/ -I./driver/STUB/GL/ 
  else
    SYS_CFLAGS += -I/usr/include/GL 
    LIBFLAGS += -lGL -lGLU -lglut -lXi -lXmu 
  endif
endif
endif

ifeq ("$(CPU)", "G5")
 CFLAGS += -mcpu=970 -mpowerpc64 -mtune=970
 CFLAGS += -mpowerpc-gpopt -force_cpusubtype_ALL
endif

# every directory in ./apps, except those filtered out, is a target for compilation
TARGETS =  hog

all: fast
targets: $(TARGETS)

dev: CFLAGS = $(DEV_CFLAGS) $(SYS_CFLAGS) $(HOGINCLUDES) 
dev: APPSTARGET = dev
dev: $(TARGETS)

fast: CFLAGS = $(FAST_CFLAGS) $(SYS_CFLAGS) $(HOGINCLUDES)
fast: APPSTARGET = fast
fast: $(TARGETS)

# NB: splitting up HOG into multiple bits (like below) creates some circular
# dependency problems when linking on systems other than OSX.
# The options --start-group --end-group specify a dependency closure to fix this.
# The cost is that repeatedly searching the closure to resolve deps can be slow. 
.PHONY: hog
hog : hogcore driver 
	@echo "### Building target: "$(@)"  ###"
	cd apps; $(MAKE) -f hog.mk $(APPSTARGET) OPENGL=$(OPENGL); cd ..
	$(CC) -o $(addprefix bin/,$(@)) $(LIBFLAGS) $(STARTGROUPFLAG) -l$(@) -lhogcore -ldriver $(ENDGROUPFLAG)

.PHONY: driver
driver : $(DRIVER_OBJ) $(EXTRAS_OBJ)
	@echo "### Creating libdriver.a ###"
	@ar -crs libs/lib$(@).a $(DRIVER_OBJ) $(EXTRAS_OBJ)

.PHONY: hogcore
hogcore : $(HOGCORE_OBJ)
	@echo "### Creating libhogcore.a ###"
	@ar -crs libs/lib$(@).a $(HOGCORE_OBJ)

$(addprefix lib, $(addsuffix .a, $(TARGETS))) : 
	@echo "making app: "$(@)" ("$(APPSTARGET)")"
	cd apps; $(MAKE) -f $(patsubst lib%,%.mk,$(basename $(@))) OPENGL=$(OPENGL) ; cd ..
	#@cd apps; $(MAKE) $(APPSTARGET); $(MAKE) -f $(patsubst lib%,%.mk,$(basename $(@))) OPENGL=$(OPENGL) $(@); cd ..

$(UTIL_OBJ) : $(UTIL_SRC) $(UTIL_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,util/,$(@))) -o $(@) $(CFLAGS)

$(SIMULATION_OBJ) : $(SIMULATION_SRC) $(SIMULATION_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,simulation/,$(@))) -o $(@) $(CFLAGS)

$(ABSTRACTION_OBJ) : $(ABSTRACTION_SRC) $(ABSTRACTION_SRC.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,abstraction/,$(@))) -o $(@) $(CFLAGS)

$(SHARED_OBJ) : $(SHARED_SRC) $(SHARED_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,shared/,$(@))) -o $(@) $(CFLAGS)

$(DRIVER_OBJ) : $(DRIVER_SRC) $(DRIVER_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,driver/,$(@))) -o $(@) $(CFLAGS)

$(GLSTUB_OBJ) : $(GLSTUB_SRC) $(GLSTUB_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,driver/STUB/GL/,$(@))) -o $(@) $(CFLAGS)

$(HPASTAR_OBJ) : $(HPASTAR_SRC) $(HPASTAR_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,hpa/,$(@))) -o $(@) $(CFLAGS)

$(RSR_OBJ) : $(RSR_SRC) $(RSR_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,rsr/,$(@))) -o $(@) $(CFLAGS)

$(JUMP_OBJ) : $(JUMP_SRC) $(JUMP_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,jump/,$(@))) -o $(@) $(CFLAGS)

$(POLICIES_OBJ) : $(POLICIES_SRC) $(POLICIES_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,policies/,$(@))) -o $(@) $(CFLAGS)

$(HEURISTICS_OBJ) : $(HEURISTICS_SRC) $(HEURISTICS_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,heuristics/,$(@))) -o $(@) $(CFLAGS)

$(FILTERS_OBJ) : $(FILTERS_SRC) $(FILTERS_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,filters/,$(@))) -o $(@) $(CFLAGS)

$(EXTRAS_OBJ) : $(EXTRAS_SRC) $(EXTRAS_SRC:.cpp=.h)
	$(CC) -c $(subst .o,.cpp, $(subst objs/,extras/,$(@))) -o $(@) $(CFLAGS)

clean: cleanapps
	@echo cleaning ./
	@-$(RM) objs/*.o
	@-$(RM) libs/*
	@-$(RM) bin/*

cleanapps:
	@echo cleaning ./apps
	@cd apps; $(MAKE) clean; cd ..

.PHONY: tags
tags:
	ctags -R .
