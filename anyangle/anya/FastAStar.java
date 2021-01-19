package pgraph.alg;

import org.jgrapht.traverse.Heuristic;
import org.jgrapht.util.FibonacciHeap;
import org.jgrapht.util.FibonacciHeapNode;

import pgraph.anya.experiments.MBRunnable;
import pgraph.base.ExpansionPolicy;

// An implementation of A* optimised for explicit search spaces.
// Uses an object pool to avoid constant memory re-allocation.
//  
// @author: dharabor
// @created: 2015-04-02
//

public class FastAStar<V> implements MBRunnable {

	private static int search_id_counter = 0;
	private ExpansionPolicy<V> expander;
	private Heuristic<V> heuristic;	
	private Object[] pool;
	
	public int expanded;
	public int insertions;
	public int generated;
	public int heap_ops;
	FibonacciHeap<V> open;
	
	public V mb_start_;
	public V mb_target_;
	public double mb_cost_;
	
	public boolean verbose = false;
	
	// This class holds various bits of data needed to 
	// drive the search
    class SearchNode extends FibonacciHeapNode<V>
    {
    	// parent node
    	private SearchNode parent;

    	// tracks if the node has been added to open
        public int search_id;
    	
        // tracks if the node has been expanded
        public boolean closed;
                
        SearchNode(V vertex) 
        { 
        	super(vertex);
        	search_id = -1;
        }
        
        public void reset()
        {
        	parent = null;
        	search_id = search_id_counter;
        	closed = false;
        	super.reset();
        }
                
        public String toString()
        {
        	return "searchnode "+expander.hashCode(this.getData()) + ";" 
        				+ this.getData().toString() + " g_val "
        				+ this.getSecondaryKey()+" parent "+ 
        				(parent==null?"null":parent.getData().toString());
        }
    }
	
	public FastAStar(ExpansionPolicy<V> expander, int search_space_size)
	{
		pool = new Object[search_space_size];
		open = new FibonacciHeap<V>();
		this.heuristic = expander.heuristic();
		this.expander = expander;
	}
	
	private void init()
	{
		search_id_counter++;
		expanded = 0;
		insertions = 0;
		generated = 0;
		heap_ops = 0;
		open.clear();
	}
		
	public Path<V> search(V start, V target)
	{
		double cost = this.search_costonly(start, target);
		// generate the path
		Path<V> path = null; 
		if(cost != -1)
		{
			SearchNode node = generate(target);
			do
			{
				path = new Path<V>(node.getData(), path, node.getSecondaryKey());
				node = node.parent;
				
			}while(!(node.parent == null));
		}
		return path;
	}
	
    private void print_path(SearchNode current, java.io.PrintStream stream)
    {
    	if(current.parent != null)
    	{
    		print_path(current.parent, stream);
    	}
    	stream.println(expander.hashCode(current.getData()) + "; " 
    			+ current.getData().toString()  
    			+ "; g=" + current.getSecondaryKey());
    }

	public double search_costonly(V start, V target)
	{
		init();
		double cost = -1;
		if(!expander.validate_instance(start, target)) 
		{
			return cost;
		}
		
		SearchNode startNode = generate(start);
		startNode.reset();
		open.insert(startNode, heuristic.getValue(start, target), 0);
		
		while(!open.isEmpty())
		{
			SearchNode current = (SearchNode)open.removeMin();
			if(verbose) { System.out.println("expanding (f="+current.getKey()+") "+current.toString()); }
			expander.expand(current.getData());
			expanded++;
			heap_ops++;
			if(current.getData().equals(target))
			{
				// found the goal
				cost = current.getKey();
				
				if(verbose)
				{
					print_path(current, System.err);
				}
				break;
			}
			
			// iterate over all neighbours			
			while(expander.hasNext())
			{
				SearchNode neighbour = generate(expander.next());			
				if(neighbour.search_id != search_id_counter)
				{
					// neighbour is not on open or closed
					neighbour.reset();
					neighbour.parent = current;
					double new_g_value = 
						current.getSecondaryKey() + expander.step_cost();
					open.insert(neighbour, 
							new_g_value +
							heuristic.getValue(neighbour.getData(), target),
							new_g_value); 
							
					heap_ops++;
					insertions++;
					if(verbose)
					{
						System.out.println("\tinserting with f=" + neighbour.getKey() +" (g= "+new_g_value+");" + neighbour.toString());
					}
				}
				else 
				{ 
					if(!neighbour.closed)
					{
						// neighbour is on open; relax if necessary
						double edge_weight = expander.step_cost();
						double alt_g = current.getSecondaryKey() + edge_weight;
						if(alt_g < neighbour.getSecondaryKey())
						{
							neighbour.parent = current;
							open.decreaseKey(neighbour, 
									alt_g + 
									heuristic.getValue(neighbour.getData(), target),
									alt_g);
							heap_ops++;
							if(verbose)
							{
								System.out.println("\trelaxing with f=" + neighbour.getKey() +" (g= "+alt_g+");" + neighbour.toString());
							}
						}
						else
						{
							if(verbose)
							{
								System.out.println("\tcannot relax " + neighbour.toString());								
							}
						}
					}
					else
					{
						if(verbose)
						{
							System.out.println("\talready closed " + neighbour.toString());
						}
					}
				}
			}
			current.closed = true;
		}
		if(verbose)
		{
			System.out.println("finishing search");
		}
		return cost;
	}
	
	private SearchNode 
	generate(V v)
	{
		SearchNode retval = (SearchNode)pool[expander.hashCode(v)];
		if(retval == null)
		{
			retval = new SearchNode(v);
			pool[expander.hashCode(v)] = retval;
		}
		generated++;
		return retval;
	}
	
	public int getExpanded() {
		return expanded;
	}

	public void setExpanded(int expanded) {
		this.expanded = expanded;
	}

	public int getGenerated() {
		return insertions;
	}

	public void setGenerated(int generated) {
		this.insertions = generated;
	}

	public int getTouched() {
		return generated;
	}

	public void setTouched(int touched) {
		this.generated = touched;
	}

	public int getHeap_ops() {
		return heap_ops;
	}

	public void setHeap_ops(int heap_ops) {
		this.heap_ops = heap_ops;
	}

	@Override
	public void run() 
	{
		mb_cost_ = this.search_costonly(mb_start_, mb_target_);
	}

	@Override
	public void cleanUp() 
	{
		
	}

}
