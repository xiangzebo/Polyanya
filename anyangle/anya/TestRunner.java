package pgraph.test;

import pgraph.anya.AnyaExpansionPolicyTest;
import pgraph.anya.IntervalProjectionTest;
import pgraph.grid.BitpackedGrid;
import pgraph.grid.BitpackedGridTest;

public class TestRunner {

	public static void main(String[] args)
	{
		BitpackedGridTest gridtest = new BitpackedGridTest();
		AnyaExpansionPolicyTest expander = new AnyaExpansionPolicyTest();
		IntervalProjectionTest projtest = new IntervalProjectionTest();
		gridtest.run_tests();
		expander.run_tests();
		projtest.run_tests();
	}	
}
