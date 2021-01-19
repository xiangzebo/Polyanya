package pgraph.grid;

import pgraph.test.Test;

public class BitpackedGridTest implements Test
{	
	private void test_stuff()
	{
		try
		{
			BitpackedGrid grid = new BitpackedGrid("maps/local/CSC2F.map");
			
			// ************ scan_left tests ***********			
			// these coordinates test stopping due to an obstacle
			int x_value = grid.scan_left(18, 1);
			assert(x_value == 10);
			
			// these coordinates test stopping at a corner 
			x_value = grid.scan_left(21, 41);
			assert(x_value == 18);
			
			// these coordinates test starting at a corner
			x_value = grid.scan_left(18, 41);
			assert(x_value == 10);
			
			// these coordinates test starting just before a corner
			x_value = grid.scan_left(18.5, 41);
			assert(x_value == 18);
			
			// these coordinates test starting just before an obstacle
			x_value = grid.scan_left(10.5, 1);
			assert(x_value == 10);
			
			// these coordinates test starting at an obstacle
			x_value = grid.scan_left(10, 1);
			assert(x_value == 10);

			
			// ************ scan_right tests ***********			
			// these coordinates test stopping at an obstacle
			x_value = grid.scan_right(19, 41);
			assert(x_value == 37);

			// these coordinates test stopping at a corner 
			x_value = grid.scan_right(10, 41);
			assert(x_value == 18);
						
			// these coordinates test starting at a corner
			x_value = grid.scan_right(18, 41);
			assert(x_value == 37);
			
			// these coordinates test starting just before a corner
			x_value = grid.scan_right(17.5, 41);
			assert(x_value == 18);
			
			// these coordinates test starting just before an obstacle
			x_value = grid.scan_right(36.5, 41);
			assert(x_value == 37);
			
			// these coordinates test starting at an obstacle
			x_value = grid.scan_right(37, 41);
			assert(x_value == 37);
			
			// *** scan_cells_right
			x_value = grid.scan_cells_right(5, 6);
			assert(x_value == 9);
			
			x_value = grid.scan_cells_right(9, 6);
			assert(x_value == 9);
			
			// *** scan_cells_left
			x_value = grid.scan_cells_left(8, 6);
			assert(x_value == 4);
			
			x_value = grid.scan_cells_left(5, 6);
			assert(x_value == 4);
			
			x_value = grid.scan_cells_left(4, 6);
			assert(x_value == 4);
			
			
			System.out.println(grid.getClass().getName()+"; all tests done");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public void run_tests()
	{
		test_stuff();
	}
}
