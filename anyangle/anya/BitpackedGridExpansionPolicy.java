package pgraph.grid;

import java.awt.Point;
import org.jgrapht.traverse.Heuristic;
import org.jgrapht.traverse.OctileDistanceHeuristic;

import pgraph.base.ExpansionPolicy;

// BitpackedGridExpansionPolicy.java
// 
// A policy for expanding vertices located at the
// corners of a grid lattice.
//
// @author: dharabor
// @created: 2015-06-04
//

public class BitpackedGridExpansionPolicy implements ExpansionPolicy<Point> 
{
	private BitpackedGrid grid_;
	private HackyHeuristic h;

	private Point s, t;
	private Point[] pool_;
	private Point[] neis_;
	private double[] step_costs_;
	
	private double stepcost_;
	private int index_;
	private int num_neis_;
	
	public class HackyHeuristic implements Heuristic<Point>
	{
		private OctileDistanceHeuristic h;
		protected Point target;
		
		public HackyHeuristic()
		{
			h = new OctileDistanceHeuristic(null);
		}
		
		@Override
		public double getValue(Point n) 
		{
			if(target == null) { return 0; }
			return h.getValue(n.x, n.y, target.x, target.y);
		}

		@Override
		public double getValue(Point n, Point t) 
		{
			return h.getValue(n.x, n.y, t.x, t.y);
		} 		
	}
	
	public BitpackedGridExpansionPolicy(BitpackedGrid grid)
	{
		this.grid_ = grid;
		neis_ = new Point[8];
		step_costs_ = new double[8];
		h = new HackyHeuristic();
		
		pool_ = new Point[grid.get_num_cells()];
		for(int y = 0;  y < grid.get_padded_height(); y++)
		{
			for(int x = 0; x < grid.get_padded_width(); x++)
			{
				int index = compute_id(x, y);
				if(index >= pool_.length)
				{
					System.err.println("wtf?");
				}
				pool_[index] = new Point(x, y);
			}
		}
	}

	// we require that the cells (s.x, s.y) and (t.x, t.y) are 
	// non-obstacle locations; i.e. the instances are valid
	// both for the corner graph that we search and also on
	// the cell-based graph representation of the grid.
	// We make this decision to keep compatibility with 
	// Nathan Sturtevant's benchmarks (http://movingai.com)
	// i.e. every benchmark problem should be solvable and 
	// any that isn't should also fail here
	@Override
	public boolean validate_instance(Point start, Point target) 
	{
		s = start;
		t = target;
		h.target = target;
		boolean result = 
				grid_.get_cell_is_traversable(s.x, s.y) &&
				grid_.get_cell_is_traversable(t.x, t.y);
		return result;
	}
	
	// here we handle the case where the start node is double corner 
	// of the following type (. means traversable, @ means obstacle):
	// .@
	// @.
	private void expand_start_is_valid_double_corner(Point v)
	{
		boolean se = grid_.get_cell_is_traversable(v.x, v.y);
		boolean sw = grid_.get_cell_is_traversable(v.x-1, v.y);
		boolean nw = grid_.get_cell_is_traversable(v.x-1, v.y-1);
		boolean ne = grid_.get_cell_is_traversable(v.x, v.y-1);
	
		// add diagonals
		if(se)
		{
			step_costs_[num_neis_] = OctileDistanceHeuristic.ROOT_TWO;
			neis_[num_neis_++] = pool_[compute_id(v.x+1, v.y+1)];
		}

		// add cardinals  (east and south only)
		if(ne || se)
		{
			step_costs_[num_neis_] = 1.0;
			neis_[num_neis_++] = pool_[compute_id(v.x+1, v.y)];
		}
		if(se || sw)
		{
			step_costs_[num_neis_] = 1.0;
			neis_[num_neis_++] = pool_[compute_id(v.x, v.y+1)];
		}		
	}

	@Override
	public void expand(Point v) 
	{
		num_neis_ = 0;
		index_ = 0;
		if(grid_.get_point_is_double_corner(v.x, v.y)) 
		{
			if(v.x == s.x && v.y == s.y)
			{
				expand_start_is_valid_double_corner(v);
			}
			return; 
		}
		
		boolean se = grid_.get_cell_is_traversable(v.x, v.y);
		boolean sw = grid_.get_cell_is_traversable(v.x-1, v.y);
		boolean nw = grid_.get_cell_is_traversable(v.x-1, v.y-1);
		boolean ne = grid_.get_cell_is_traversable(v.x, v.y-1);

		// add diagonals
		if(ne)
		{
			step_costs_[num_neis_] = OctileDistanceHeuristic.ROOT_TWO;
			neis_[num_neis_++] = pool_[compute_id(v.x+1, v.y-1)];
		}
		if(se)
		{
			step_costs_[num_neis_] = OctileDistanceHeuristic.ROOT_TWO;
			neis_[num_neis_++] = pool_[compute_id(v.x+1, v.y+1)];
		}
		if(nw)
		{
			step_costs_[num_neis_] = OctileDistanceHeuristic.ROOT_TWO;
			neis_[num_neis_++] = pool_[compute_id(v.x-1, v.y-1)];
		}
		if(sw)
		{
			step_costs_[num_neis_] = OctileDistanceHeuristic.ROOT_TWO;
			neis_[num_neis_++] = pool_[compute_id(v.x-1, v.y+1)];
		}

		// add cardinals
		if(ne || se)
		{
			step_costs_[num_neis_] = 1.0;
			neis_[num_neis_++] = pool_[compute_id(v.x+1, v.y)];
		}
		if(nw || sw)
		{
			step_costs_[num_neis_] = 1.0;
			neis_[num_neis_++] = pool_[compute_id(v.x-1, v.y)];
		}
		if(ne || nw)
		{
			step_costs_[num_neis_] = 1.0;
			neis_[num_neis_++] = pool_[compute_id(v.x, v.y-1)];
		}
		if(se || sw)
		{
			step_costs_[num_neis_] = 1.0;
			neis_[num_neis_++] = pool_[compute_id(v.x, v.y+1)];
		}		
	}

	@Override
	public Point next() 
	{
		if(index_ < num_neis_)
		{
			stepcost_ = step_costs_[index_];
			return neis_[index_++];
		}
		stepcost_ = 0;
		return null;
	}

	@Override
	public boolean hasNext() 
	{
		return index_ < num_neis_;
	}

	@Override
	public double step_cost() 
	{
		return stepcost_;
	}
	
	public Point getGridVertex(int x, int y)
	{
		if(x < grid_.get_padded_width() && y < grid_.get_padded_height())
		{
			return pool_[compute_id(x, y)];
		}
		return null;
	}

	@Override
	public Heuristic<Point> heuristic() 
	{
		return h;
	}
	
	private int compute_id(int x, int y)
	{
		return y*grid_.get_padded_width() + x;
	}

	@Override
	public int hashCode(Point v) 
	{
		return compute_id(v.x, v.y);
	}

}
