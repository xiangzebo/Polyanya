SHELL = /bin/bash
CC = g++

FAST_CFLAGS =  -O3 -ansi -DNDEBUG
DEV_CFLAGS =  -Wall -Wno-long-long -Wno-deprecated -g -ggdb -ansi -pedantic

# locations of library files program depends on
ifeq ("$(OPENGL)", "STUB")
  _CFLAGS += -I../driver/STUB/ -I../driver/STUB/GL/ -DNO_OPENGL
endif

ifeq ("$(findstring Darwin, "$(shell uname -s)")", "Darwin")
  # MacOS
  _CFLAGS += -DOS_MAC -I/opt/local/include/ -I/usr/local/include/ 
  _CFLAGS += -I/System/Library/Frameworks/Foundation.framework/Versions/A/Headers/
  _CFLAGS += -I/System/Library/Frameworks/AppKit.framework/Versions/A/Headers/
  LIBFLAGS = -framework AppKit -framework Foundation
  ifneq ("$(OPENGL)", "STUB")
    _CFLAGS += -I/System/Library/Frameworks/GLUT.framework/Versions/A/Headers/
    _CFLAGS += -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/
    _CFLAGS += -I/System/Library/Frameworks/AGL.framework/Versions/A/Headers/
    LIBFLAGS += -framework GLUT -framework OpenGL 
  endif
else 
  ifeq ("$(findstring CYGWIN, $(shell uname -s))", "CYGWIN")
  # Cygwin/Windows
    _CFLAGS += -Dlinux 
    LIBFLAGS = -Lapps/libs -L/lib/w32api
    ifneq ("$(OPENGL)", "STUB")
	  _CFLAGS += -I/usr/include/opengl
      LIBFLAGS += -lopengl32 -lglu32 -lglut32 
    endif
  else 
	# Linux et al 
    _CFLAGS += -Dlinux 
    LIBFLAGS = -Lapps/libs -L/usr/lib -L$(HOME)/lib -L/opt/local/lib -L/usr/local/lib
    ifneq ("$(OPENGL)", "STUB")
	  _CFLAGS += -I/usr/include/GL
      LIBFLAGS += -L/usr/X11R6/lib64 -L/usr/X11R6/lib -lGL -lGLU -lglut -lXi -lXmu 
    endif
  endif
endif

HOGCORE_INCLUDE = -I../jump -I../hpa -I../rsr -I../abstraction -I../driver -I../shared \
				  -I../simulation -I../util -I../policies -I../filters -I../heuristics
EXTRAS_INCLUDE = -I../extras
DRIVER_INCLUDE = -I../driver

