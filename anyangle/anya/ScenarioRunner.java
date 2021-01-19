import java.awt.Point;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import pgraph.anya.experiments.AStarExperimentLoader;
import pgraph.anya.experiments.AnyaExperimentLoader;
import pgraph.anya.experiments.ExperimentInterface;

import pgraph.alg.AnyaSearch;
import pgraph.alg.FastAStar;
import pgraph.anya.AnyaExpansionPolicy;
import pgraph.anya.AnyaInterval;
import pgraph.anya.AnyaNode;
import pgraph.grid.BitpackedGrid;
import pgraph.grid.BitpackedGridExpansionPolicy;
import pgraph.util.MicroBenchmark;

public class ScenarioRunner {

	private static boolean verbose = false;

    public static void run_astar(String scenarioFilePath)
    {    	
        System.gc();
        AStarExperimentLoader expLoader = new AStarExperimentLoader();                
        BitpackedGrid grid;
        BitpackedGridExpansionPolicy expander;
        List<ExperimentInterface> experiments;
        FastAStar<Point> astar; 
                
        try
        {
        	experiments = expLoader.loadExperiments(scenarioFilePath);   
            if(experiments.size() == 0)
            {
            	System.err.println("No experiments to run; finishing.");
            	return;
            }
            grid = new BitpackedGrid(experiments.get(0).getMapFile());
            expander = new BitpackedGridExpansionPolicy(grid);
            astar = new FastAStar<Point>(expander, grid.get_num_cells());
            astar.verbose = ScenarioRunner.verbose;
        }
        catch(Exception e)
        {
        	e.printStackTrace(System.err);
        	return;
        }
        
        System.out.println("exp" + ";"+
        		"alg" + ";" +
                "wallt_micro"+";"+
                "runt_micro" + ";"+
                "expanded"+";" +
                "generated"+ ";"+
                "heapops" +";"+
                "start" +";"+
                "target"  +";"+
                "gridcost"+";"+
                "realcost"+";"+
                "map" +";");

        
        MicroBenchmark exp_runner = new MicroBenchmark(astar);

        for (int i = 0; i < experiments.size(); i++)
        {
        	ExperimentInterface exp = experiments.get(i);
        	astar.mb_start_ = expander.getGridVertex(exp.getStartX(), exp.getStartY());
        	astar.mb_target_ = expander.getGridVertex(exp.getEndX(), exp.getEndY());
        	
        	long wallt_micro = exp_runner.benchmark(1);
        	double cost = astar.mb_cost_;
        	long duration = (long)(exp_runner.getAvgTime()+0.5);
        	
            System.out.println(i + ";"+ 
            		"AStar" + ";" +
                    wallt_micro + ";"+
            		duration + ";"+
                    astar.expanded + ";"+
                    astar.generated + ";"+
                    astar.heap_ops + ";" +
                    "("+ exp.getStartX()+","+ exp.getStartY() + ")" +";"+
                    "(" + exp.getEndX() + "," + exp.getEndY() + ")" +";"+
                    exp.getUpperBound()  +";"+
                    cost+";"+
                    exp.getMapFile() +";");
        }
    }

    public static void run_anya(String scenarioFilePath)
    {
        System.gc();
        AnyaExperimentLoader exploader = new AnyaExperimentLoader();
        List<ExperimentInterface> experiments = null;
        AnyaSearch anya;

        try
        {
            experiments = exploader.loadExperiments(scenarioFilePath); 
            if(experiments.size() == 0)
            {
            	System.err.println("No experiments to run; finishing.");
            	return;
            }
            String mapfile = experiments.get(0).getMapFile();
        	anya = new AnyaSearch(new AnyaExpansionPolicy(mapfile));
        	anya.verbose = ScenarioRunner.verbose;
        }
        catch(Exception e)
        {
        	e.printStackTrace(System.err);
        	return;
        }
        
        System.out.println("exp" + ";"+
        		"alg" + ";" +
                "wallt_micro"+";"+
                "runt_micro" + ";"+
                "expanded"+";" +
                "generated"+ ";"+
                "heapops" +";"+
                "start" +";"+
                "target"  +";"+
                "gridcost"+";"+
                "realcost"+";"+
                "map");

        
        MicroBenchmark exp_runner = new MicroBenchmark(anya);
        AnyaNode start = new AnyaNode(null, new AnyaInterval(0, 0, 0), 0, 0);
        AnyaNode target = new AnyaNode(null, new AnyaInterval(0, 0, 0), 0, 0);
    	anya.mb_start_ = start;
    	anya.mb_target_ = target;
        for (int i = 0; i < experiments.size(); i++)
        {
        	ExperimentInterface exp = experiments.get(i);
        	start.root.setLocation(exp.getStartX(), exp.getStartY());
        	start.interval.init(exp.getStartX(), exp.getStartX(), exp.getStartY());
        	target.root.setLocation(exp.getEndX(), exp.getEndY());
        	target.interval.init(exp.getEndX(), exp.getEndX(), exp.getEndY());
        	        	
        	long wallt_micro = exp_runner.benchmark(1);
        	double cost = anya.mb_cost_;
        	long duration = (long)(exp_runner.getAvgTime()+0.5);
        	
            System.out.println(i + ";"+ 
            		"AnyaSearch" + ";" +
                    wallt_micro + ";"+
                    duration + ";"+
                    anya.expanded + ";"+
                    anya.generated + ";"+
                    anya.heap_ops + ";" +
                    "("+ exp.getStartX()+","+ exp.getStartY() + ")" +";"+
                    "(" + exp.getEndX() + "," + exp.getEndY() + ")" +";"+
                    exp.getUpperBound()  +";"+
                    cost+";"+
                    exp.getMapFile());

        }
    }
    
    public static void main(String[] args) throws IOException 
    {
    	String algo = "";
    	String scenario = "";
    	for(int i = 0; i < args.length; i++)
    	{
    		if(args[i].equals("-v"))
    		{
    			ScenarioRunner.verbose = true;
    			continue;
    		}
    		
    		if( args[i].equals("-ASTAR") || args[i].equals("-ANYA"))
    		{
    			if(algo.equals("") && (i+1) < args.length)
    			{
    				algo = args[i];
    				scenario = args[i+1];
    				i++;
    			}
    			else
    			{
    				printHelp();
    			}
    			continue;
    		}    		
    	}
    	
    	run(algo, scenario);
    	
    }
    
    private static void run(String algo, String scenario) throws IOException
    {
    	if(algo.equals("") || scenario.equals(""))
    	{
    		printHelp();
    		return;
		}
    	
    	switch(algo)
    	{
    		case "-ASTAR":
    			run_astar(scenario);
    			break;
    		case "-ANYA":
                run_anya(scenario);
    	}
    }
    
    private static void printHelp()
    {
		System.err.println("Parameters: [alg] [scenario]\n;"+
				"Possible values for [alg]: -ASTAR, -ANYA");

    }

}
