CC = g++

# Required to compile if not running in HOG.
CFLAGS = -std=c++11
DBG_FLAGS = -DNO_HOG
REL_FLAGS = -DNO_HOG -DNDEBUG -DSG_QUIET -O3

SG_PATH = source
SG_INC = \
  -I$(SG_PATH) \
  -I$(SG_PATH)/CH_GPPC \
  -I$(SG_PATH)/ContractionHierarchy \
  -I$(SG_PATH)/Graphs \
  -I$(SG_PATH)/Graphs/Grid2D \
  -I$(SG_PATH)/Graphs/LatticeGraph \
  -I$(SG_PATH)/Graphs/LatticeGraph/Heuristic \
  -I$(SG_PATH)/Graphs/LatticeGraph/Lattice \
  -I$(SG_PATH)/Graphs/LatticeGraph/LatticeFreespace \
  -I$(SG_PATH)/Parameters \
  -I$(SG_PATH)/QueryMethods \
  -I$(SG_PATH)/ReachabilityRelation \
  -I$(SG_PATH)/ReachabilityRelation/Grid2D \
  -I$(SG_PATH)/ReachabilityRelation/Lattice \
  -I$(SG_PATH)/SearchMethods \
  -I$(SG_PATH)/SubgoalGraph \
  -I$(SG_PATH)/SubgoalGraph/Construction \
  -I$(SG_PATH)/SubgoalGraph/Grid2D \
  -I$(SG_PATH)/SubgoalGraph/Lattice \
  -I$(SG_PATH)/SubgoalGraph/Search \
  -I$(SG_PATH)/Utils \
  -I$(SG_PATH)/Utils/Experiments \
  -I$(SG_PATH)/Utils/Statistics

APP_SG = $(SG_PATH)
SRC_CPP = \
  $(APP_SG)/Graphs/Grid2D/Grid2D.cpp \
  $(APP_SG)/Graphs/Grid2D/GPPCGrid.cpp \
  $(APP_SG)/Graphs/LatticeGraph/Lattice/MotionPrimitive.cpp \
  $(APP_SG)/Graphs/LatticeGraph/Lattice/Lattice.cpp \
  $(APP_SG)/Graphs/LatticeGraph/LatticeGraph.cpp \
  $(APP_SG)/Graphs/LatticeGraph/LatticeFreespace/LatticeFreespace.cpp \
  $(APP_SG)/SubgoalGraph/Lattice/LatticeSG.cpp \
  $(APP_SG)/Graphs/CommonHeuristics.cpp \
  $(APP_SG)/ContractionHierarchy/ContractionCore.cpp \
  $(APP_SG)/Utils/CPUTimer.cpp \
  $(APP_SG)/CH_GPPC/mapper.cpp \
  $(APP_SG)/Utils/Experiments/ScenarioLoader.cpp \
  main.cpp


MAP = arena
#MAP = 64room_000
#MAP = maze512-2-0
#MAP = random512-10-0
#MAP = AcrosstheCape
#MAP = arena2
#MAP = brc000d
#MAP = brc203d
#MAP = den005d
#MAP = lak304d
#MAP = random512-10-0
#MAP = AR0011SR
#MAP = AR0602SR
#MAP = GhostTown
#MAP = 64room_000
#MAP = maze512-32-0
#MAP = BigGameHunters
#MAP = Aurora
#MAP = Predators
#MAP = orz100d
#MAP = ost100d
#MAP = random512-40-3
#MAP = random512-40-8
#MAP = TheFrozenSea
#MAP = AR0011SR
#MAP = orz700d
#MAP = lak405d

SG_GRID_FLAG = 
SG_GRID_EXEC = SG-Grid

all:
	make sggrid
	
sggrid:
#	$(CC) $(CFLAGS) $(DBG_FLAGS) $(SG_FLAG) $(SG_INC) -o $(SG_EXEC).db $(SRC_CPP)
	$(CC) $(CFLAGS) $(REL_FLAGS) $(SG_GRID_FLAG) $(SG_INC) -o $(SG_GRID_EXEC) $(SRC_CPP)

#gdb:
#	make debug
#	gdb --args ./$(EXEC).db dao arena2

