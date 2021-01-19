package pgraph.util;

import pgraph.anya.experiments.MBRunnable;

/**
 * Created by Dindar on 21.9.2014.
 */
public class MicroBenchmark {

    MBRunnable runnable= null;

    double avgTime = -1;
    long maxTime = -1;
    long minTime = Long.MAX_VALUE;

    public MicroBenchmark(MBRunnable runnable) {
        this.runnable = runnable;
    }

    public double getAvgTime() {
        return avgTime;
    }

    public long getMaxTime() {
        return maxTime;
    }

    public long getMinTime() {
        return minTime;
    }

    // run an experiment and record some statistics. experiments are
    // run repeatedly until the recorded time  
    // @return elapsed wallclock time in micro-secs
    public long benchmark(int reps)
    {
        if (reps<=0)
            return 0;
        
        long wall_start = System.nanoTime();
        gc_if_necessary();      
        
        long start = System.nanoTime();
        for (int i= 0; i<reps; i++)
        {
            double free_memory_before = java.lang.Runtime.getRuntime().freeMemory();      
            runnable.run();
            double free_memory_after = java.lang.Runtime.getRuntime().freeMemory();
            // GC might have occurred between instances; restart in such cases
            // NB: this check is blunt and undetected gc episodes can still occur
            if(free_memory_after > free_memory_before)
            {
            	i = -1;
            	gc_if_necessary();
            	start = System.nanoTime();
            }
        }
        long totaltime = (System.nanoTime()-start);
        avgTime = (totaltime / 1000.0) / reps; // in microsecs
        
        // rerun the experiment if the total time is below the 
        // guaranteed resolution of the timer (1 millisecond)
        if((totaltime/1000000) == 0) 
        {
        	this.benchmark(reps*2);
        }

        cleanUp();
        return (long)(((System.nanoTime() - wall_start) / 1000)+0.5);
    }
    
    // run the garbage collector if memory is getting low
    private void gc_if_necessary()
    {
        Runtime runtime = java.lang.Runtime.getRuntime();
        if((runtime.freeMemory() / (double)runtime.totalMemory()) < 0.1)
        {
            runtime.gc();
        }
    }

    private void cleanUp() 
    {
        runnable.cleanUp();
    }

    private void run(boolean validIteration)
    {
        long start = System.nanoTime();
        runnable.run();
        long time = (System.nanoTime()-start)/1000; // converting to microseconds

        if(!validIteration)
            return;
        //System.out.println(" Time : "+ time);

        avgTime += time;

        if (maxTime<time)
            maxTime = time;

        if (minTime>time )
            minTime= time;


    }


}
