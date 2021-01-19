package pgraph.anya;

import pgraph.anya.IntervalProjection;
import pgraph.grid.BitpackedGrid;
import pgraph.test.Test;

public class IntervalProjectionTest implements Test 
{
	private BitpackedGrid grid;
	
	@Override
	public void run_tests() 
	{
		try 
		{
			grid = new BitpackedGrid("maps/local/CSC2F.map");
		} 
		catch (Exception e) 
		{
			e.printStackTrace(System.err);
		}		
		
		assert(grid != null);

		test_project_cone_up();
		test_project_cone_always_has_valid_endpoints();
		test_test();
	}
	
	private void test_project_cone_up()
	{
		// project up; does not project through obstacles
		IntervalProjection proj = new IntervalProjection();
		proj.project(14, 17, 42, 14, 43, grid);
		assert(proj.getValid() == false);		
	}
	
	private void test_project_cone_always_has_valid_endpoints()
	{
		// when projecting we may interpolate points that lie
		// outside the grid. the projection needs to detect such cases
		// and fix them s.t. both endpoints are inside the grid and
		// left <= right
		
		IntervalProjection proj = new IntervalProjection();
		proj.project(74, 82, 4, 59, 5, grid);
		assert(proj.left == 74 && proj.right == 82);
	}
	
	private void test_test()
	{
		try 
		{
			BitpackedGrid grid2 = new BitpackedGrid("maps/random/random512-10-1.map");
			IntervalProjection proj = new IntervalProjection();
			boolean dctest = grid2.get_point_is_double_corner(434, 154);
			boolean obs = grid2.get_cell_is_traversable(434, 154);
			boolean obs2 = grid2.get_cell_is_traversable(433, 154);

			for(int j = 149; j < 160; j++)
			{
				System.out.print(j+" ");
				for(int i = 430; i < 440; i++)
				{
					if(i == 438 && j == 149)
					{
						System.out.print("s");						
					}
					else
					{
						System.out.print(grid2.get_cell_is_traversable(i, j) ? "." : "@");											
					}
						
				}
			System.out.println("");
			}
			proj.project(434+2.0/3.0, 438, 154, 438, 149, grid2);
			
			AnyaExpansionPolicy expander = new AnyaExpansionPolicy("maps/random/random512-10-1.map");
			
			
			//expanding (f=119.57006314291216) searchnode 980546781;root: Point2D.Double[438.0, 149.0] Interval (434.66666666666663, 438.0, 154)
			AnyaNode node = new AnyaNode(null, new AnyaInterval(434.66666666666663, 438, 154), 438, 149);
			expander.expand(node);

			
			node = new AnyaNode(null, new AnyaInterval(434, 434, 155), 438, 149);
			expander.expand(node);


		} 
		catch (Exception e) 
		{
			e.printStackTrace(System.err);
		}		
		
			
	}

}
