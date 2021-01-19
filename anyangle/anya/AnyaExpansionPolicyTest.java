package pgraph.anya;

import java.util.ArrayList;

import pgraph.anya.AnyaExpansionPolicy;
import pgraph.anya.AnyaInterval;
import pgraph.anya.AnyaNode;
import pgraph.grid.BitpackedGrid;
import pgraph.test.Test;

public class AnyaExpansionPolicyTest implements Test
{
    public void test_generate_non_observable_flat()
    {
    	try
    	{
    		// disable pruning
	    	AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", false);
	    	
	    	// test; down corner, scan right
	    	AnyaInterval interval = new AnyaInterval(10,18, 6);
	    	AnyaNode node = new AnyaNode(null, interval, 10, 6);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project_f2c(node, ag.getGrid());	    	
	    	ag.flat_node_nobs(node, retval, projection);
	    	
	    	assert(retval.size() == 1);
	    	AnyaNode successor = retval.get(0);
	    	assert( successor.root.x == 18 && 
	    			successor.root.y == 6);
	    	
	    	assert( successor.interval.getLeft() == 18 && 
	    			successor.interval.getRight() == 22 && 
	    			successor.interval.getRow() == 7);
	    	
	    	// test; up corner, scan left
	    	interval.setRight(23);
	    	interval.setLeft(22);
	    	interval.setRow(7);
	    	node.root.setLocation(23, 7);
	    	
	    	projection.project_f2c(node, ag.getGrid());	    	
	    	ag.flat_node_nobs(node, retval, projection);
	    	assert(retval.size() == 2);

	    	successor = retval.get(1);
	    	assert( successor.root.x == 22 && 
	    			successor.root.y == 7);
	    	
	    	assert( successor.interval.getLeft() == 18 && 
	    			successor.interval.getRight() == 22 && 
	    			successor.interval.getRow() == 6);
	    	
	    	// test; up corner, scan right, multiple successors
	    	interval.init(58, 59, 5);
	    	node.root.setLocation(58, 5);
	    	retval.clear();
	    	
	    	projection.project_f2c(node, ag.getGrid());
	    	ag.flat_node_nobs(node, retval, projection);

	    	assert(retval.size() == 4);
    		assert(retval.get(3).interval.range_size() == 14);
    		assert(retval.get(2).interval.range_size() == 1);
    		assert(retval.get(1).interval.range_size() == 8);
    		assert(retval.get(0).interval.range_size() == 5);
    		
	    	// test; down corner, scan right
    		interval.init(82, 87, 3);
	    	node.root.setLocation(87, 3);
	    	retval.clear();
	    	
	    	projection.project_f2c(node, ag.getGrid());
	    	ag.flat_node_nobs(node, retval, projection);
	    	
	    	assert(retval.size() == 1);
	    	successor = retval.get(0);
	    	assert( successor.root.x == 82 && 
	    			successor.root.y == 3);

	    	assert( successor.interval.getLeft() == 74 && 
	    			successor.interval.getRight() == 82 && 
	    			successor.interval.getRow() == 4);
	    	
	    	// test; zero non-observable successors 
	    	interval.init(23, 26, 6);
	    	node.root.setLocation(23, 6);
	    	retval.clear();
	    	
	    	projection.project_f2c(node, ag.getGrid());
	    	ag.flat_node_nobs(node, retval, projection);
	    	
	    	assert(retval.size() == 0);

	    	System.out.println("test_generate_non_observable_flat; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    }
    
    public void test_generate_non_observable_flat_with_pruning()
    {
    	try
    	{
    		// enable pruning
	    	AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", true);
	    	
	    	// test; down corner, scan right
	    	AnyaInterval interval = new AnyaInterval(58,59, 5);
	    	AnyaNode node = new AnyaNode(null, interval, 58, 5);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project_f2c(node, ag.getGrid());
	    	ag.flat_node_nobs(node, retval, projection);

	    	assert(retval.size() == 2);
    		assert(retval.get(1).interval.range_size() == 14);
    		assert(retval.get(0).interval.range_size() == 8);
    		
    		// test; up corner, scan right. 
    		// generate sterile interval if it contains the goal
    		node.root.x = 58;
    		node.root.y = 5;
    		ag.tx_ = 84;
    		ag.ty_ = 4;
    		interval.init(58.0001, 59, 5);
    		retval.clear();

    		projection.project_f2c(node, ag.getGrid());
    		ag.flat_node_nobs(node, retval, projection);
    		assert(retval.size() == 3);
    		AnyaNode successor = retval.get(0);
    		assert(successor.interval.getLeft() == 82 && 
    				successor.interval.getRight() == 87 &&
    				successor.interval.getRow() == 4);

	    	System.out.println("test_generate_non_observable_flat_with_pruning; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    }
    
    public void test_generate_observable_flat()
    {
    	try
    	{
    		AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", false);
    		
    		// test; scan right
    		AnyaInterval interval = new AnyaInterval(18, 26, 18);
	    	AnyaNode node = new AnyaNode(null, interval, 18, 18);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.flat_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);

	    	AnyaNode successor = retval.get(0);
	    	assert( successor.interval.getLeft() == 26 && 
	    			successor.interval.getRight() == 38 &&
	    			successor.interval.getRow() == 18);
	    	assert(successor.root.x == 18 && successor.root.y == 18);	   
	    	
    		// test; generate sterile (scan right)
	    	successor = retval.get(0);
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.flat_node_obs(successor, retval, projection);
	    	assert(retval.size() == 2);

	    	// test; scan left
	    	interval.init(38, 47, interval.getRow());
	    	node.root.x = 47;
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.flat_node_obs(node, retval, projection);
	    	assert(retval.size() == 3);
	    	
	    	successor = retval.get(2);
	    	assert( successor.interval.getLeft() == 26 && 
	    			successor.interval.getRight() == 38 &&
	    			successor.interval.getRow() == 18);
	    	assert(successor.root.x == 47 && successor.root.y == 18);	
	    	
    		// test; generate sterile (scan left)
	    	successor = retval.get(2);
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.flat_node_obs(successor, retval, projection);
	    	assert(retval.size() == 4);
	    	
	    	System.out.println("test_generate_observable_flat; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    }
    
    public void test_generate_observable_flat_with_pruning()
    {
    	try
    	{
    		AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", true);
    		
    		// test; prune sterile (scan right)
    		AnyaInterval interval = new AnyaInterval(26, 38, 18);
	    	AnyaNode node = new AnyaNode(null, interval, 18, 18);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.flat_node_obs(node, retval, projection);
	    	assert(retval.size() == 0);
	    	
	    	// test; generate sterile when it contains the goal (scan right)
	    	ag.tx_ = 42;
	    	ag.ty_ = 18;
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.flat_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);

	    	// test; prune sterile (scan left)
	    	node.root.x = 47;
	    	projection.project(node, ag.getGrid());
	    	ag.flat_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	
	    	// test; generate sterile when it contains the goal (scan left)
	    	ag.tx_ = 22;
	    	projection.project(node, ag.getGrid());
	    	ag.flat_node_obs(node, retval, projection);
	    	assert(retval.size() == 2);
	    	
	    	
	    	System.out.println("test_generate_observable_flat_with_pruning; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    }
    
    public void test_generate_observable_cone()
    {
    	try
    	{
    		// going up
    		AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", false);
    		AnyaInterval interval = new AnyaInterval(18, 47, 8);
	    	AnyaNode node = new AnyaNode(null, interval, 18, 10);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 8);
	    	
	    	// going down
	    	node.root.x = 18;
	    	node.root.y = 16;
	    	interval.init(18, 47, 17);
	    	retval.clear();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 3);
	    	
	    	// going down (projection is not valid)
	    	node.root.x = 17;
	    	node.root.y = 16;
	    	interval.init(1,  17,  18);
	    	retval.clear();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 0);
	    	
	    	// going up (root > right endpoint)
	    	retval.clear();
	    	node.root.x = 17;
	    	node.root.y = 11;
	    	interval.init(1, 13, 9);
	    	projection.project(node, ag.getGrid());
	    	
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	AnyaNode successor = retval.get(0);
	    	assert(successor.interval.getLeft() == 1 
	    			&& successor.interval.getRight() == 11
	    			&& successor.interval.getRow() == 8);
	    	
	    	// going down (root < left endpoint)
	    	node.root.x = 10;
	    	node.root.y = 1;
	    	interval.init(18, 22, 6);
	    	retval.clear();

	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);
	    		    	
	    	System.out.println("test_generate_observable_cone; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    }
    
    public void test_generate_observable_cone_with_pruning()
    {
       	try
    	{
       		// going up (root == left endpoint)
    		AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", true);
    		AnyaInterval interval = new AnyaInterval(18, 47, 8);
	    	AnyaNode node = new AnyaNode(null, interval, 18, 10);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 4);
	    	
	    	// going up (root > right endpoint)
	    	retval.clear();
	    	node.root.x = 17;
	    	node.root.y = 11;
	    	interval.init(1, 13, 9);
	    	projection.project(node, ag.getGrid());
	    	
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	AnyaNode successor = retval.get(0);
	    	assert(successor.interval.getLeft() == 5 
	    			&& successor.interval.getRight() == 9);
	    	
	    	// going down (root == left endpoint)
	    	node.root.x = 18;
	    	node.root.y = 16;
	    	interval.init(18, 47, 17);
	    	retval.clear();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 2);
	    	
	    	// going down (root < left endpoint)
	    	node.root.x = 10;
	    	node.root.y = 1;
	    	interval.init(18, 22, 6);
	    	retval.clear();

	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	
	    	// going down (projection is not observable)
	    	retval.clear();
	    	node.root.x = 10;
	    	node.root.y = 5;
	    	interval.init(18, 22, 6);
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(!projection.observable);
	    	assert(retval.size() == 0);

	    	// going down (generate nothing if projection is not valid)
	    	node.root.x = 17;
	    	node.root.y = 16;
	    	interval.init(1, 17, 18);
	    	retval.clear();	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 0);
	    	
	    	// going up (rootx == left); turn left at first opportunity
	    	node.root.x = 18;
	    	node.root.y = 47;
	    	interval.init(18, 26, 44);
	    	retval.clear();	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	successor = retval.get(0);
	    	assert(successor.interval.getRow() == 41 &&
	    			successor.interval.getLeft() == 18 &&
	    			successor.interval.getRight() == 34);
	    	
	    	// going down; scan left
	    	// goal interval is one among several potential successors
	    	// make sure we do not mark it as intermediate and accidentally 
	    	// skip over it
	    	node.root.setLocation(45, 6);
	    	node.interval.init(41, 45, 7);
	    	retval.clear();
	    	ag.tx_ = 19;
	    	ag.ty_ = 13;
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	successor = retval.get(0);
	    	assert(successor.interval.getLeft() <= ag.tx_ &&
	    			successor.interval.getRight() >= ag.tx_ &&
	    			successor.interval.getRow() == ag.ty_);

	    	
	    	System.out.println("test_generate_observable_cone_with_pruning; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    }
    
    public void test_generate_non_observable_cone()
    {
       	try
    	{
       		// going up
    		AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", false);
    		AnyaInterval interval = new AnyaInterval(5, 9, 6);
	    	AnyaNode node = new AnyaNode(null, interval, 5, 7);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 2);
	    	
	    	// going up (root > right endpoint)
	    	retval.clear();
	    	node.root.x = 17;
	    	node.root.y = 11;
	    	interval.init(5, 9, 7);
	    	projection.project(node, ag.getGrid());
	    	
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	AnyaNode successor = retval.get(0);
	    	assert(successor.interval.getLeft() == 7	 
	    			&& successor.interval.getRight() == 9);
	    	
	    	// going up (root < left endpoint)
	    	retval.clear();
	    	node.root.x = 1;
	    	node.root.y = 11;
	    	interval.init(5, 9, 7);
	    	projection.project(node, ag.getGrid());
	    	
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	successor = retval.get(0);
	    	assert(successor.interval.getLeft() == 5	 
	    			&& successor.interval.getRight() == 6
	    			&& successor.interval.getRow() == 6);
	    	
	    	// going down (root < left endpoint)
	    	node.root.x = 10;
	    	node.root.y = 1;
	    	interval.init(18, 22, 6);
	    	retval.clear();

	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	successor = retval.get(0);
	    	assert(successor.interval.getLeft() == 18	 
	    			&& successor.interval.getRight() == 19.6
	    			&& successor.interval.getRow() == 7);
	    	
	    	// going down; do not generate flat non-taut successors
	    	node.root.x = 47;
	    	node.root.y = 8;
	    	interval.init(18, 47, 16);
	    	retval.clear();

	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 0);	    	
	    	
	    	// going down; interval size < 1
	    	node.root.x = 38;
	    	node.root.y = 18;
	    	interval.init(48, 48.636363636363, 31);
	    	retval.clear();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	
	    	// going down; generate flat and conical successors
	    	// left of the current interval
	    	node.root.x = 52;
	    	node.root.y = 70;
	    	interval.init(52, 52.3, 71);
	    	retval.clear();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 2);
	    	successor = retval.get(0);
	    	assert(successor.interval.getRow() == 71 && 
	    			successor.interval.getLeft() == 47);
	    	successor = retval.get(1);
	    	assert(successor.interval.getRow() == 72 && 
	    			successor.interval.getLeft() == 18);

	    	
	    	// the following tests ensure that we generate non-observable successors 
	    	// even when there is no adjacent observable projection that is valid 
	    	// (i.e. the angle from the root to the interval is too low to observe 
	    	// points from the next row)
	    	
	    	// going up and left; 
	    	node.root.x = 59;
	    	node.root.y = 5;
	    	interval.init(74, 82, 4);
	    	retval.clear();
	    	
	    	projection.project(node, ag.getGrid());
	    	assert(projection.valid && !projection.observable);
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 1);	 
	    	successor = retval.get(0);
	    	assert(successor.interval.getLeft() == node.interval.getLeft() && 
	    			successor.interval.getRight() == projection.max_right);
	    	
	    	// going up and right
	    	node.root.x = 71;
	    	node.root.y = 39;
	    	interval.init(38, 47, 37);
	    	retval.clear();
	    	
	    	projection.project(node, ag.getGrid());
	    	assert(projection.valid && !projection.observable);
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 1);	 
	    	successor = retval.get(0);
	    	assert(successor.interval.getRight() == node.interval.getRight() && 
	    			successor.interval.getLeft() == projection.max_left);


	    	System.out.println("test_generate_non_observable_cone; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    }
    
    public void test_generate_non_observable_cone_with_pruning()
    {
       	try
    	{
       		// going up
    		AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", true);
    		AnyaInterval interval = new AnyaInterval(5, 9, 6);
	    	AnyaNode node = new AnyaNode(null, interval, 5, 7);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 0);
	    	
	    	// going up (root > right endpoint)
	    	retval.clear();
	    	node.root.x = 17;
	    	node.root.y = 11;
	    	interval.init(5, 9, 7);
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 0);
	    	
	    	// going down; do not generate flat non-taut successors
	    	node.root.x = 47;
	    	node.root.y = 8;
	    	interval.init(18, 47, 16);
	    	retval.clear();

	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_nobs(node, retval, projection);
	    	assert(retval.size() == 0);	   
	    	
	    	System.out.println("test_generate_non_observable_cone; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    }
    
    public void test_generate_observable_cone_with_intermediate_pruning()
    {
       	try
    	{
       		// going up; goal is not in an intermediate interval
    		AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", true);
    		AnyaInterval interval = new AnyaInterval(48, 66, 11);
	    	AnyaNode node = new AnyaNode(null, interval, 48, 12);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);

	    	assert(retval.size() == 1);

	    	AnyaNode successor = retval.get(0);
	    	assert(successor.interval.getRow() == 8);
	    	assert(successor.interval.getLeft() == 48 && 
	    			successor.interval.getRight() == 66);
	    		
	    	// going up; goal is in an intermediate interval
	    	ag.tx_ = 55;
	    	ag.ty_ = 9;
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 2);

	    	successor = retval.get(1);
	    	assert(successor.interval.getRow() == 9);
	    	assert(successor.interval.getLeft() == 48 && 
	    			successor.interval.getRight() == 66);
	    	
	    	// going down; do not generate flat non-taut successors
	    	node.root.x = 5;
	    	node.root.y = 1;
	    	interval.init(5, 9, 3);
	    	retval.clear();

	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 1);
	    	successor = retval.get(0);
	    	assert(successor.interval.getRow() == 7);
	    	
	    	System.out.println("test_observable_cone_with_intermediate_pruning; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}	    	
    }
    
    public void test_generate_observable_flat_with_intermediate_pruning()
    {
       	try
    	{
       		// going up; goal is not in an intermediate interval
    		AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", true);
    		AnyaInterval interval = new AnyaInterval(26, 37, 7);
	    	AnyaNode node = new AnyaNode(null, interval, 18, 7);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	    	IntervalProjection projection = new IntervalProjection();
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);

	    	assert(retval.size() == 1);
	    	AnyaNode successor = retval.get(0);
	    	assert(successor.interval.getRow() == 7);
	    	assert(successor.interval.getLeft() == 40 && 
	    			successor.interval.getRight() == 41);
	    	assert(successor.root.equals(node.root));
	    	
       		// going up; goal is in an intermediate interval
	    	ag.tx_ = 39;
	    	ag.ty_ = 7;
	    	
	    	projection.project(node, ag.getGrid());
	    	ag.cone_node_obs(node, retval, projection);
	    	assert(retval.size() == 2);
	    	assert(successor.interval.getRow() == 7);
	    	assert(successor.interval.getLeft() == 37 && 
	    			successor.interval.getRight() == 40);
	    	assert(successor.root.equals(node.root));
	    	
	    	System.out.println("test_observable_cone_with_intermediate_pruning; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}	    	
    }
   
    public void test_generate_start_successors()
    {
       	try
    	{
       		// going up; goal is not in an intermediate interval
    		AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/local/CSC2F.map", false);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();

	    	// test; start node is a corner (loc (x, y-1) blocked; (x-1, y-1) free)
	    	// should have 1 flat successor to the left, one conical successor left-down
	    	// and one conical successor up-left
    		AnyaInterval interval = new AnyaInterval(42, 42, 37);
	    	AnyaNode node = new AnyaNode(null, interval, 42, 37);
	    	
	    	// TODO: test target is visible from and on adjacent row to start
	    	// TODO: test target is visible from and on the same row as start

	    	ag.generate_start_successors(node, retval);
	    	assert(retval.size() == 6);

	    	// test; start node is a corner (loc (x, y-1) blocked; (x-1, y-1) free)
	    	// shoud have no successor up-and-right but a successor everywhere else
    		interval.init(47, 47, 37);
	    	node.root.setLocation(47, 37);
	    	retval.clear();
	    	
	    	ag.generate_start_successors(node, retval);
	    	assert(retval.size() == 5);
	    	
	    	// test; start node has a single-width obstacle directly above and a
	    	// single-width obstacle directly to the left. the only valid 
	    	// successors are right and down-right
    		interval.init(10, 10, 1);
	    	node.root.setLocation(10, 1);
	    	retval.clear();
	    	
	    	ag.generate_start_successors(node, retval);
	    	assert(retval.size() == 2);
	    	
	    	// test; start node has a single-width obstacle directly above and a
	    	// single-width obstacle directly to the right. the only valid 
	    	// successors are left and down-left
    		interval.init(22, 22, 1);
	    	node.root.setLocation(22, 1);
	    	retval.clear();
	    	
	    	ag.generate_start_successors(node, retval);
	    	assert(retval.size() == 2);

	    	System.out.println("test_generate_start_successors; all tests done");
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}	
    }
    
    private void test_test()
    {
    	
    	try
    	{
    		// >> expanding (f=322.3367433083149) searchnode 352367347;root: Point2D.Double[319.0, 70.0] Interval (319.0, 319.3333333333333, 71)
	    	
	   		// going up; goal is not in an intermediate interval
			AnyaExpansionPolicy ag = new AnyaExpansionPolicy("maps/random/random512-40-7.map", false);
	    	ArrayList<AnyaNode> retval = new ArrayList<AnyaNode>();
	
			AnyaInterval interval = new AnyaInterval(319, 319+(1.0/3.0), 71);
	    	AnyaNode node = new AnyaNode(null, interval, 319, 70);
	    	
	    	ag.generate_successors(node, retval);
	    	int sz = retval.size();
	    	sz = sz;
    	}
    	catch(Exception e)
    	{
    		e.printStackTrace();
    	}
    }
    
    
    
    public void run_tests()
    {
		AnyaExpansionPolicyTest test = new AnyaExpansionPolicyTest();
		test.test_generate_non_observable_flat();
		test.test_generate_non_observable_flat_with_pruning();

		test.test_generate_observable_flat();
		test.test_generate_observable_flat_with_pruning();
		
		test.test_generate_observable_cone();
		test.test_generate_observable_cone_with_pruning();
		
		test.test_generate_non_observable_cone();
		test.test_generate_non_observable_cone_with_pruning();
		
		test_generate_observable_cone_with_intermediate_pruning();
		
		test_generate_start_successors();
		test_test();
    }
}
