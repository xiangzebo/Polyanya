package org.jgrapht.traverse;

import pgraph.base.BaseVertex;

/**
 * Created with IntelliJ IDEA.
 * User: dindar.oz
 * Date: 3/29/13
 * Time: 9:20 AM
 * To change this template use File | Settings | File Templates.
 */
public class OctileDistanceHeuristic implements Heuristic<BaseVertex> {
    public static final double ROOT_TWO = 1.414213562f;
    private BaseVertex target;

    public OctileDistanceHeuristic(BaseVertex t) 
    {
        target =t;
    }

    @Override
    public double getValue(BaseVertex s) 
    {
    	if(target == null)
    		return 0;
		return getValue(s, target);
    }
    
    public double getValue(BaseVertex s, BaseVertex t)
    {
        if (s== null  )
            return 0;
        
        return getValue(
        		(int)s.pos.x, (int)s.pos.y, 
        		(int)t.pos.x, (int)t.pos.y);
    }
    
    public double getValue(int x1, int y1, int x2, int y2)
    {
        int  dx = Math.abs(x1 - x2);
        int  dy = Math.abs(y1 - y2);
        int min = (dx<dy) ? dx:dy;
        double octileDistance = ((int)Math.abs(dx-dy)) + min*ROOT_TWO;	
        return octileDistance;
    }
}
