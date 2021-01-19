This code was developed by Tansel Uras (turas@usc.edu) at USC.
If you use this code in your research, please cite our IJCAI paper:

Daniel D. Harabor, Tansel Uras, Peter J. Stuckey, Sven Koenig. Regarding Jump Point Search and Subgoal Graphs. In Proceedings of the International Joint Conference on Artificial Intelligence (IJCAI), 2019.

Bibtex:
@inproceedings{uras:18,
  author = "Daniel D. Harabor and Tansel Uras and Peter J. Stuckey and Sven Koenig",
  title = "Regarding Jump Point Search and Subgoal Graphs",
  booktitle = "Proceedings of the International Joint Conference on Artificial Intelligence",
  year = "2019"
}

This package contains the source files and scripts for running the experiments and generating result summaries. 

Compiling:
Simply run 'make' to generate the executable "SG-Grid".

Running "SG-Grid":
SG-Grid executable can be run with two arguments: 
1) The path to a map file. The associated scenario file should be contained in the same directory as the map file. For instance, if the map file "arena2.map" and the associated scenario file "arena2.map.scen" are both contained in the directory "maps", use the command "./SG-Grid maps/arena2.map".
2) The ID of the algorithm, ranging from 1-9 (1: A*, 2: CH-GPPC, 3: CH, 4: SG, 5: JPD, 6: JP, 7: SG-CH, 8: SG-JPD, 9: SG-JP). For instance, to run SG on arena2.map, use the command "./SG-Grid maps/arena2.map 4". If the second argument is omitted, the executable will return the number of algorithms. 

Running experiments with "experiment.pl":
Place all maps and associated scenario files in a directory called "maps", located within the same directory a the "SG-Grid" executable and run "perl experiments.pl". The "maps" directory can be organized hierarchically: For instance, it can contain a "MovingAI" subdirectory with all the MovingAI benchmarks, further subdivided into game, random, room, maze, and street maps, and a "GPPC" subdirectory with all the GPPC benchmarks. The location for the experiments can be changed within the "experiment.pl" file. 

Extracting results with "tables.pl":
After running the experiments, run "perl tables.pl".
