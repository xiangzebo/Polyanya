#include "QuickBucket.h"

QuickBucket::QuickBucket()
{
	GenerateCases();
	
	// Depending on the parent, we might not have to expand all neighbors
	for (direction c = 0; c <= 6; c+=2)
	{
		parentMask[c] = 255;	// Assume we have to expand all the neighbors
		parentMask[c] &= ~(1<<c);	// Don't expand the parent
		parentMask[c] &= ~(1<<((c+1)%8));	// Don't expand the cells that are cardinally reachable from the parent
		parentMask[c] &= ~(1<<((c+7)%8));
	}
	
	for (direction d = 1; d <= 7; d+=2)
	{
		parentMask[d] = 0;			// Assume we don't have to expand any neighbors
		parentMask[d] |= (1<<((d+4)%8));	// Expand the cell that we would end up if we keep traveling in the same direction
		parentMask[d] |= (1<<((d+3)%8));	// And the cardinal cells that are associated with the diagonal cell
		parentMask[d] |= (1<<((d+5)%8));
	}
	parentMask[8] = 255;	// Special case for the start state which does not have a parent
	
	search = 0;
	maxOpenSize = 0;
	elementExpansions = 0;
	bucketExpansions = 0;
	alreadyExpanded = 0;
}

void QuickBucket::LoadMap(std::vector<bool> &bits, int _width, int _height)
{
	height = _height;
	width = _width;

	theMap = new mapCell*[width];

	int nTraversable = 0;

	for (int x = 0; x < width; x++)
	{
		theMap[x] = new mapCell[height];
		for (int y = 0; y < height; y++)
		{
			theMap[x][y].open = false;
			theMap[x][y].generated = 0;
			theMap[x][y].parent = 0;		
			theMap[x][y].neighbors = 0;	

			if(bits[y*width+x])
			{
				nTraversable++;
				
				// cardinal neighbors
				for (direction c = 0; c <= 6; c += 2)
				{
					if (0 <= x+X[c] && x+X[c] < width && 0 <= y+Y[c] && y+Y[c] < height)
						if (bits[(y + Y[c]) * width + x + X[c]])
							theMap[x][y].neighbors |= (1 << c);
				}
				
				// diagonal neighbors
				for (direction d = 1; d <= 7; d += 2)
				{
					if (0 <= x+X[d] && x+X[d] < width && 0 <= y+Y[d] && y+Y[d] < height)
						if (bits[(y+Y[d])*width+x+X[d]] && bits[y*width+x+X[d]] && bits[(y+Y[d])*width+x])
							theMap[x][y].neighbors |= (1 << d);
				}
			}
		}
	}

#ifdef BUCKET_STATISTICS
	std::cout<<"Height: "<<height<<std::endl;
	std::cout<<"Width: "<<width<<std::endl;
	std::cout<<"Traversable cells: "<<nTraversable<<std::endl;
#endif
}

int QuickBucket::GetBaseCase(xyLoc & l, xyLoc & g)
{
	int mask = 0;
	
	// Assuming goal is the origin..
	
	// Is l to the left or right of the origin
	if (l.x > g.x)
		mask |= 8;
		
	// Is l above or below the origin
	if (l.y > g.y)
		mask |= 16;
	
	int dx = (l.x > g.x)?(l.x-g.x):(g.x-l.x);
	int dy = (l.y > g.y)?(l.y-g.y):(g.y-l.y);

	// Is l above or below the diagonal separator that divides the quadrant l is in
	if (dx > dy)
		mask |= 32;
	
	// Does l lie on one of the cardinal axes
	if (dx == 0 || dy == 0)
		mask |= 64;
	
	// Does l lie on one of the diagonal axes
	if (dx == dy)
		mask |= 128;

	// Is l 1 cardinal move away from a diagonal
	if (dx == (dy+1) || dy == (dx+1))
		mask |= 256;
		
	return mask;
}

void QuickBucket::GenerateCases()
{
	xyLoc o(100,100);
	
	for (int x = o.x-3; x <= o.x+3; x++)
	{
		for (int y = o.y-3; y <= o.y+3; y++)
		{
			xyLoc l(x,y);
			int baseCase = GetBaseCase(l,o);
			
			for (direction d = 0; d < 8; d++)
			{
				xyLoc l2(x+X[d], y+Y[d]);
				cases[baseCase] = GetCase(l,l2,o);
				baseCase ++;
			}
		}
	}
}

void QuickBucket::GenerateBucket(int bucketId)
{
	// If the bucket does not exist, create it
	if (bucketId >= buckets.size())
		buckets.resize(bucketId+1);
		
	while (bucketId >= bucketsInUse)
	{
		buckets[bucketsInUse].contents.clear();
		buckets[bucketsInUse].open = false;
		bucketsInUse++;
	}
}

int QuickBucket::GetCase(xyLoc & l1, xyLoc & l2, xyLoc & g)
{
	abCost cost(DeltaCostCD(l1,l2,g));
	for (int i = 0; i < 6; i ++)
	{
		if (cost.a == A[i] && cost.b == B[i])
		{
			return i;
		}
	}
	return 0;
}

cost QuickBucket::GetPath(xyLoc & s, xyLoc & g, std::vector<xyLoc> & path)
{
	search++;
	theHeap.clear();
	bucketsInUse = 0;	
	
	#ifdef BUCKET_STATISTICS
		int openSize = 1;
	#endif
		
	// Succesor buckets
	int id[6];	// 0: 0, 1: 2C-D, 2: 2D-2C, 3: 
	
	id[0] = GetBucketId(abCost(0,0));	// Get the bucket for the start state
	GenerateBucket(id[0]);
	buckets[id[0]].contents.push_back(bucketElement(s, 8));	// Insert the start state to the bucket, mark its parent as 8 = no parent
	
	buckets[id[0]].open = true;
	AddToOpen(abCost(0,0));					// Insert the bucket into the open list
	
	theMap[s.x][s.y].open = true;	// Set the start state as open
	theMap[s.x][s.y].generated = search;
	
	theMap[g.x][g.y].open = true;	// Also, set the goal state as open, but do not put it in the buckets yet	
	theMap[g.x][g.y].generated = search;
	
	abCost currCost;
	
	while (theMap[g.x][g.y].open && !theHeap.empty())
	{	
		// Select a bucket to expand
		currCost = theHeap[0];
		PopMin();
	
		#ifdef BUCKET_STATISTICS
			bucketExpansions++;			
		#endif 
		
		// Get the bucket and its successor buckets	
		id[0] = GetBucketId(currCost);
		buckets[id[0]].open = false;
	
		for (int i = 1; i < 6; i++)
		{
			id[i] = GetBucketId(abCost(currCost.a + A[i], currCost.b + B[i]));
		}
		GenerateBucket(id[5]+4);	// make sure we generate all the buckets with just one call
				
		// Expand all the contents of the bucket
		while (!buckets[id[0]].contents.empty())
		{
			bucketElement elem = buckets[id[0]].contents.back();
			buckets[id[0]].contents.pop_back();
			#ifdef BUCKET_STATISTICS			
				openSize--;
			#endif									
			if (theMap[elem.loc.x][elem.loc.y].open)
			{
				#ifdef BUCKET_STATISTICS
					elementExpansions++;
				#endif
				
				theMap[elem.loc.x][elem.loc.y].open = false;
				theMap[elem.loc.x][elem.loc.y].parent = elem.parent;
				
				//unsigned char neighbors = theMap[elem.loc.x][elem.loc.y].neighbors;
				unsigned char neighbors = theMap[elem.loc.x][elem.loc.y].neighbors & parentMask[elem.parent];
				
				unsigned char currNeighbor = 1;
				int baseCase = GetBaseCase(elem.loc, g);
						
				for (int d = 0; d < 8; d++)	// look at 8 directions
				{
					if (currNeighbor & neighbors)	// if there is a potential successor in a direction
					{
						xyLoc succ = xyLoc(elem.loc.x + X[d], elem.loc.y + Y[d]);
						
						// generate it if necessary
						if (theMap[succ.x][succ.y].generated != search)
						{
							theMap[succ.x][succ.y].generated = search;
							theMap[succ.x][succ.y].open = true;
						}
						
						// if it was not already expanded
						if (theMap[succ.x][succ.y].open)
						{
							buckets[id[cases[baseCase]]].contents.push_back(bucketElement(succ, (d+4)&7));
							#ifdef BUCKET_STATISTICS
								openSize++;
								if (openSize > maxOpenSize)
									maxOpenSize = openSize;
							#endif
						}
					}
					baseCase++;
					currNeighbor = (currNeighbor << 1);
				}
			}
			#ifdef BUCKET_STATISTICS
				else
					alreadyExpanded++;
			#endif
		}
		
		// Bucket expanded, try to add new buckets to the open list
		for (int i = 1; i < 6; i++)
		{
			if (!buckets[id[i]].open && !buckets[id[i]].contents.empty())	// if it is not already open and it contains at least one element
			{
				buckets[id[i]].open = true;
				AddToOpen(abCost(currCost.a + A[i], currCost.b + B[i]));
			}
		}
		
		#ifdef BUCKET_MEMORY_OPTIMIZATION
			std::vector<bucketElement>().swap(buckets[id[0]].contents);	// Free the memory for that bucket
			buckets[id[0]].contents.reserve(DEFAULT_BUCKET_SIZE);	// Allow it to have 256 elements so that we don't reallocate too often
		#endif
	}

	if (!theMap[g.x][g.y].open)
	{
		path.clear();
		xyLoc loc = g;
		while (loc.x != s.x || loc.y != s.y)
		{
			path.push_back(loc);
			int parent = theMap[loc.x][loc.y].parent;
			loc.x += X[parent];
			loc.y += Y[parent];
		}
		path.push_back(loc);
		std::reverse(path.begin(), path.end());
		return HCost(s,g) + currCost.GetCost();
	}
	
	return 0;//HCost(s,g);
}

void QuickBucket::GetStatistics()
{
	#ifdef BUCKET_STATISTICS
	std::cout<<"Bucket statistics:"<<std::endl;
	std::cout<<"Total number of searches = "<<search<<std::endl;	
	std::cout<<"Maximum number of elements in the bucket = "<<maxOpenSize<<std::endl;	
	std::cout<<"Number of actual expansions = "<<elementExpansions<<std::endl;	
	std::cout<<"Number of states already expanded = "<<alreadyExpanded<<std::endl;	
	std::cout<<"Number of bucket expansions = "<<bucketExpansions<<std::endl;	
	std::cout<<std::endl;
		
	int nBuckets = buckets.capacity();
	int nElements = 0;
	for (int i = 0; i < buckets.size(); i++)
	{
		nElements += buckets[i].contents.capacity();
	}
	
	std::cout<<"Number of allocated bucket slots = "<<nBuckets<<std::endl;		
	std::cout<<"Number of allocated element slots = "<<nElements<<std::endl;	
	//std::cout<<"Number of allocated element slots (1st bucket) = "<<buckets[0].contents.capacity()<<std::endl;	
	std::cout<<"Esimated memory use by buckets = "<<(nBuckets*sizeof(bucket) + nElements*sizeof(bucketElement))/(double)(1024*1024)<<"mb."<<std::endl;	
	#endif
}
