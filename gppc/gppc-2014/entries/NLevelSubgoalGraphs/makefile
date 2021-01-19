CFLAGS = -DNO_HOG -O3
CC = g++
EXEC = NLevelSubgoalGraph
#MAP = 64room_000
#MAP = maze512-2-0
#MAP = random512-30-0
#MAP = AcrosstheCape
#MAP = arena2
#MAP = Aurora
#MAP = Predators
MAP = orz100d
#MAP = TheFrozenSea
#MAP = AR0011SR

all:	
	$(CC) $(CFLAGS) -o $(EXEC) main.cpp Entry.cpp ScenarioLoader.cpp Timer.cpp SubgoalGraph.cpp

test:	
	./$(EXEC) -pre $(MAP).map $(MAP).map.scen
	./$(EXEC) -run $(MAP).map $(MAP).map.scen
#	./$(EXEC) -full $(MAP).map $(MAP).scen
