package pgraph.anya.astar;

import pgraph.base.BaseVertex;

/**
 * Created by Dindar on 26.10.2014.
 */
public class NullHeuristic<T> implements org.jgrapht.traverse.Heuristic<pgraph.base.BaseVertex> {
    @Override
    public double getValue(BaseVertex s) {
        return 0;
    }
    
    public double getValue(BaseVertex s, BaseVertex t)
    {
    	return 0;
    }
}
