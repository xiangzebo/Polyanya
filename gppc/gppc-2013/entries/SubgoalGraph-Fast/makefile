EXEC = SubgoalGraph-Fast
MAP = AcrosstheCape

all:	
	g++ -O3 *.cpp -o $(EXEC)
	
test:	
	g++ -O3 *.cpp -o $(EXEC)
	./$(EXEC) -pre $(MAP).map $(MAP).map.scen
	./$(EXEC) -run $(MAP).map $(MAP).map.scen
