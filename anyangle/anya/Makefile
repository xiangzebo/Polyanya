ANYA_SRC = $(wildcard *java)
ANYA_CLASS = $(ANYA_SRC:.java=.class)

.PHONY: all
all: 
	javac -d ./ $(ANYA_SRC)

.PHONY: clean
clean: 
	-rm -rf org
	-rm -rf pgraph
	-rm ScenarioRunner.class
