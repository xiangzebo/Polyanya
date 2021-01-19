#include "BL_JPS_SubGoal_Exp.h"
//#include "JumpPointNode.h"
#include <stdio.h>
#include <string.h>
Coordinate nextCoordinate(const Coordinate& c, const int dir)
{
	static char dirMov[] = { 0, -1, 1, -1, 1, 0, 1, 1, 0, 1, -1, 1, -1, 0, -1, -1, 0, 0 };
	return Coordinate(c.x + dirMov[dir * 2], c.y + dirMov[dir * 2 + 1]);
}
inline int implies(const int a, const int b)
{
	return a ? b : 1;
}
inline unsigned char addDirectionToSet(const unsigned char dirs, const int dir)
{
	return dirs | 1 << dir;
}
unsigned char naturalNeighbours(const int dir)
{
	if (dir == NO_DIRECTION)
		return 255;

	unsigned char dirs = 0;
	dirs = addDirectionToSet(dirs, dir);
	if (dir&1) {
		dirs = addDirectionToSet(dirs, (dir + 1) % 8);
		dirs = addDirectionToSet(dirs, (dir + 7) % 8);
	}
	return dirs;
}
void addByX(vector<DiagonalJumpEntry2> & vec, DiagonalJumpEntry2 val)
{
	for (int i = 0; i < vec.size(); i++)
		if (vec[i].from.x > val.from.x)
		{
			vec.insert(vec.begin() + i, val);
			return;
		}
	vec.push_back(val);
}
char getDir2(xyLoc fromLoc, xyLoc toLoc)
{
	int dx = toLoc.x - fromLoc.x;
	int dy = toLoc.y - fromLoc.y;
	if (abs(dx) == abs(dy))
		if (dy>0)
			return dx>0 ? 3 : 5;
		else
			return dx>0 ? 1 : 7;
	if (abs(dx)>abs(dy))
		return dx>0 ? 2 : 6;
	return dy>0 ? 4 : 0;
}
char getDiagDir(const Coordinate& c, const Coordinate& nxt, char dir)
{
	if (!(dir&1))
		return dir;
	int xDif = nxt.x - c.x;
	int yDif = nxt.y - c.y;
	if (abs(xDif) == abs(yDif))
		return dir;
	if (abs(xDif) > abs(yDif))
		return xDif > 0 ? 2 : 6;
	return yDif > 0 ? 4 : 0;
}

short BL_JPS_SUBGOAL_EXP::binarySearchR(const vector<pair<short, short> > & v, short val)
{
	short l = 0, r = v.size() - 1;
	if (r < 0)
		return -1;
	short index = r / 2;
	while (1)
	{
		if (v[index].first >= val && v[index].second <= val)
		{
			if (v[index].first == val && index + 1 < v.size() && v[index + 1].second == val)
				return v[index + 1].second;
			return  v[index].second;
		}
		if (v[index].second > val)
		{
			r = index - 1;
			if (r <0 || r<l)
				return -1;
		}
		else
		{
			l = index + 1;
			if (l > v.size() - 1 || r<l)
				return -1;
		}

		index = l + (r - l) / 2;

	}
	return -1;
}
short BL_JPS_SUBGOAL_EXP::binarySearchL(const vector<pair<short, short> > & v, short val)
{
	short l = 0, r = v.size() - 1;
	if (r < 0)
		return -1;
	short index = r / 2;
	while (1)
	{
		if (v[index].first <= val && v[index].second >= val)
		{
			if (v[index].first == val && index - 1 >-1 && v[index - 1].second == val)
				return v[index - 1].second;
			return  v[index].second;
		}
		if (v[index].second > val)
		{
			r = index - 1;
			if (r <0 || r<l)
				return -1;
		}
		else
		{
			l = index + 1;
			if (l > v.size() - 1 || r<l)
				return -1;
		}

		index = l + (r - l) / 2;

	}
	return -1;
}
bool BL_JPS_SUBGOAL_EXP::getJumpPointNew(Coordinate s, const char direction, Coordinate & jp)
{
	s = nextCoordinate(s, direction);

	if (!IsTraversable(s))
		return false;
	//		bool ret = false;

	int index;
	switch (direction)
	{

	case 0://North
		index = binarySearchR(jumpLookup[0][s.x], s.y);
		/*if (s.x == eX&& s.y >= eY &&s.y >= eYSpace[0] && s.y <= eYSpace[1])
		{
			jp = Coordinate(eX, eY);
			return true;
		}*/
		if (index != -1)
			jp = Coordinate(s.x, index);
		return index>-1;
	case 2://EAST
		index = binarySearchL(jumpLookup[1][s.y], s.x);
		/*if (s.y == eY&& s.x <= eX &&s.x <= eXSpace[1] && s.x >= eXSpace[0])
		{
			jp = Coordinate(eX, eY);
			return true;
		}*/
		if (index != -1)
			jp = Coordinate(index, s.y);
		return index>-1;
	case 4://SOUTH
		index = binarySearchL(jumpLookup[2][s.x], s.y);

		/*if (s.x == eX&& s.y <= eY &&s.y <= eYSpace[1] && s.y >= eYSpace[0])
		{
			jp = Coordinate(eX, eY);
			return true;
		}*/
		if (index != -1)
			jp = Coordinate(s.x, index);
		return index>-1;
	case 6://WEST
		index = binarySearchR(jumpLookup[3][s.y], s.x);
		/*if (s.y == eY&& s.x >= eX &&s.x >= eXSpace[0] && s.x <= eXSpace[1])
		{
			jp = Coordinate(eX, eY);
			return true;
		}*/
		if (index != -1)
			jp = Coordinate(index, s.y);
		return index>-1;
	}
	return false;
}

int BL_JPS_SUBGOAL_EXP::jumpNew(const Coordinate &c, const char dir, std::vector<subGoalIdDir> & subgoals)
{
	Coordinate nc =  nextCoordinate(c, dir);

	Coordinate offset(0, 0);
	offset = nextCoordinate(offset, dir);
	while (1)
	{
		bool b = true;
#ifdef DIAG_UNBLOCKED
		b = ((dir & 1) == 0) || ((dir & 1) && (IsTraversable(Coordinate(nc.x - offset.x, nc.y)) && IsTraversable(Coordinate(nc.x, nc.y - offset.y))));
#endif
		if (!IsTraversable(nc) || !b)
			return -1;
		int index = ToMapLoc(nc);
		if (forcedNeighbours(nc, dir))
		{
			subgoals.push_back(subGoalIdDir(ToSubgoalId(index), dir, dir, 0xff));
			return index;
		}
		if (dir&1)
		{
			Coordinate newP(-1, -1);
			if (getJumpPointNew(nc, (dir + 7) % 8, newP))
				subgoals.push_back(subGoalIdDir(ToSubgoalId(ToMapLoc(newP)), dir, (dir + 7) % 8,0xff));
			if (getJumpPointNew(nc, (dir + 1) % 8, newP))
				subgoals.push_back(subGoalIdDir(ToSubgoalId(ToMapLoc(newP)), dir, (dir + 1) % 8, 0xff));
		}
		else
		{
			Coordinate newP(-1, -1);
			if (getJumpPointNew(c, dir, newP))
				subgoals.push_back(subGoalIdDir(ToSubgoalId(ToMapLoc(newP)), dir,dir,0xff ));

			return 1;
		}
		nc.add(offset);
	}
	return 0;
}

Coordinate BL_JPS_SUBGOAL_EXP::diagonalDirection(Coordinate c, int dir)
{
#ifdef DIAG_UNBLOCKED
	while (IsTraversable(ToMapLoc(c)) && IsTraversable(ToMapLoc(Coordinate(nextCoordinate(c, dir).x, c.y))) && IsTraversable(ToMapLoc(Coordinate(c.x, nextCoordinate(c, dir).y))))
#else
	while (isPassable(c))
#endif
		c = nextCoordinate(c, dir);
	if (!IsTraversable(ToMapLoc(c)))
		c = nextCoordinate(c, (dir + 4) & 7);
	return c;

}
int BL_JPS_SUBGOAL_EXP::jump(const Coordinate &c, const char dir)
{
	Coordinate nc = nextCoordinate(c, dir);
	if (nc.x<0 || nc.y<0||nc.x>=width-1||nc.y>=height-1 || !IsTraversable(nc))
		return -1;

	int index = ToMapLoc(nc);
	unsigned char dirs;
	if ( (dirs = forcedNeighbours(nc, dir)))
		return index;

	if (dir&1)
	{
		int next = jump(nc, (dir + 7) % 8);
		if (next >= 0)
			return index;
		next = jump(nc, (dir + 1) % 8);
		if (next >= 0)
			return index;
	}

	return jump(nc, dir);
}
void insertionOrdered(vector<pair<short, short> > & vec, pair<short, short> v)
{
	for (int i = 0; i < vec.size(); i++)
		if (vec[i].second > v.second)
		{
			vec.insert(vec.begin() + i, v);
			return;
		}
	vec.push_back(v);
}

/*void BL_JPS_SUBGOAL_EXP::preProcessSubGoalGraph()
{

	vector<JumpPointNode*> allNodes;
	for (int i = 0; i < nSubgoals; i++)
		allNodes.push_back(new JumpPointNode(location[i].x, location[i].y, i));

	for (int i = 0; i < nSubgoals; i++)
	{
		for (int dir = 0; dir < 8; dir++)
			allNodes[i]->addNatForcedNeighbours(naturalNeighbours(dir) | forcedNeighbours(allNodes[i]->pos, dir), dir);
		std::vector<subGoalIdDir> edges;
		for (int dir = 0; dir < 8; dir++)
		{
			if (IsTraversable(nextCoordinate(allNodes[i]->pos, (dir + 4) & 7)) || (allNodes[i]->neighbours[(dir + 1) & 7] & (1 << dir)) || (allNodes[i]->neighbours[(dir + 7) & 7] & (1 << dir)))

				//if (IsTraversable(nextCoordinate(allNodes[i]->pos, (dir + 4) & 7)))
				jump(allNodes[i]->pos, dir, dir, edges);
			for (int id = 0; id < edges.size(); id++)
			{
				allNodes[i]->addToDirs(allNodes[edges[id].id], edges[id].dir);
				int newDir = getDiagDir(allNodes[i]->pos, allNodes[edges[id].id]->pos, dir);

				allNodes[edges[id].id]->addToIncDirs(pair<JumpPointNode*, char>(allNodes[i], dir), newDir);
			}

			edges.clear();
		}

	}


	vector<Coordinate> solution;
	for (int i = 0; i < allNodes.size(); i++)
		for (int dirI = 0; dirI < 8; dirI++)
			for (int ii = 0; ii < allNodes[i]->getDirSize(dirI); ii++)
			{
				int newDir = getDiagDir(allNodes[i]->pos, allNodes[i]->getDirItem(dirI, ii)->pos, dirI);
				allNodes[i]->getDirItem(dirI, ii)->addToIncDirs(pair<JumpPointNode*, char>(allNodes[i], dirI), newDir);
			}
	for (int i = 0; i < allNodes.size(); i++)
	{
		for (int dir = 0; dir < 8; dir++)
		{
			bool found = 0;
			Coordinate nc = allNodes[i]->pos;
			{
				char dirs = naturalNeighbours(dir) | forcedNeighbours(allNodes[i]->pos, dir);

				for (int dirI = 0; dirI < 8 && !found; dirI++)
				{
					if (dirs&(1 << dirI))
						for (int ii = 0; ii < allNodes[i]->getDirSize(dirI) && !found; ii++)
						{
							//int newDir = getDiagDir(allNodes[i]->pos, allNodes[i]->incDirs[dirI][ii]->pos, dir);
							char dirs2 = 0xff - ((1 << (dirI + 4) & 7) | (1 << ((dirI + 1 + 4) & 7)) | (1 << ((dirI - 1 + 4) & 7)));// naturalNeighbours(newDir) | forcedNeighbours(allNodes[i]->dirs[dirI][ii]->pos, newDir);
							for (int dirII = 0; dirII < 8 && !found; dirII++)
							{
								//char dirs2B = naturalNeighbours((dirII + 4) & 7) | forcedNeighbours(allNodes[i]->pos, (dirII + 4) & 7);

								if (dirs2&(1 << dirII))// && dirs2B&(1 << dirII))
									for (int iii = 0; iii < allNodes[i]->incDirs[dirII].size() && !found; iii++)
									{
										solution.clear();
										JumpPointNode *b = allNodes[i];
										JumpPointNode *c = allNodes[i]->getDirItem(dirI, ii);// dirs[dirI][ii];
										JumpPointNode *a = allNodes[i]->incDirs[dirII][iii].first;
										bool sameDist = abs(Node::estimateDistance(a->pos, c->pos) - Node::estimateDistance(a->pos, b->pos) - Node::estimateDistance(b->pos, c->pos)) < 0.1;
										if (!sameDist)
											found = true;
										if (!directSolution(c->pos.x, c->pos.y, a->pos.x, a->pos.y, solution))
											found = true;
									}
							}
						}
				}
				if (!found)
				{
					JumpPointNode *a = allNodes[i];

					if (allNodes[i]->getDirSize(dir) == 0)
						allNodes[i]->addEdgeNode(dir);
					allNodes[i]->pruneEdge(dir);
					//char dirs = naturalNeighbours(dir) | forcedNeighbours(allNodes[i]->pos, dir);
					char dirs = ((1 << ((dir)& 7)) | (1 << ((dir + 1) & 7)) | (1 << ((dir + 7) & 7)));
					for (int dirII = 0; dirII < 8; dirII++)
						if (dirs&(1 << dirII))
						{
							for (int iii = 0; iii < allNodes[i]->incDirs[dirII].size(); iii++)
								for (int iv = 0; iv < allNodes[i]->getDirSize(dir); iv++)
								{
									char oldDir = allNodes[i]->incDirs[dirII][iii].second;
									//if (oldDir == dirII)
									{
										JumpPointNode *b = allNodes[i]->getDirItem(dir, iv);// dirs[dir][iv];
										pair<JumpPointNode*, char> c = allNodes[i]->incDirs[dirII][iii];
										c.first->addToDirs(b, oldDir);
										b->addToIncDirs(c, dirII);

										//if (dir == 3 && c->pos.y > b->pos.y)
										//	int aa = 1;

									}
								}

							if (dirII & 1)
							{
								char dirIIOff = (dirII + 1) & 7;

								for (int iii = 0; iii < allNodes[i]->incDirs[dirIIOff].size(); iii++)
									for (int iv = 0; iv < allNodes[i]->getDirSize(dir); iv++)
									{
										char oldDir = allNodes[i]->incDirs[dirIIOff][iii].second;

										if (oldDir == dirII)
										{
											JumpPointNode *b = allNodes[i]->getDirItem(dir, iv);
											pair<JumpPointNode*, char> c = allNodes[i]->incDirs[dirIIOff][iii];
											c.first->addToDirs(b, oldDir);
											b->addToIncDirs(c, dirII);
										}
									}
								dirIIOff = (dirII + 7) & 7;

								for (int iii = 0; iii < allNodes[i]->incDirs[dirIIOff].size(); iii++)
									for (int iv = 0; iv < allNodes[i]->getDirSize(dir); iv++)
									{
										char oldDir = allNodes[i]->incDirs[dirIIOff][iii].second;

										if (oldDir == dirII)
										{
											JumpPointNode *b = allNodes[i]->getDirItem(dir, iv);
											pair<JumpPointNode*, char> c = allNodes[i]->incDirs[dirIIOff][iii];
											c.first->addToDirs(b, oldDir);
											b->addToIncDirs(c, dirII);
										}
									}

							}

						}
				}

			}


		}
	}
	//	printf("edge nodes:%d %d : %4.2f %%\n", numUselessNodes, allNodes.size()*8,numUselessNodes / ((float)allNodes.size()*8) * 100);
	for (int i = 0; i < allNodes.size(); i++)
	{
		Coordinate nc = allNodes[i]->pos;

		for (int dirI = 0; dirI < 8; dirI++)
			for (int ii = 0; ii < allNodes[i]->getDirSize(dirI); ii++)
			{
				solution.clear();
				JumpPointNode * a = allNodes[i]->getDirItem(dirI, ii);
				//bool hasDirectSolution = directSolution(allNodes[i]->pos.x, allNodes[i]->pos.y, a->pos.x, a->pos.y, solution);
				bool firstConnection = false;
				directSolution(allNodes[i]->pos, a->pos, firstConnection);
				char d2 = getDir2(allNodes[i]->pos, a->pos);
				bool b1 = (firstConnection && (!a->isPrunedEdge(dirI) || a->isEdgeNode(dirI)));//|| allNodes[i]->isEdgeNode(dirI)
				bool b2 = (!firstConnection && (!a->isPrunedEdge(d2) || a->isEdgeNode(d2)));//|| allNodes[i]->dirs[dirI][ii]->isEdgeNode(d2)

				if (b1 || b2)// || (!firstConnection&&hasDirectSolution))
				{
					if (firstConnection)
						allNodes[i]->addHighGoal(pair<JumpPointNode*, char>(a, getDiagDir(allNodes[i]->pos, a->pos, getDir(allNodes[i]->pos, a->pos))), dirI);
					//allNodes[i]->highGoals[dirI].push_back(pair<JumpPointNode*, char>(a, getDiagDir(allNodes[i]->pos, a->pos, getDir(allNodes[i]->pos, a->pos))));
					else
						allNodes[i]->addHighGoal(pair<JumpPointNode*, char>(a, d2), dirI);
					//allNodes[i]->highGoals[dirI].push_back(pair<JumpPointNode*, char>(a, d2));

				}
			}
	}
	for (int i = 0; i < allNodes.size(); i++)
		for (int dirI = 0; dirI < 8; dirI++)
			for (int ii = 0; ii < allNodes[i]->incDirs[dirI].size(); ii++)
			{
				if (allNodes[i]->incDirs[dirI][ii].second != dirI)
				{
					allNodes[i]->incDirs[allNodes[i]->incDirs[dirI][ii].second].push_back(allNodes[i]->incDirs[dirI][ii]);
					allNodes[i]->incDirs[dirI].erase(allNodes[i]->incDirs[dirI].begin() + ii);
					ii--;
				}
			}




	vector<JumpPointNode*>& allNodes2 = allNodes;



	for (int i = 0; i < allNodes.size(); i++)
	{
		edgeVector[i].clear();


		for (int dir = 0; dir < 8; dir++)
			for (int j = 0; j < allNodes2[i]->getNumHighGoals(dir); j++)
				edgeVector[i].push_back(subGoalIdDir(allNodes2[i]->getHighGoal(dir, j).first->id, dir, allNodes2[i]->getHighGoal(dir, j).second,0xff));
		incomingConnectionsVector[i].clear();

		for (int dir = 0; dir < 8; dir++)
			for (int j = 0; j < allNodes2[i]->incDirs[dir].size(); j++)
				incomingConnectionsVector[i].push_back(subGoalIdDir(allNodes2[i]->incDirs[dir][j].first->id, dir, allNodes2[i]->incDirs[dir][j].second,0xff));

		pruned[i] = allNodes[i]->pruned;
		//for (int j = 0; j < allNodes2[i]->dirs.size(); j++)
		//	allNodes2[i].dirs

	}
	for (int i = 0; i < allNodes.size(); i++)
		delete allNodes[i];
}*/
void BL_JPS_SUBGOAL_EXP::getDiagCoordinate2(const Coordinate& nxt, vector<Coordinate> & solution)
{
	int xDif = solution.back().x - nxt.x;
	int yDif = solution.back().y - nxt.y;
	if (!(abs(xDif) == abs(yDif) || xDif == 0 || yDif == 0))
	{
		if (abs(xDif)>abs(yDif))
		{
			//int diagDif = abs(xDif) - abs(yDif);

			solution.push_back(Coordinate(nxt.x + mag(xDif)*abs(yDif), nxt.y + yDif));

			//if (!pathSegmentValid(*(solution.end() - 2), solution.back()) || !pathSegmentValid(solution.back(), nxt))
			//	solution.back() = Coordinate(nxt.x - mag(xDif)*(abs(yDif) - abs(xDif)), nxt.y);
		}
		else
		{

			solution.push_back(Coordinate(nxt.x + xDif, nxt.y + mag(yDif)*abs(xDif)));
			//if (!pathSegmentValid(*(solution.end() - 2), solution.back()) || !pathSegmentValid(solution.back(), nxt))
			//	solution.back() = Coordinate(nxt.x, nxt.y + mag(yDif)*(abs(yDif) - abs(xDif)));

		}

	}
	solution.push_back(nxt);
}
bool BL_JPS_SUBGOAL_EXP::directSolution(Coordinate s, Coordinate e, bool &firstConnection)
{
	short sX = s.x;
	short sY = s.y;
	short eX = e.x;
	short eY = e.y;
	firstConnection = false;

	if (sY == eY)
	{
		if (isSpaceIdX(getSpaceIdX(sX, sY), eX, eY))
		{
			firstConnection = true;
			return true;
		}
	}
	else if (sX == eX)
	{
		if (isSpaceIdY(getSpaceIdY(sX, sY), eX, eY))
		{
			firstConnection = true;
			return true;
		}
	}
	else
	{
		bool isDiagOnly = (abs(sX - eX) - abs(sY - eY)) == 0;

		int diagMovements = min(abs(sX - eX), abs(sY - eY)) - 1;//we -1 as we don't need to check the original or destination points as they are checked earlier
		int mx = sX - eX<0 ? -1 : 1;
		int my = sY - eY<0 ? -1 : 1;
		Coordinate offset(sX - eX<0 ? 1 : -1, sY - eY<0 ? 1 : -1);
		Coordinate check(sX, sY);
		bool bPass = true;
		if (!isDiagOnly)
		{
			Coordinate check2(sX + offset.x*(diagMovements + 1), sY + offset.y*(diagMovements + 1));
			if (abs(sX - eX) < abs(sY - eY))
			{
				if (!isSpaceIdY(getSpaceIdY(check2.x, check2.y), eX, eY))
					bPass = false;
			}
			else
				if (!isSpaceIdX(getSpaceIdX(check2.x, check2.y), eX, eY))
					bPass = false;
		}
		if (bPass)
		{
			while (diagMovements)
			{
				diagMovements--;
				//	check.add(offset);
				bool b = true;
#ifdef DIAG_UNBLOCKED
				b = (IsTraversable(Coordinate(check.x + offset.x, check.y)) && IsTraversable(Coordinate(check.x, check.y + offset.y)));
#endif

				check.add(offset);
				if (!IsTraversable(check) || !b)
				{
					bPass = false;
					diagMovements = 0;
					break;
				}

#ifdef DIAG_UNBLOCKED
				if (bPass && min(abs(sX - eX), abs(sY - eY)) != 0)
					bPass = (IsTraversable(Coordinate(check.x + offset.x, check.y)) && IsTraversable(Coordinate(check.x, check.y + offset.y)));
#endif
			}
			if (bPass)
				if (isDiagOnly)//only diagonal movement
				{
					firstConnection = false;
					return true;
				}
				else //diagonal movement and then horiz/vertic
				{
					firstConnection = true;
					return true;
				}
		}


	}
	return false;
}
void BL_JPS_SUBGOAL_EXP::buildDiagEndJumps()
{
	for (int i = 0; i < 4; i++)
		diagonalJumps[i].resize(width + height);

	for (int i = 0; i < nSubgoals; i++)
	{
		Coordinate c = location[i];
		char availDirs = 0;
		for (int j = 0; j < edgeVector[i].size(); j++)
			availDirs|=1 << edgeVector[i][j].dir;

		for (int dir = 1; dir < 8; dir += 2)
		{

			//if (availDirs&(1<<dir))
			//if (allNodes[i]->getDirSize(dir) || (allNodes[i]->neighbours[(dir + 1) & 7] & (1 << dir)) || (allNodes[i]->neighbours[(dir + 7) & 7] & (1 << dir)))
			{
				Coordinate from(c);
				Coordinate to(diagonalDirection(c, dir));

				if (!(from.x == to.x &&  from.y == to.y))
				{
					if (dir == 1 || dir == 5)
						addByX(diagonalJumps[(dir - 1) / 2][from.x + from.y], DiagonalJumpEntry2(from, to, i));
					//diagonalJumps[(dir - 1) / 2][from.x + from.y].push_back(DiagonalJumpEntry2(from, to, allNodes[i]));
					else
					{
						int ix = from.x + (height - from.y - 1);
						addByX(diagonalJumps[(dir - 1) / 2][ix], DiagonalJumpEntry2(from, to, i));
					}

					//diagonalJumps[(dir - 1) / 2][from.x + (gridHeight-from.y-1)].push_back(DiagonalJumpEntry2(from, to, allNodes[i]));
				}

			}
		}
	}


	for (int y = 0; y < height-2; y++)
	{
		jumpLookup[1].push_back(vector<pair<short, short> >());
		jumpLookup[3].push_back(vector<pair<short, short> >());
		vector<pair<short, short> > & vec = jumpLookup[1].back();
		for (int xId = 0; xId < xBoundaryPoints[y].size(); xId += 2)
		{
			int x = xBoundaryPoints[y][xId];
			int index;
			do
			{
				index = jump(Coordinate(x, y), 2);
				if (index != -1)
				{
					int newX = ToXYLoc(index).x;
					vec.push_back(pair<short, short>(x, newX));
					x = newX;
				}
			} while (index != -1);
		}
		vector<pair<short, short> > & vecB = jumpLookup[3].back();
		for (int xId = 1; xId < xBoundaryPoints[y].size(); xId += 2)
		{
			int x = xBoundaryPoints[y][xId] - 1;
			int index;
			do
			{
				index = jump(Coordinate(x, y), 6);
				if (index != -1)
				{
					int newX = ToXYLoc(index).x;
					int yA = ToXYLoc(index).y;
					insertionOrdered(vecB, pair<short, short>(x, newX));
					x = newX;
				}
			} while (index != -1);
		}
	}
	for (int x = 0; x < width-2; x++)
	{
		jumpLookup[2].push_back(vector<pair<short, short> >());
		jumpLookup[0].push_back(vector<pair<short, short> >());
		vector<pair<short, short> > & vec = jumpLookup[2].back();
		for (int yId = 0; yId < yBoundaryPoints[x].size(); yId += 2)
		{
			int y = yBoundaryPoints[x][yId];
			int index;
			do
			{
				index = jump(Coordinate(x, y), 4);
				if (index != -1)
				{
					int newY = ToXYLoc(index).y;
					vec.push_back(pair<short, short>(y, newY));
					y = newY;
				}
			} while (index != -1);
		}
		vector<pair<short, short> > & vecB = jumpLookup[0].back();
		for (int yId = 1; yId < yBoundaryPoints[x].size(); yId += 2)
		{
			int y = yBoundaryPoints[x][yId] - 1;
			int index;
			do
			{
				index = jump(Coordinate(x, y), 0);
				if (index != -1)
				{
					int newY = ToXYLoc(index).y;
					insertionOrdered(vecB, pair<short, short>(y, newY));
					y = newY;
				}
			} while (index != -1);
		}

	}


}

int BL_JPS_SUBGOAL_EXP::binarySearchL(const vector<DiagonalJumpEntry2> & v, short val)
{
	short l = 0, r = v.size() - 1;
	if (r < 0)
		return 0;
	short index = r / 2;
	while (1)
	{
		if ((index == 0 || (index>0 && v[index - 1].from.x < val)) && v[index].from.x >= val)
			return  index;

		if (v[index].from.x >= val)
		{
			r = index - 1;
			if (r <0 || r<l)
				return v.size();
		}
		else
		{
			l = index + 1;
			if (l > v.size() - 1 || r<l)
				return v.size();
		}

		index = l + (r - l) / 2;

	}
	return v.size();
}
void addEndNode(vector<subGoalIdDir> &endNodes, subgoalId  jp)
{
	for (int i = 0; i < endNodes.size(); i++)
		if (endNodes[i].id == jp)
		{
			return;
		}
	endNodes.push_back(subGoalIdDir(jp, 8, 8, 0xFF));
}
void BL_JPS_SUBGOAL_EXP::getAllEndNodes(short eX, short eY, std::vector<subGoalIdDir> &endNodes)
{
	//int indexA = gridIndex(Coordinate(eX, eY));

	int idX = getSpaceIdX(eX, eY);
	Coordinate horiz(xBoundaryPoints[eY][idX], xBoundaryPoints[eY][idX + 1] - 1);
	int idY = getSpaceIdY(eX, eY);
	Coordinate vert(yBoundaryPoints[eX][idY], yBoundaryPoints[eX][idY + 1] - 1);

	//Get NE bridged points
	{
		int miT = horiz.x + eY;
		int maT = vert.y + eX;
		for (int i = miT; i <= maT; i++)
		{
			bool broken = false;
			for (int ii = 0; ii < diagonalJumps[0][i].size(); ii++)
			{
				DiagonalJumpEntry2 &diagPoint = diagonalJumps[0][i][ii];
				if (diagPoint.from.y >= eY  && diagPoint.from.x <= eX)
				{

					int xIntercept = diagPoint.from.x + (diagPoint.from.y - eY);
					int yIntercept = diagPoint.from.y + (diagPoint.from.x - eX);

					if (xIntercept <= diagPoint.to.x && xIntercept >= horiz.x  && xIntercept <= eX)
						addEndNode(endNodes, diagPoint.id);
					//endNodes.push_back(diagPoint.jp);
					else if (yIntercept >= diagPoint.to.y && yIntercept <= vert.y && yIntercept >= eY)
						addEndNode(endNodes, diagPoint.id);

					//endNodes.push_back(diagPoint.jp);
				}
				else if (diagPoint.from.x > eX)
					break;
			}
		}
	}
		{
			int miT = horiz.y + (height - 1 - eY);
			int maT = (height - 1 - vert.y) + eX;

			//NW
			for (int i = maT; i <= miT; i++)
				for (int ii = binarySearchL(diagonalJumps[3][i], eX); ii < diagonalJumps[3][i].size(); ii++)
				{
					DiagonalJumpEntry2 &diagPoint = diagonalJumps[3][i][ii];
					if (diagPoint.from.y >= eY && diagPoint.from.x >= eX)
					{

						int xIntercept = diagPoint.from.x - (diagPoint.from.y - eY);
						int yIntercept = diagPoint.from.y - (diagPoint.from.x - eX);
						if (xIntercept >= diagPoint.to.x && xIntercept <= horiz.y  && xIntercept >= eX)
							addEndNode(endNodes, diagPoint.id);

						//endNodes.push_back(diagPoint.jp);
						else if (yIntercept >= diagPoint.to.y && yIntercept <= vert.y && yIntercept >= eY)
							addEndNode(endNodes, diagPoint.id);

						//	endNodes.push_back(diagPoint.jp);
					}

				}
		}
		{
			int miT = horiz.y + eY;
			int maT = vert.x + eX;

			//SW
			for (int i = maT; i <= miT; i++)
				for (int ii = binarySearchL(diagonalJumps[2][i], eX); ii < diagonalJumps[2][i].size(); ii++)
				{
					DiagonalJumpEntry2 &diagPoint = diagonalJumps[2][i][ii];
					if (diagPoint.from.y <= eY && diagPoint.from.x >= eX)
					{
						int xIntercept = diagPoint.from.x + (diagPoint.from.y - eY);
						int yIntercept = diagPoint.from.y + (diagPoint.from.x - eX);
						if (xIntercept >= diagPoint.to.x && xIntercept <= horiz.y  && xIntercept >= eX)
							addEndNode(endNodes, diagPoint.id);

						//endNodes.push_back(diagPoint.jp);
						else if (yIntercept <= diagPoint.to.y && yIntercept >= vert.x && yIntercept <= eY)
							addEndNode(endNodes, diagPoint.id);

						//endNodes.push_back(diagPoint.jp);
					}
				}
		}
		{
			int miT = horiz.x + (height - 1 - eY);
			int maT = (height - 1 - vert.x) + eX;
			//SE
			for (int i = miT; i <= maT; i++)
				for (int ii = 0; ii < diagonalJumps[1][i].size(); ii++)
				{
					DiagonalJumpEntry2 &diagPoint = diagonalJumps[1][i][ii];
					if (diagPoint.from.y <= eY  && diagPoint.from.x <= eX)
					{
						int xIntercept = diagPoint.from.x - (diagPoint.from.y - eY);
						int yIntercept = diagPoint.from.y - (diagPoint.from.x - eX);

						if (xIntercept <= diagPoint.to.x && xIntercept >= horiz.x  && xIntercept <= eX)
							addEndNode(endNodes, diagPoint.id);

						//	endNodes.push_back(diagPoint.jp);
						else if (yIntercept <= diagPoint.to.y && yIntercept >= vert.x && yIntercept <= eY)
							addEndNode(endNodes, diagPoint.id);

						//endNodes.push_back(diagPoint.jp);
					}
					else if (diagPoint.from.x > eX)
						break;
				}
		}
#ifdef DIAG_UNBLOCKED

		//unordered_map<pair<short, short>, JumpPointNode*, CoordinateHash>::iterator it = jumpPoints.end();
		//if (horiz.x == eX || eX == horiz.y)
		{
			//Test north
			int pY = eY;
			while (vert.x < pY)
			{
				pY--;
				if (IsTraversable(ToMapLoc(Coordinate(eX - 1, pY))) || IsTraversable(ToMapLoc(Coordinate(eX + 1, pY))))
				{
					//it = jumpPoints.find(pair<short, short>(eX, pY));
					//if (it != jumpPoints.end())
					mapLoc loc = ToMapLoc(Coordinate(eX, pY));
					if (IsSubgoal(loc))
						addEndNode(endNodes, ToSubgoalId(loc));
					break;
				}
			}

			//Test south
			pY = eY;
			while (vert.y > pY)
			{
				pY++;
				if (IsTraversable(ToMapLoc(Coordinate(eX - 1, pY))) || IsTraversable(ToMapLoc(Coordinate(eX + 1, pY))))
				{
					mapLoc loc = ToMapLoc(Coordinate(eX, pY));
					if (IsSubgoal(loc))
						addEndNode(endNodes, ToSubgoalId(loc));

					break;
				}
			}
		}
		//if (vert.x == eY || eY == vert.y)
		{
			//Test East
			int pX = eX;
			while (horiz.x < pX)
			{
				pX--;
				if (IsTraversable(ToMapLoc(Coordinate(pX, eY - 1))) || IsTraversable(ToMapLoc(Coordinate(pX, eY + 1))))
				//if (isPassable(Coordinate(pX, eY - 1)) || isPassable(Coordinate(pX, eY + 1)))
				{
					/*it = jumpPoints.find(pair<short, short>(pX, eY));
					if (it != jumpPoints.end())
						addEndNode(endNodes, it->second);*/
					//addEndNode(endNodes, ToSubgoalId(ToMapLoc(Coordinate(pX, eY))));
					mapLoc loc = ToMapLoc(Coordinate(pX, eY));
					if (IsSubgoal(loc))
						addEndNode(endNodes, ToSubgoalId(loc));
					break;
				}
			}

			//Test West
			pX = eX;
			while (horiz.y > pX)
			{
				pX++;
				//if (isPassable(Coordinate(pX, eY - 1)) || isPassable(Coordinate(pX, eY + 1)))
				if (IsTraversable(ToMapLoc(Coordinate(pX, eY - 1))) || IsTraversable(ToMapLoc(Coordinate(pX, eY + 1))))

				{
					/*it = jumpPoints.find(pair<short, short>(pX, eY));
					if (it != jumpPoints.end())
						addEndNode(endNodes, it->second);*/
					//addEndNode(endNodes, ToSubgoalId(ToMapLoc(Coordinate(pX, eY))));
					mapLoc loc = ToMapLoc(Coordinate(pX, eY));
					if (IsSubgoal(loc))
						addEndNode(endNodes, ToSubgoalId(loc));
					break;
				}

			}
		}

		//diagonalJumps
#endif

}


void BL_JPS_SUBGOAL_EXP::InitializeValues()
{
	search = MAX_SEARCH;
	finalized = false;

	traversable = NULL;
	subgoal = NULL;
	cellInfo = NULL;

	location = NULL;
	pruned = NULL;
	neighbors = NULL;
	nNeighbors = NULL;
	localNeighbors = NULL;
	nLocalNeighbors = NULL;
	hasExtraEdge = NULL;

	generated = NULL;
	gCost = NULL;
	parent = NULL;
	open = NULL;

	dist = NULL;
}

void BL_JPS_SUBGOAL_EXP::preProcessGrid()
{
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif

	useSubgoals = true;	// Use the subgoal graph at all? If set to false, the program defaults to buckets

#ifndef USE_SUBGOALS
	useSubgoals = false;
#endif
	preprocessBoundaryLookupTables();
	IdentifySubgoals();

	if (!useSubgoals)	// IdentifySubgoals might decide that buckets are preferable
	{
	//	SaveGraph(filename);
		return;
	}

	ComputeClearances();
	LinkSubgoals();
	buildDiagEndJumps();
#ifdef SG_STATISTICS
	t.EndTimer();
	std::cout << "Graph built in " << t.GetElapsedTime() * 1000 << "ms" << std::endl;
	std::cout << "--------------------" << std::endl;
	std::cout << "Memory limit: " << memoryLimit / (double)(1024 * 1024) << " MiB." << std::endl;
#endif

	// -1 : undecided, 0 : no, 1 : yes
	usePairwise = -1;	// Precompute and store pairwise distance matrix?
	int useTwoLevel = -1;	// Use the Simple subgoal graph or the Two-level subgoal graph?



#ifdef KEEP_LOCAL_EDGES
	keepLocalEdges = true;
#else
	keepLocalEdges = false;
#endif


#ifndef PAIRWISE_DISTANCES
	usePairwise = 0;
#endif

#ifndef PRUNE_GRAPH
	useTwoLevel = 0;
#endif

	// If we can fit into memory, use pairwise distances with the simple subgoal graph
	// Fast: Preliminary experiments show that simple subgoal + pairwise does not have a noticable speed boost over two-level + pairwise. Therefore, only use simple subgoal + pairwise if we are below ~40% of the memory limit

	if (usePairwise != 0 && useTwoLevel != 1 && UnprunedPairwiseMemory() < (MEMORY_LIMIT*0.4))
	{
		usePairwise = 1;
		useTwoLevel = 0;
	}

	if (useTwoLevel != 0)	// If using two-level subgoal graphs hasn't been ruled out
	{
		useTwoLevel = 1;
		PruneSubgoals();

		// If we can fit into memory, use pairwise distances with the two-level subgoal graph
		if (usePairwise != 0 && PrunedPairwiseMemory() < MEMORY_LIMIT)
		{
			usePairwise = 1;
		}
		else
			usePairwise = 0;
	}

#ifdef SG_STATISTICS

	if (useTwoLevel)
		std::cout << "GRAPH: TWO-LEVEL SUBGOAL GRAPH" << std::endl;
	else
		std::cout << "GRAPH: SIMPLE SUBGOAL GRAPH" << std::endl;
#ifdef PAIRWISE_DISTANCES
	if (usePairwise > 0)
		std::cout << "PAIRWISE: YES" << std::endl;
	else
		std::cout << "PAIRWISE: NO" << std::endl;
#endif
	//	std::cout<<"--------------------"<<std::endl;
	PrintGraphStatistics();
#endif
	//preProcessSubGoalGraph();
	/*if (usePairwise > 0)
	{
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	for (int i = 0; i < edgeVector[sg].size(); i++)
	{
	bool hasNeighbour = false;
	subgoalId sg2 = edgeVector[sg][i].id;
	for (int j = 0; j < edgeVector[sg2].size(); j++)
	if (edgeVector[sg2][j].id == sg)
	{
	hasNeighbour = true;
	break;
	}
	if (!hasNeighbour && sg2 != edgeVector[sg][i].id)
	edgeVector[sg2].push_back(edgeVector[sg][i]);
	}
	}*/
	FinalizeGraph();
	//MemoryAnalysis(memoryLimit);	// Used to be called to see if the pairwise matrix would fit

	// OTHER IDEAS:
	// if (nSubgoals / nTraversableCells > X)	// use fast buckets instead, because subgoal graph would not be very effective
	// (i.e. random maps) (X ~ 0.2, maybe?)
	// if (nGlobalSubgoals / nSubgoals > Y)	// use simple subgoal graph instead, because the overhead of using the
	// two-level subgoal graph might not be worth it (i.e. mazes and rooms) (Y ~ 0.9, maybe?)

#ifdef PAIRWISE_DISTANCES
	if (usePairwise > 0)
		CalculatePairwiseDistances();
#endif

	//SaveGraph(filename);
}

#ifdef SG_RUNNING_IN_HOG
BL_JPS_SUBGOAL_EXP::BL_JPS_SUBGOAL_EXP(Map* map)
#else
BL_JPS_SUBGOAL_EXP::BL_JPS_SUBGOAL_EXP(std::vector<bool> &bits, int width, int height, int memoryLimit, int timeLimit) :PathFindingAlgorithm(BLJPS_SUBGOAL_ALG_EXP_NAME.c_str(),AT_BL_JPS_SUBGOAL_EXP)
#endif
{
	InitializeValues();
#ifdef SG_RUNNING_IN_HOG
	LoadMap(map);
#else
	LoadMap(bits, width, height);
#endif

	SetDirections();
	//preProcessGrid();

}

#ifdef SG_RUNNING_IN_HOG
void BL_JPS_SUBGOAL_EXP::LoadMap(Map* map)
{
	height = map->GetMapHeight() + 2;	// Add the padding
	width = map->GetMapWidth() + 2;

#else
void BL_JPS_SUBGOAL_EXP::LoadMap(std::vector<bool> &bits, int _width, int _height)
{
	height = _height + 2;	// Add the padding
	width = _width + 2;
#endif

	mapSize = height*width;

#ifdef USE_BOOL
	traversable = new bool[mapSize];
	for (unsigned int i = 0; i < mapSize; i++)
		traversable[i] = false;
#else
	unsigned int reducedMapSize = (mapSize + 7) / 8;
	traversable = new char[reducedMapSize];
	for (unsigned int i = 0; i < reducedMapSize; i++)
		traversable[i] = 0;
#endif

	int nTraversable = 0;

	for (unsigned int x = 0; x < width - 2; x++){
		for (unsigned int y = 0; y < height - 2; y++){
#ifdef SG_RUNNING_IN_HOG
			if (map->GetTerrainType(x, y) == kGround)
#else
			if (bits[y*_width + x])
#endif
			{
				SetTraversable(ToMapLoc(xyLoc(x, y)));
				nTraversable++;
			}
		}
	}

#ifdef SG_STATISTICS
	std::cout << "Height: " << height << std::endl;
	std::cout << "Width: " << width << std::endl;
	std::cout << "Map size: " << mapSize << std::endl;
	std::cout << "Traversable cells: " << nTraversable << std::endl;
#endif
}

BL_JPS_SUBGOAL_EXP::BL_JPS_SUBGOAL_EXP(const char *filename) :PathFindingAlgorithm(BLJPS_SUBGOAL_ALG_EXP_NAME.c_str(), AT_BL_JPS_SUBGOAL_EXP)
{
	InitializeValues();
	LoadGraph(filename);
	if (!useSubgoals)	// If we will be using buckets
		return;

	finalized = true;
	search = 0;
	for (subgoalId sg = 0; sg < nSubgoals + 2; sg++)
		generated[sg] = 0;

}
BL_JPS_SUBGOAL_EXP::~BL_JPS_SUBGOAL_EXP()
{
	if (neighbors)
	{
		for (subgoalId sg = 0; sg < nSubgoals; sg++)
		{
			if (neighbors[sg])
				delete[] neighbors[sg];
		}
	}

	if (localNeighbors)
	{
		for (subgoalId sg = 0; sg < nLocalSubgoals; sg++)
		{
			if (localNeighbors[sg])
				delete[] localNeighbors[sg];
		}
	}

	if (dist)
	{
		for (subgoalId sg = 0; sg < nGlobalSubgoals; sg++)
		{
			if (dist[sg])		delete[] dist[sg];
		}
	}

	if (traversable)	delete[] traversable;
	if (subgoal)		delete[] subgoal;
	if (cellInfo)	delete[] cellInfo;

	if (location)	delete[] location;
	if (pruned)		delete[] pruned;
	if (neighbors)	delete[] neighbors;
	if (nNeighbors)	delete[] nNeighbors;
	if (localNeighbors)	delete[] localNeighbors;
	if (nLocalNeighbors)	delete[] nLocalNeighbors;
	if (hasExtraEdge)delete[] hasExtraEdge;

	if (generated)	delete[] generated;
	if (gCost)		delete[] gCost;
	if (parent)		delete[] parent;
	if (open)		delete[] open;

	//if(dist)	delete [] dist;
}

void BL_JPS_SUBGOAL_EXP::SetDirections()
{
	deltaMapLoc[0] = -width;	// North
	deltaMapLoc[2] = 1;			// East
	deltaMapLoc[4] = width;		// South
	deltaMapLoc[6] = -1;		// West
	deltaMapLoc[1] = deltaMapLoc[0] + deltaMapLoc[2];	// North-East
	deltaMapLoc[3] = deltaMapLoc[2] + deltaMapLoc[4];	// South-East
	deltaMapLoc[5] = deltaMapLoc[4] + deltaMapLoc[6];	// South-West
	deltaMapLoc[7] = deltaMapLoc[6] + deltaMapLoc[0];	// North-West

	// Create the extra copies
	for (direction d = 0; d < 8; d++)
	{
		deltaMapLoc[d + 8] = deltaMapLoc[d];
		deltaMapLoc[d + 16] = deltaMapLoc[d];
	}
}
void BL_JPS_SUBGOAL_EXP::IdentifySubgoals()
{
#ifdef USE_BOOL
	subgoal = new bool[mapSize];
	for (unsigned int i = 0; i < mapSize; i++)
		subgoal[i] = false;
#else
	unsigned int reducedMapSize = (mapSize + 7) / 8;
	subgoal = new char[reducedMapSize];
	for (unsigned int i = 0; i < reducedMapSize; i++)
		subgoal[i] = 0;
#endif

	cellInfo = new subgoalId[mapSize];
	std::vector<xyLoc> locationVector;
	nSubgoals = 0;

	for (mapLoc l = 0; l < mapSize; l++)
	{
		if (IsTraversable(l))	// If the cell is traversable
		{
			for (direction d = 1; d <= 7; d += 2)	// Check its corners
			{
				// If there is an obstacle in the diagonal direction but no obstacles in the associated cardinal directions
				if (!IsTraversable(l + deltaMapLoc[d]) && IsTraversable(l + deltaMapLoc[d - 1]) && IsTraversable(l + deltaMapLoc[d + 1]))
				{
					// We have found a subgoal
					SetSubgoal(l);
					cellInfo[l] = nSubgoals;	// This will be the id of the subgoal
#ifdef SUBGOAL_LIMIT
					if (nSubgoals == 65535)
					{
						useSubgoals = false;
						return;
					}
#endif
					nSubgoals++;
					locationVector.push_back(ToXYLoc(l));
					break;	// Don't look at any more corners
				}
			}
		}
	}

	nGlobalSubgoals = nSubgoals;	// initially every subgoal is global
	location = new xyLoc[nSubgoals + 2];
	generated = new uint16_t[nSubgoals + 2];	// +2 for possible start and goal states
	gCost = new cost[nSubgoals + 2];
	parent = new subgoalId[nSubgoals + 2];

#ifdef USE_BOOL
	pruned = new bool[nSubgoals + 2];
	for (unsigned int i = 0; i < nSubgoals + 2; i++)
		pruned[i] = false;
#else
	unsigned int reducedNSubgoals = (nSubgoals + 9) / 8;
	pruned = new char[nSubgoals+2];
	for (unsigned int i = 0; i < nSubgoals + 2; i++)
		pruned[i] = 0;
#endif


#ifdef USE_BOOL
	open = new bool[nSubgoals + 2];
#else

	open = new char[(nSubgoals + 9) / 8];
#endif

	for (unsigned int i = 0; i < nSubgoals; i++)
	{
		location[i] = locationVector[i];
	}
#ifdef SG_STATISTICS
	std::cout << "Initial subgoal graph has " << nSubgoals << " subgoals." << std::endl;
#endif
}
void BL_JPS_SUBGOAL_EXP::ComputeClearances()
{
	direction d = 0;	// North clearances
	for (int x = 0; x < (int)width - 2; x++)
	{
		int clearance = 0;
		for (int y = 0; y < (int)height - 2; y++)
		{
			mapLoc loc = ToMapLoc(xyLoc(x, y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance++;
				SetClearance(loc, d, clearance);
			}
		}
	}

	d = 4;	// South clearances
	for (int x = 0; x < (int)width - 2; x++)
	{
		int clearance = 0;
		for (int y = (int)height - 3; y >= 0; y--)
		{
			mapLoc loc = ToMapLoc(xyLoc(x, y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance++;
				SetClearance(loc, d, clearance);
			}
		}
	}

	d = 6;	// West clearances
	for (int y = 0; y < (int)height - 2; y++)
	{
		int clearance = 0;
		for (int x = 0; x < (int)width - 2; x++)
		{
			mapLoc loc = ToMapLoc(xyLoc(x, y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance++;
				SetClearance(loc, d, clearance);
			}
		}
	}

	d = 2;	// East clearances
	for (int y = 0; y < (int)height - 2; y++)
	{
		int clearance = 0;
		for (int x = (int)width - 3; x >= 0; x--)
		{
			mapLoc loc = ToMapLoc(xyLoc(x, y));
			if (IsSubgoal(loc) || !IsTraversable(loc))
			{
				clearance = 0;
			}
			else
			{
				clearance++;
				SetClearance(loc, d, clearance);
			}
		}
	}
}
void BL_JPS_SUBGOAL_EXP::LinkSubgoals()
{
	std::vector<subGoalIdDir> directHReachableNeigbors;
	//	nNeighbors = new uint16_t[nSubgoals+2];	/// AAAAAAAAAAAA
	//	neighbors = new subgoalId*[nSubgoals+2];
	int nEdges = 0;
	//edgeVector.resize(nSubgoals);
	///neighborhoodVector.resize(nSubgoals);
	incomingConnectionsVector.resize(nSubgoals);
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{

		GetDirectHReachableSubgoals(location[sg], directHReachableNeigbors,true);
		//edgeVector[sg].insert(edgeVector[sg].end(), directHReachableNeigbors.begin(), directHReachableNeigbors.end());
	//	neighborhoodVector[sg].insert(neighborhoodVector[sg].end(), directHReachableNeigbors.begin(), directHReachableNeigbors.end());
		neighborhoodVector.push_back(directHReachableNeigbors);
		edgeVector.push_back(directHReachableNeigbors);

		nEdges += directHReachableNeigbors.size();

		for (int i = 0; i < directHReachableNeigbors.size(); i++)
		{
			incomingConnectionsVector[directHReachableNeigbors[i].id].push_back(subGoalIdDir(sg,  directHReachableNeigbors[i].dir, directHReachableNeigbors[i].dir2, directHReachableNeigbors[i].followUpDirs));//directHReachableNeigbors[i].dir
		}
		nEdges += directHReachableNeigbors.size();
	}
#ifdef SG_STATISTICS
	std::cout << "Initial subgoal graph has " << nEdges << " edges." << std::endl;
#endif
}
char reverseForced(char checkDirs)
{
	char res=0;
	for (int i = 0; i < 8; i++)
		if ((1 << i)&checkDirs)
			res |= 1<<((i + 4) & 7);
	return res;
}
void BL_JPS_SUBGOAL_EXP::PruneSubgoals()
{
	//return;
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif
	nLocalSubgoals = 0;
	vector<pair<subGoalIdDir, xyLoc>> allNeighbours;
	/*subgoalId sgA = 42;
	allNeighbours.clear();
	for (int i = 0; i < edgeVector[sgA].size(); i++)
		allNeighbours.push_back(pair<subGoalIdDir, xyLoc>(edgeVector[sgA][i], location[edgeVector[sgA][i].id]));
		*/
	int numPruned = 0;
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		for (int directionSet = 1; directionSet >=0; directionSet--)//run all diagonals first then cardinal directions
			for (int dir = directionSet; dir < 8; dir += 2)
		{
			bool necessary = IsPruned(sg,dir);	// This is just a short-cut to not do any of the following stuff if the subgoal is already pruned
			unsigned char checkDirs = forcedNeighbours(location[sg], dir) | naturalNeighbours(dir);//directions to check for jump points
			unsigned char checkDirsReversed = forcedNeighbours(location[sg], (dir + 4) & 7) | naturalNeighbours((dir + 4) & 7);//directions to check for jump points
			checkDirsReversed = reverseForced(checkDirsReversed);
			std::vector<subGoalIdDir> &neighbors = neighborhoodVector[sg];
			std::vector<subGoalIdDir> &incomingNeighbors = incomingConnectionsVector[sg];

			for (unsigned int i = 0; i  < neighbors.size() && !necessary; i++)	// Try to find a pair of subgoals that needs this one
			{
				if ((1 << neighbors[i].dir) & checkDirs)
					for (unsigned int j = 0; j < incomingNeighbors.size() && !necessary; j++)
						if ((1<<incomingNeighbors[j].dir2) & checkDirsReversed)
						{
							//xyLoc iLoc = location[neighbors.at(i).id];
						//	xyLoc jLoc = location[incomingNeighbors.at(j).id];
							necessary = IsNecessaryToConnect(sg, neighbors.at(i).id, incomingNeighbors.at(j).id);
						}
			}

			if (!necessary)	// If there is no reason not to prune this subgoal, prune it
			{
				PruneSubgoal(sg, dir, checkDirs, checkDirsReversed);
				numPruned++;
			}
		}
	}
	//printf("noPruned:%d\n", numPruned);
	/*allNeighbours.clear();
	for (int i = 0; i < edgeVector[sgA].size(); i++)
		allNeighbours.push_back(pair<subGoalIdDir, xyLoc>(edgeVector[sgA][i], location[edgeVector[sgA][i].id]));*/

	// Treat start and goal as pruned subgoals
	SetPruned(nSubgoals,0);
	SetPruned(nSubgoals + 1, 0);


#ifdef SG_STATISTICS
	t.EndTimer();
	std::cout << "Graph pruned in " << t.GetElapsedTime() * 1000 << "ms" << std::endl;
#endif
}
void BL_JPS_SUBGOAL_EXP::FinalizeGraph()
{
	// Initialize the hasExtraEdge array
#ifdef USE_BOOL
	hasExtraEdge = new bool[nSubgoals + 2];
	for (unsigned int i = 0; i < nSubgoals + 2; i++)
		hasExtraEdge[i] = false;
#else
	unsigned int reducedNSubgoals = (nSubgoals + 9) / 8;
	hasExtraEdge = new char[reducedNSubgoals];
	for (unsigned int i = 0; i < reducedNSubgoals; i++)
		hasExtraEdge[i] = 0;
#endif

	// Rearrange the subgoals so that global subgoals come before local subgoals
	subGoalIdDir front = subGoalIdDir(0,8);
	subGoalIdDir back = subGoalIdDir(nSubgoals - 1,8);

	std::vector<subGoalIdDir> subgoalMap;
	subgoalMap.resize(nSubgoals);



	for (subgoalId sg = 0; sg < nSubgoals; sg++)
		subgoalMap[sg] = subGoalIdDir(sg,8);

	while (true)
	{
		while (!IsPruned(front.id) && front.id < back.id)
			front.id++;

		while (IsPruned(back.id) && back.id > front.id)
			back.id--;

		if (front.id < back.id)
		{
			subgoalMap[front.id] = back;
			subgoalMap[back.id] = front;

			swap(pruned[front.id], pruned[back.id]);
			//SetUnpruned(front.id);
			//SetPruned(back.id);

			std::vector<subGoalIdDir> tempVector;
			tempVector = edgeVector[front.id];
			edgeVector[front.id] = edgeVector[back.id];
			edgeVector[back.id] = tempVector;

			tempVector = neighborhoodVector[front.id];
			neighborhoodVector[front.id] = neighborhoodVector[back.id];
			neighborhoodVector[back.id] = tempVector;


			tempVector = incomingConnectionsVector[front.id];
			incomingConnectionsVector[front.id] = incomingConnectionsVector[back.id];
			incomingConnectionsVector[back.id] = tempVector;

			xyLoc tempXY = location[front.id];
			location[front.id] = location[back.id];
			location[back.id] = tempXY;

			cellInfo[ToMapLoc(location[front.id])] = front.id;
			cellInfo[ToMapLoc(location[back.id])] = back.id;
		}
		else
			break;
	}

	for (int i = 0; i < 4; i++)
		for (int id = 0; id < diagonalJumps[i].size(); id++)
			for (int j = 0; j < diagonalJumps[i][id].size(); j++)
				diagonalJumps[i][id][j].id = subgoalMap[diagonalJumps[i][id][j].id].id;

	// Process all the edges that lead to a global subgoal
	nNeighbors = new uint16_t[nSubgoals + 2];
	neighbors = new subGoalIdDir*[nSubgoals + 2];

	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		nNeighbors[sg] = edgeVector[sg].size();
		neighbors[sg] = new subGoalIdDir[nNeighbors[sg] + 1];	// Allocate space for all of sg's neighbors + 1 for use during search

		for (unsigned i = 0; i < nNeighbors[sg]; i++)	// Fill the array, except for the last spot
		{
			subgoalId id = subgoalMap[edgeVector[sg].at(i).id].id;
			unsigned char dir2 = edgeVector[sg].at(i).dir2;// getDir2(sg, edgeVector[sg].at(i).id);
			char dir = edgeVector[sg].at(i).dir;
			char followUpDirs = forcedNeighbours(location[id], dir2) | naturalNeighbours(dir2);
			neighbors[sg][i] = subGoalIdDir(id,dir,dir2,followUpDirs );//getDir(subgoalMap[sg].id, subgoalMap[edgeVector[sg].at(i).id].id
		}

		for (unsigned i = 0; i < incomingConnectionsVector[sg].size(); i++)	// Fill the array, except for the last spot
			incomingConnectionsVector[sg][i].id = subgoalMap[incomingConnectionsVector[sg][i].id].id;
	}

	// Process the local-to-local edges (always ignore global-to-local edges - they are reconstructed by reversing local-to-global edges, when necessary)
	if (keepLocalEdges)
	{
		//nLocalNeighbors = new uint16_t[nLocalSubgoals];
		//localNeighbors = new subgoalId*[nLocalSubgoals];
		/* Since local subgoals are all grouped together, at the end of the list of subgoals, we can just use an array of size 'nLocalSubgoals'
		to store all local-to-local edges. But we would have to use an offset = nGlobalSubgoals, whenever we are accesing the array.
		To avoid having to deal with offsets, we simply initialize the array for all the subgoals, not just local subgoals. The extra memory is
		presumably small, compared to the number of local-to-local edges we store
		*/

		nLocalNeighbors = new uint16_t[nSubgoals];
		localNeighbors = new subGoalIdDir*[nSubgoals];

		for (subgoalId sg = nGlobalSubgoals; sg < nSubgoals; sg++)
		{
			nLocalNeighbors[sg] = neighborhoodVector[sg].size() - edgeVector[sg].size();	// Number of local neighbors
			localNeighbors[sg] = new subGoalIdDir[nLocalNeighbors[sg]];	// Allocate space for all of the local neighbors

			int l = 0;
			for (unsigned int i = 0; i < neighborhoodVector[sg].size(); i++)	// Add the edges
			{
				subGoalIdDir edgeTo = subgoalMap[neighborhoodVector[sg].at(i).id];
				if (IsPruned(edgeTo.id))	// If local-to-global, the edge is already processed before
				{
					localNeighbors[sg][l] = edgeTo;
					l++;
				}
			}
		}
	}

	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		edgeVector[sg].clear();
		neighborhoodVector[sg].clear();
	}
	edgeVector.clear();
	neighborhoodVector.clear();

	finalized = true;
}
int BL_JPS_SUBGOAL_EXP::GetDirectedEdgeCount()
{
	int nEdges = 0;
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
		nEdges += edgeVector[sg].size();

	return nEdges;
}
double BL_JPS_SUBGOAL_EXP::UnprunedPairwiseMemory()
{
	int estimatedStorageForUnpruned =
		(BITS_PER_CELL * mapSize + BITS_PER_SUBGOAL_STORED * nSubgoals + BITS_PER_EDGE * GetDirectedEdgeCount()) / 8; // /(8.0*1024*1024);
	double pairwiseMemory = ((((double)nSubgoals*(nSubgoals + 1)) / 2) * 32) / 8;
#ifdef SG_STATISTICS
	std::cout << "Estimated file size (unpruned): " << estimatedStorageForUnpruned / (double)(1024 * 1024) << " MiB." << std::endl;
	std::cout << "Estimated file size (unpruned + cost matrix): " << (estimatedStorageForUnpruned + pairwiseMemory) / (double)(1024 * 1024) << " MiB." << std::endl;
#endif
	// Add ~ 1kb for the extra variables that are stored
	return estimatedStorageForUnpruned + pairwiseMemory + 1024;
}
double BL_JPS_SUBGOAL_EXP::PrunedPairwiseMemory()
{
	int estimatedStorageForPruned =
		(BITS_PER_CELL * mapSize + BITS_PER_SUBGOAL_STORED * nSubgoals + BITS_PER_EDGE * GetDirectedEdgeCount()) / 8; // /(8.0*1024*1024);
	double pairwiseMemory = ((((double)nGlobalSubgoals*(nGlobalSubgoals + 1)) / 2) * 32) / 8;
#ifdef SG_STATISTICS
	std::cout << "Estimated file size (pruned): " << estimatedStorageForPruned / (double)(1024 * 1024) << " MiB." << std::endl;
	std::cout << "Estimated file size (pruned + cost matrix): " << (estimatedStorageForPruned + pairwiseMemory) / (double)(1024 * 1024) << " MiB." << std::endl;
#endif
	// Add ~ 1kb for the extra variables that are stored
	return estimatedStorageForPruned + pairwiseMemory + 1024;
}

void BL_JPS_SUBGOAL_EXP::MemoryAnalysis(int memoryLimit)
{
	int bitsPerMapCell = 1 + 1 + 32;
	// traversable, subgoal, cellInfo

	int bitsPerSubgoal = 32 + 32 + 16 + 32 + 32 + 32 + 16 + 1 + 1 + 1;
	// location + neighborPtr + nNeighbor + extraEdge + gCost + parent + search counter + open + pruned + hasExtraEdge

	int bitsPerEdge = 32;
	int nEdges = 0;
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
		nEdges += nNeighbors[sg];

	double estimatedMapMemory = (bitsPerMapCell * mapSize) / (8 * 1024);	//kb
	double estimatedSubgoalMemory = (bitsPerSubgoal*(nSubgoals + 2)) / (8 * 1024);	//kb
	double estimatedEdgeMemory = (bitsPerEdge*nEdges) / (8 * 1024);	//kb
	double estimatedMemory = (bitsPerMapCell * mapSize + bitsPerSubgoal*(nSubgoals + 2) + bitsPerEdge*nEdges) / (8 * 1024);	//kb

	double pairwiseSize = nGlobalSubgoals*(nGlobalSubgoals + 1) / 2;
	double costMatrixMemory = (pairwiseSize * 32 + nGlobalSubgoals * 32) / (8 * 1024);	//kb

#ifdef SG_STATISTICS
	std::cout << "Memory limit: " << memoryLimit / (double)(1024 * 1024) << " mb." << std::endl;
	std::cout << "Estimated memory usage (map): " << estimatedMapMemory / (double)1024 << " mb." << std::endl;
	std::cout << "Estimated memory usage (subgoals): " << estimatedSubgoalMemory / (double)1024 << " mb." << std::endl;
	std::cout << "Estimated memory usage (edges): " << estimatedEdgeMemory / (double)1024 << " mb." << std::endl;
	std::cout << "Estimated memory usage (all): " << estimatedMemory / (double)1024 << " mb." << std::endl;
#ifdef PAIRWISE_DISTANCES
	std::cout << "Estimated memory usage (with cost matrix): " << (estimatedMemory + costMatrixMemory) / (double)1024 << " mb." << std::endl;
#endif
#endif

	useSubgoals = true;
	usePairwise = 0;

	if (memoryLimit <= estimatedMemory)
	{
#ifdef SG_STATISTICS
		std::cout << "Memory limit exceeded, not using subgoals" << std::endl;
#endif
		useSubgoals = false;
	}
#ifdef PAIRWISE_DISTANCES

	else if (estimatedMemory + costMatrixMemory < memoryLimit)
	{
#ifdef SG_STATISTICS
		std::cout << "Enough space for cost matrix" << std::endl;
#endif
		usePairwise = 1;
	}
#endif
	else
	{
#ifdef SG_STATISTICS
		std::cout << "Using the subgoal graph as it is" << std::endl;
#endif
	}
}

void BL_JPS_SUBGOAL_EXP::CalculatePairwiseDistances()
{
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif
	// Floyd-Warshall algorithm to compute pairwise distances between all global subgoals

	// Set self-costs to 0; rest to infinity
	dist = new cost*[nGlobalSubgoals];

	for (int sg = 0; sg < nGlobalSubgoals; sg++)
	{
		dist[sg] = new cost[sg + 1];
		for (int sg2 = 0; sg2 < sg; sg2++)
			dist[sg][sg2] = INFINITE_COST;

		dist[sg][sg] = 0;
	}

	// Add the edge costs
	for (int sg = 0; sg < nGlobalSubgoals; sg++)
	{
		for (int i = 0; i < nNeighbors[sg]; i++)
		{
			subgoalId sg2 = neighbors[sg][i].id;
			if (!IsPruned(sg2, neighbors[sg][i].dir) || !IsPruned(sg, neighbors[sg][i].dir))
			{
				cost c = HCost(sg, sg2);
				if (sg > sg2)	dist[sg][sg2] = c;
				else			dist[sg2][sg] = c;
			}
		}
		/*for (int i = 0; i < incomingConnectionsVector[sg].size(); i++)
		{
			subgoalId sg2 = incomingConnectionsVector[sg][i].id;
			//if (!IsPruned(sg2))
			{
				cost c = HCost(sg, sg2);
				if (sg > sg2)	dist[sg][sg2] = c;
				else			dist[sg2][sg] = c;
			}
		}*/
	}

	// Find the pairwise distances
	for (int sg = 0; sg < nGlobalSubgoals; sg++)
		for (int sg1 = 0; sg1 < nGlobalSubgoals; sg1++)
		{
			cost cost1 = (sg>sg1) ? dist[sg][sg1] : dist[sg1][sg];
			if (cost1 != INFINITE_COST)
			{
				for (int sg2 = 0; sg2 <= sg1; sg2++)
				{
					cost cost2 = (sg>sg2) ? dist[sg][sg2] : dist[sg2][sg];
					cost2 = (cost2 != INFINITE_COST) ? (cost1 + cost2) : cost2;
					dist[sg1][sg2] = (dist[sg1][sg2] < cost2) ? dist[sg1][sg2] : cost2;
				}
			}
		}

#ifdef SG_STATISTICS
	std::cout << "Pairwise distances computed in " << t.EndTimer() * 1000 << "ms" << std::endl;
#endif
}
void dumpJumpPointData(std::ofstream &out, vector<pair<short, short> > & vec)
{
	short numBoundaries = vec.size();
	out.write((char*)&numBoundaries, 2);
	for (int i = 0; i<vec.size(); i++)
		out.write((char*)&vec[i], 4);
}
void readJumpPointData(std::ifstream &in, vector<pair<short, short> > & vec)
{
	short numBoundaries = 0;
	in.read((char*)&numBoundaries, 2);

	for (int i = 0; i<numBoundaries; i++)
	{
		pair<short, short> temp;
		in.read((char*)&temp, 4);
		vec.push_back(temp);
	}
}
void dumpPreprocessedEndDiagonals(std::ofstream &out, vector<DiagonalJumpEntry2 > & vec)
{
	short numBoundaries = vec.size();
	out.write((char*)&numBoundaries, 2);
	for (int i = 0; i < vec.size(); i++)
	{
		out.write((char*)&vec[i], sizeof(DiagonalJumpEntry2));

	}
}
void readPreprocessedEndDiagonals(std::ifstream &in, vector<DiagonalJumpEntry2 > & vec)
{
	short numBoundaries = 0;
	in.read((char*)&numBoundaries, 2);
	for (int i = 0; i < numBoundaries; i++)
	{
		DiagonalJumpEntry2 temp(Coordinate(0, 0), Coordinate(0, 0), 0);
		in.read((char*)& temp, sizeof(DiagonalJumpEntry2) );
		vec.push_back(temp);
	}
}
void dumpPreprocessedIncomingConnections(std::ofstream &out, vector<subGoalIdDir > & vec)
{
	short numBoundaries = vec.size();
	out.write((char*)&numBoundaries, 2);
	for (int i = 0; i < vec.size(); i++)
	{
		out.write((char*)&vec[i], sizeof(subGoalIdDir));

	}
}
void readPreprocessedIncomingConnections(std::ifstream &in, vector<subGoalIdDir > & vec)
{
	short numBoundaries = 0;
	in.read((char*)&numBoundaries, 2);
	for (int i = 0; i < numBoundaries; i++)
	{
		subGoalIdDir temp(0, 0);
		in.read((char*)& temp, sizeof(subGoalIdDir));
		vec.push_back(temp);
	}
}
void BL_JPS_SUBGOAL_EXP::SaveGraph(const char *filename)
{
	std::ofstream out(filename, std::ios::out | std::ios::binary);

	out.write((char*)&useSubgoals, sizeof(bool));

	if (!useSubgoals)
		return;

	out.write((char*)&usePairwise, sizeof(char));
	out.write((char*)&keepLocalEdges, sizeof(bool));
	out.write((char*)&height, sizeof(unsigned int));
	out.write((char*)&width, sizeof(unsigned int));
	out.write((char*)&mapSize, sizeof(unsigned int));
	out.write((char*)&nSubgoals, sizeof(unsigned int));
	out.write((char*)&nGlobalSubgoals, sizeof(unsigned int));
	out.write((char*)&nLocalSubgoals, sizeof(unsigned int));
	out.write((char*)&deltaMapLoc[0], sizeof(int) * 24);

	out.write((char*)cellInfo, sizeof(subgoalId)*(mapSize));
#ifdef USE_BOOL
	out.write((char*)traversable, mapSize);
	out.write((char*)subgoal, mapSize);
	out.write((char*)pruned, nSubgoals + 2);
#else
	unsigned int reducedMapSize = (mapSize + 7) / 8;
	unsigned int reducedNSubgoals = (nSubgoals + 9) / 8;
	out.write(traversable, reducedMapSize);
	out.write(subgoal, reducedMapSize);
	out.write(pruned, reducedNSubgoals);
#endif
	out.write((char*)location, sizeof(xyLoc)*(nSubgoals + 2));
	out.write((char*)nNeighbors, sizeof(uint16_t)*(nSubgoals + 2));
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		//assert(!this->HasExtraEdge(sg));
		out.write((char*)neighbors[sg], sizeof(subGoalIdDir)*nNeighbors[sg]);
	}
	//std::vector<std::vector<subGoalIdDir> > incomingConnectionsVector;
	///vector<vector<DiagonalJumpEntry2>> diagonalJumps[4];
	//vector<vector<pair<short, short> > > jumpLookup[4];
	/*if (keepLocalEdges)
	{
		out.write((char*)nLocalNeighbors, sizeof(uint16_t)*(nSubgoals));
		for (subgoalId sg = 0; sg < nSubgoals; sg++)
		{
			if (nLocalNeighbors[sg] > 0)
				out.write((char*)localNeighbors[sg], sizeof(subGoalIdDir)*nLocalNeighbors[sg]);
		}
	}*/

#ifdef PAIRWISE_DISTANCES
	if (usePairwise > 0)
		for (int sg = 0; sg < nGlobalSubgoals; sg++)
			out.write((char*)dist[sg], sizeof(cost)*(sg + 1));
#endif
	for (int y = 0; y<height-2; y++)
	{
		short numBoundaries = xBoundaryPoints[y].size() - 1;
		out.write((char*)&numBoundaries, 2);
		for (int i = 0; i<xBoundaryPoints[y].size() - 1; i++)
			out.write((char*)&xBoundaryPoints[y][i], 2);
	}
	for (int x = 0; x<width - 2; x++)
	{
		short numBoundaries = yBoundaryPoints[x].size() - 1;
		out.write((char*)&numBoundaries, 2);
		for (int i = 0; i<yBoundaryPoints[x].size() - 1; i++)
			out.write((char*)&yBoundaryPoints[x][i], 2);
	}
	for (int y = 0; y < height - 2; y++)
	{
		dumpJumpPointData(out, jumpLookup[1][y]);
		dumpJumpPointData(out, jumpLookup[3][y]);
	}
	for (int x = 0; x < width - 2; x++)
	{
		dumpJumpPointData(out, jumpLookup[2][x]);
		dumpJumpPointData(out, jumpLookup[0][x]);
	}
	for (int i = 0; i < 4; i++)
	{
		short size = diagonalJumps[i].size();
		out.write((char*)&size, sizeof(short));
		for (int j = 0; j < size; j++)
			dumpPreprocessedEndDiagonals(out, diagonalJumps[i][j]);
	}
	for (int i = 0; i < nSubgoals; i++)
		dumpPreprocessedIncomingConnections(out, incomingConnectionsVector[i]);
	out.close();
}
void BL_JPS_SUBGOAL_EXP::LoadGraph(const char *filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);

	in.read((char*)&useSubgoals, sizeof(bool));
	if (!useSubgoals)
		return;

	in.read((char*)&usePairwise, sizeof(char));
	in.read((char*)&keepLocalEdges, sizeof(bool));
	in.read((char*)&height, sizeof(unsigned int));
	in.read((char*)&width, sizeof(unsigned int));
	in.read((char*)&mapSize, sizeof(unsigned int));
	in.read((char*)&nSubgoals, sizeof(unsigned int));
	in.read((char*)&nGlobalSubgoals, sizeof(unsigned int));
	in.read((char*)&nLocalSubgoals, sizeof(unsigned int));
	in.read((char*)&deltaMapLoc[0], sizeof(int) * 24);

	// Allocate space
	cellInfo = new subgoalId[mapSize];
#ifdef USE_BOOL
	traversable = new bool[mapSize];
	subgoal = new bool[mapSize];
	pruned = new bool[nSubgoals + 2];
	open = new bool[nSubgoals + 2];
	hasExtraEdge = new bool[nSubgoals + 2];
	for (subgoalId sg = 0; sg < nSubgoals + 2; sg++)
	{
		open[sg] = false;
		hasExtraEdge[sg] = false;
	}
#else
	unsigned int reducedMapSize = (mapSize + 7) / 8;
	unsigned int reducedNSubgoals = (nSubgoals + 9) / 8;
	traversable = new char[reducedMapSize];
	subgoal = new char[reducedMapSize];
	pruned = new char[reducedNSubgoals];
	open = new char[reducedNSubgoals];
	hasExtraEdge = new char[reducedNSubgoals];
	for (unsigned int i = 0; i < reducedNSubgoals; i++)
	{
		open[i] = 0;
		hasExtraEdge[i] = 0;
	}
#endif
	location = new xyLoc[nSubgoals + 2];
	nNeighbors = new uint16_t[nSubgoals + 2];
	nLocalNeighbors = new uint16_t[nSubgoals];

	generated = new uint16_t[nSubgoals + 2];
	gCost = new cost[nSubgoals + 2];
	parent = new subgoalId[nSubgoals + 2];

	in.read((char*)cellInfo, sizeof(subgoalId)*(mapSize));
#ifdef USE_BOOL
	in.read((char*)traversable, mapSize);
	in.read((char*)subgoal, mapSize);
	in.read((char*)pruned, nSubgoals + 2);
#else
	in.read(traversable, reducedMapSize);
	in.read(subgoal, reducedMapSize);
	in.read(pruned, reducedNSubgoals);
#endif
	in.read((char*)location, sizeof(xyLoc)*(nSubgoals + 2));
	in.read((char*)nNeighbors, sizeof(uint16_t)*(nSubgoals + 2));

	neighbors = new subGoalIdDir*[nSubgoals + 2];
	// Read the edges
	for (subgoalId sg = 0; sg < nSubgoals; sg++)
	{
		neighbors[sg] = new subGoalIdDir[nNeighbors[sg] + 1];	// + 1 for the extra edge during search
		in.read((char*)neighbors[sg], sizeof(subGoalIdDir)*nNeighbors[sg]);
	}

	/*if (keepLocalEdges)
	{
		in.read((char*)nLocalNeighbors, sizeof(uint16_t)*(nSubgoals));
		localNeighbors = new subgoalId*[nSubgoals];
		for (subgoalId sg = 0; sg < nSubgoals; sg++)
		{
			if (nLocalNeighbors[sg] > 0)
			{
				localNeighbors[sg] = new subgoalId[nLocalNeighbors[sg]];
				in.read((char*)localNeighbors[sg], sizeof(subgoalId)*nLocalNeighbors[sg]);
			}
		}
	}*/

#ifdef PAIRWISE_DISTANCES
	// Read the pairwise distances (if written)
	if (usePairwise > 0)
	{
		dist = new cost*[nGlobalSubgoals];
		for (int sg = 0; sg < nGlobalSubgoals; sg++)
		{
			dist[sg] = new cost[sg + 1];
			in.read((char*)dist[sg], sizeof(cost)*(sg + 1));
		}
	}
#endif

#ifdef SG_STATISTICS
	std::cout << "Graph read from file.." << std::endl;
#endif

	for (int y = 0; y<height - 2; y++)
	{
		xBoundaryPoints.push_back(vector<short>());
		short numBoundaries = 0;
		in.read((char*)&numBoundaries, 2);
		for (int i = 0; i<numBoundaries; i++)
		{
			short tempVal = 0;
			in.read((char*)&tempVal, 2);
			xBoundaryPoints[y].push_back(tempVal);
		}
		xBoundaryPoints[y].push_back(width - 2);
	}
	for (int x = 0; x<width - 2; x++)
	{
		yBoundaryPoints.push_back(vector<short>());
		short numBoundaries = 0;
		in.read((char*)&numBoundaries, 2);
		for (int i = 0; i<numBoundaries; i++)
		{
			short tempVal = 0;
			in.read((char*)&tempVal, 2);
			yBoundaryPoints[x].push_back(tempVal);
		}
		yBoundaryPoints[x].push_back(height - 2);
	}
	for (int y = 0; y < height - 2; y++)
	{
		jumpLookup[1].push_back(vector<pair<short, short> >());
		jumpLookup[3].push_back(vector<pair<short, short> >());
		readJumpPointData(in, jumpLookup[1].back());
		readJumpPointData(in, jumpLookup[3].back());

	}
	for (int x = 0; x < width - 2; x++)
	{
		jumpLookup[2].push_back(vector<pair<short, short> >());
		jumpLookup[0].push_back(vector<pair<short, short> >());
		readJumpPointData(in, jumpLookup[2].back());
		readJumpPointData(in, jumpLookup[0].back());
	}
	for (int i = 0; i < 4; i++)
	{
		short size = 0;
		in.read((char*)&size, sizeof(short));
		for (int j = 0; j < size; j++)
		{
			diagonalJumps[i].push_back(vector<DiagonalJumpEntry2>());

			readPreprocessedEndDiagonals(in, diagonalJumps[i][j]);
		}
	}
	incomingConnectionsVector.resize(nSubgoals);

	/*for (int i = 0; i < nSubgoals; i++)
		for (int j = 0; j < nNeighbors[i];j++)
		{
			incomingConnectionsVector[neighbors[i][j].id].push_back(subGoalIdDir(i, neighbors[i][j].dir, neighbors[i][j].dir2, neighbors[i][j].followUpDirs));
		}*/
	for (int i = 0; i < nSubgoals; i++)
		readPreprocessedIncomingConnections(in, incomingConnectionsVector[i]);
	initializeTime = 0.0;
	searchTime = 0.0;
	finalizeTime = 0.0;
	finalized = true;
	in.close();
}

void BL_JPS_SUBGOAL_EXP::GetMoveDirectionDetails(xyLoc & from, xyLoc & to, direction & c, direction & d, int & nTotal, int & nTotalDiag)
{
	int dx = (from.x>to.x) ? (from.x - to.x) : (to.x - from.x);
	int dy = (from.y>to.y) ? (from.y - to.y) : (to.y - from.y);

	nTotal = (dx>dy) ? (dx) : (dy);	// total number of diagonal and cardinal moves that must be made to reach the target
	nTotalDiag = (dx<dy) ? (dx) : (dy);	// total number of diagonal moves that must be made to reach the target

	c = (dx>dy) ? ((from.x<to.x) ? 2 : 6) : ((from.y<to.y) ? 4 : 0);	// only make this kind of cardinal moves
	d = (from.x<to.x) ? ((from.y<to.y) ? 3 : 1) : ((from.y<to.y) ? 5 : 7);	// only make this kind of diagonal moves
}
bool BL_JPS_SUBGOAL_EXP::IsHReachable(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path, bool append)
{
	int nTotalCard, nTotalDiag, nTotal;
	direction c, d;
	GetMoveDirectionDetails(from, to, c, d, nTotal, nTotalDiag);
	nTotalCard = nTotal - nTotalDiag;

	int *diagCount = new int[nTotal + 1];
	diagCount[0] = 0;
	for (int i = 1; i <= nTotal; i++)
		diagCount[i] = -1;

	mapLoc loc = ToMapLoc(from);
	int i = 0;
	while (i < nTotal)
	{
		//if we can move cardinally, do it
		if (i - diagCount[i] < nTotalCard && diagCount[i + 1] < diagCount[i] && IsTraversable(loc + deltaMapLoc[c]))	//0 <= x+cardX && x+cardX < width && 0 <= y+cardY && y+cardY < height &&
		{
			diagCount[i + 1] = diagCount[i];
			i++;
			loc += deltaMapLoc[c];
		}

		//else, if we can move diagonally, do it
		else if (diagCount[i] < nTotalDiag && diagCount[i + 1] <= diagCount[i] && IsTraversable(loc + deltaMapLoc[d]) && IsTraversable(loc + deltaMapLoc[d - 1]) && IsTraversable(loc + deltaMapLoc[d + 1]))
		{
			diagCount[i + 1] = diagCount[i] + 1;
			i++;
			loc += deltaMapLoc[d];
		}

		//else, backtrack
		else
		{
			if (i == 0)	// cannot backtrack
			{
				delete[] diagCount;
				return false;
			}

			i--;

			if (diagCount[i] == diagCount[i + 1])
				loc -= deltaMapLoc[c];
			else
				loc -= deltaMapLoc[d];
		}
	}

	loc = ToMapLoc(from);
	if (!append)
	{
		path.clear();
		path.push_back(loc);
	}
	for (int i = 1; i <= nTotal; i++)
	{
		if (diagCount[i] > diagCount[i - 1])	// Made a diagonal move
			loc += deltaMapLoc[d];
		else
			loc += deltaMapLoc[c];

		path.push_back(loc);
	}

	delete[] diagCount;
	return true;
}
bool BL_JPS_SUBGOAL_EXP::IsQuickHReachable(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path, bool append)
{
	int nCard, nDiag, nTotal;
	direction c, d;
	GetMoveDirectionDetails(from, to, c, d, nTotal, nDiag);
	nCard = nTotal - nDiag;

	mapLoc loc = ToMapLoc(from);

	if (!append)
	{
		path.clear();
		path.push_back(loc);
	}
	while (nCard)
	{
		loc += deltaMapLoc[c];
		if (!IsTraversable(loc))
			return false;

		path.push_back(loc);
		nCard--;
	}

	while (nDiag)
	{
		loc += deltaMapLoc[d];
		if (!IsTraversable(loc) || !IsTraversable(loc + deltaMapLoc[d + 3]) || !IsTraversable(loc + deltaMapLoc[d + 5]))
			return false;

		path.push_back(loc);
		nDiag--;
	}

	return true;
}
bool BL_JPS_SUBGOAL_EXP::IsLookaheadHReachable(xyLoc & from, xyLoc & to, std::vector<mapLoc> & path, bool append)
{
	int nCard, nDiag, nTotal;
	direction c, d;
	GetMoveDirectionDetails(from, to, c, d, nTotal, nDiag);
	nCard = nTotal - nDiag;

	/*
	6 	3
	7	5	2
	C	4	1

	Current is the current position of the agent, assume it is trying to move north/nort-east.
	It can either move to 4 or 5.
	It can observe any of the numbered cells to determine its movement (observing 1,2, and 3 is the lookahead)

	Note that the loop constraint forces us to have at least one diagonal and at least one cardinal move remaining
	Therefore, if the given start and goal states are in the map limits, we don't need to check if 1,2,4,5, or 7 is in the map bounds
	*/

	int offset1 = deltaMapLoc[c] + deltaMapLoc[c];
	int offset2 = deltaMapLoc[c] + deltaMapLoc[d];
	//int offset3 = deltaMapLoc[d] + deltaMapLoc[d];
	int offset4 = deltaMapLoc[c];
	int offset5 = deltaMapLoc[d];
	//int offset6 = offset3 - deltaMapLoc[c];
	int offset7 = offset5 - deltaMapLoc[c];

	mapLoc loc = ToMapLoc(from);


	if (!append)
	{
		path.clear();
		path.push_back(loc);
	}

	while (nDiag && nCard)
	{
		if (!IsTraversable(loc + offset4))	// If 4 is an obstacle, no movement possible
			return false;

		else if (!IsTraversable(loc + offset5) || !IsTraversable(loc + offset7))	// If 5 or 7 is blocked, we can not move diagonally
		{																								// so, we move cardinally
			loc += offset4; nCard--;	path.push_back(loc);
		}

		else if (!IsTraversable(loc + offset1))	// If 1 is an obstacle and we move cardinally, then we can't move on the next turn
		{															// so, we move diagonally
			loc += offset5; nDiag--;	path.push_back(loc);
		}

		else if (!IsTraversable(loc + offset2))	// If 2 is an obstacle and we move diagonally, then we can't move on the next turn
		{															// so, we move cardinally
			loc += offset4; nCard--;	path.push_back(loc);
		}

		else	// Whether 6 or 3 is an obstacle does not force us to move a certain way, so we make a heuristic decision between cardinal and diagonal
		{
			if (nCard > nDiag)	//If we have more cardinal moves remaining, prefer cardinal move
			{
				loc += offset4; nCard--;	path.push_back(loc);
			}
			else
			{
				loc += offset5; nDiag--;	path.push_back(loc);
			}
		}
	}

	// We have only cardinal or only diagonal moves (or no moves) remaining, proceed quickly without lookahead
	while (nDiag)
	{
		if (!IsTraversable(loc + offset4) || !IsTraversable(loc + offset5) || !IsTraversable(loc + offset7))
			return false;

		loc += offset5; nDiag--;	path.push_back(loc);
	}

	while (nCard)
	{
		if (!IsTraversable(loc + offset4))
			return false;

		loc += offset4; nCard--;	path.push_back(loc);
	}

	return true;
}

void BL_JPS_SUBGOAL_EXP::GetDirectHReachableSubgoals(xyLoc & from, std::vector<subgoalId> & subgoals)
{
	mapLoc origin = ToMapLoc(from);
	subgoals.clear();
	int clearance[9];

	// Compute cardinal clearances
	for (direction c = 0; c <= 6; c += 2)
	{
		clearance[c] = 0;

		mapLoc loc = origin + deltaMapLoc[c];
		while (IsTraversable(loc) && !IsSubgoal(loc))
		{
			loc += deltaMapLoc[c];
			clearance[c]++;
		}

		if (IsSubgoal(loc))
			subgoals.push_back(ToSubgoalId(loc));
	}

	clearance[8] = clearance[0];	// Report North twice, to avoid using the modulus operation

	// Compute diagonal clearances
	for (direction d = 1; d <= 7; d += 2)
	{
		clearance[d] = 0;

		mapLoc loc = origin + deltaMapLoc[d];
		while (IsTraversable(loc) && !IsSubgoal(loc) &&
			IsTraversable(loc + deltaMapLoc[d + 3]) && 	// loc is already updated with the diagonal move, so the associated
			IsTraversable(loc + deltaMapLoc[d + 5]))		// cardinal directions are +3, +5 instead of +1 +7 (-7 == -1 % 8)
		{
			loc += deltaMapLoc[d];
			clearance[d]++;
		}

		if (IsSubgoal(loc) && IsTraversable(loc + deltaMapLoc[d + 3]) && IsTraversable(loc + deltaMapLoc[d + 5]))
			subgoals.push_back(ToSubgoalId(loc));
	}

	/* Now, explore the 8 areas that the cardinal and diagonal separators create.
	* Each area is at most as large as a parallelogram whose dimensions are the
	* clearances of its associated cardinal and diagonal directions.
	*/
	for (direction d = 1; d <= 7; d += 2)
	{
		for (direction c = d - 1; c <= d + 1; c += 2)
		{
#ifdef USE_CLEARANCES
			//int maxExt = GetClearance(from,c);
			int maxExt = clearance[c] + 1;
			mapLoc loc = origin;
#else
			int maxExt = clearance[c];
#endif

			for (int i = 1; i <= clearance[d]; i++)	// Go along the diagonal, from each cell on the diagonal ..
			{
#ifdef USE_CLEARANCES
				loc += deltaMapLoc[d];
				int cardClearance = GetTotalClearance(loc, c);
				//int cardClearance = GetClearance(loc,c);
				if (cardClearance < maxExt)
				{
					//if (cardClearance == CLEARANCE_LIMIT)
					//{
					//	cardClearance = GetTotalClearance(loc,c);
					//}

					mapLoc loc2 = loc + deltaMapLoc[c] * cardClearance;
					if (IsSubgoal(loc2))
						subgoals.push_back(ToSubgoalId(loc2));
				}

				maxExt = (maxExt < cardClearance) ? (maxExt) : (cardClearance);
#else
				mapLoc loc = origin + deltaMapLoc[d] * i + deltaMapLoc[c];	// .. start sweeping along the cardinal direction
				int ext = 0;	// we may not have to explore the whole area

				while (ext < maxExt && IsTraversable(loc) && !IsSubgoal(loc))	// may be sped up with precomputed cardinal clearances
				{
					loc += deltaMapLoc[c];
					ext++;
				}

				if (ext < maxExt && IsSubgoal(loc))
					subgoals.push_back(ToSubgoalId(loc));

				if (ext < maxExt)
					maxExt = ext;
#endif
			}
		}
	}
}

void BL_JPS_SUBGOAL_EXP::jump(const Coordinate &c, const char dir, const char origDir, std::vector<subGoalIdDir> & subgoals)
{
	Coordinate nc = nextCoordinate(c, dir);
	bool b = true;
#ifdef DIAG_UNBLOCKED
	if (dir & 1)
	{
		Coordinate nextC2 = nextCoordinate(c, (dir + 1) & 7);
		Coordinate nextC3 = nextCoordinate(c, (dir + 7) &7);
		mapLoc index2 = ToMapLoc(nextC2);
		mapLoc index3 = ToMapLoc(nextC3);

		b = IsTraversable(index2) && IsTraversable(index3);
	}
#endif
	mapLoc index = ToMapLoc(nc);

	if (!b|| !IsTraversable(index) )
		return ;

	unsigned char dirs;
	if (dirs = forcedNeighbours(nc, dir))// ||( (dir&1)&&IsSubgoal(index)))
	{
		subgoals.push_back(subGoalIdDir(ToSubgoalId(index), origDir, dir, forcedNeighbours(nc, dir) | naturalNeighbours(dir)));

		return ;
	}

	if (dir&1)
	{
		jump(nc, (dir + 7) & 7, origDir, subgoals);
		jump(nc, (dir + 1) & 7, origDir,subgoals);
	}

	jump(nc, dir, origDir,subgoals);

}
void BL_JPS_SUBGOAL_EXP::jumpSpecial(const Coordinate &c, const char dir, const char origDir, std::vector<subGoalIdDir> & subgoals)
{
	Coordinate nc = nextCoordinate(c, dir);
	bool b = true;
#ifdef DIAG_UNBLOCKED
	if (dir & 1)
	{
		Coordinate nextC2 = nextCoordinate(c, (dir + 1) & 7);
		Coordinate nextC3 = nextCoordinate(c, (dir + 7) & 7);
		mapLoc index2 = ToMapLoc(nextC2);
		mapLoc index3 = ToMapLoc(nextC3);

		b = IsTraversable(index2) && IsTraversable(index3);
	}
#endif
	mapLoc index = ToMapLoc(nc);

	if (!b || !IsTraversable(index))
		return;

	unsigned char dirs;
	if (dirs = forcedNeighbours(nc, dir) || ((dir & 1) && IsSubgoal(index)))
	{
		subgoals.push_back(subGoalIdDir(ToSubgoalId(index), origDir, dir, forcedNeighbours(nc, dir) | naturalNeighbours(dir)));
		if (!(pruned[subgoals.back().id] & (1 << dir)))
			return;
	}

	jumpSpecial(nc, dir, origDir, subgoals);

}
void BL_JPS_SUBGOAL_EXP::GetDirectHReachableSubgoals(xyLoc & from, std::vector<subGoalIdDir> & subgoals,bool jumpPoints)
{
	subgoals.clear();

	if (jumpPoints)
	{
		for (int i = 0; i < 8; i++)
		{
			//char dirs = (naturalNeighbours((i + 7) & 7) | forcedNeighbours(from, (i + 7) & 7))& (1 << i);
			//char dirs2 = (naturalNeighbours((i + 1) & 7) | forcedNeighbours(from, (i + 1) & 7))& (1 << i);

			//if (IsTraversable(nextCoordinate(from, (i + 4) & 7)) || dirs || dirs2)

				//if (IsTraversable(nextCoordinate(from, (i + 4) & 7)))
				jump(from, i, i, subgoals);
		}
		return;
	}
	mapLoc origin = ToMapLoc(from);
	int clearance[9];

	// Compute cardinal clearances
	for (direction c = 0; c <= 6; c += 2)
	{
		clearance[c] = 0;

		mapLoc loc = origin + deltaMapLoc[c];
		while (IsTraversable(loc) && !IsSubgoal(loc))
		{
			loc += deltaMapLoc[c];
			clearance[c]++;
		}

		if (IsSubgoal(loc))
			subgoals.push_back(subGoalIdDir(ToSubgoalId(loc), c));
	}

	clearance[8] = clearance[0];	// Report North twice, to avoid using the modulus operation

	// Compute diagonal clearances
	for (direction d = 1; d <= 7; d += 2)
	{
		clearance[d] = 0;

		mapLoc loc = origin + deltaMapLoc[d];
		while (IsTraversable(loc) && !IsSubgoal(loc) &&
			IsTraversable(loc + deltaMapLoc[d + 3]) && 	// loc is already updated with the diagonal move, so the associated
			IsTraversable(loc + deltaMapLoc[d + 5]))		// cardinal directions are +3, +5 instead of +1 +7 (-7 == -1 % 8)
		{
			loc += deltaMapLoc[d];
			clearance[d]++;
		}

		if (IsSubgoal(loc) && IsTraversable(loc + deltaMapLoc[d + 3]) && IsTraversable(loc + deltaMapLoc[d + 5]))
			subgoals.push_back(subGoalIdDir(ToSubgoalId(loc), d));
	}

	/* Now, explore the 8 areas that the cardinal and diagonal separators create.
	* Each area is at most as large as a parallelogram whose dimensions are the
	* clearances of its associated cardinal and diagonal directions.
	*/
	for (direction d = 1; d <= 7; d += 2)
	{
		for (direction c = d - 1; c <= d + 1; c += 2)
		{
#ifdef USE_CLEARANCES
			//int maxExt = GetClearance(from,c);
			int maxExt = clearance[c] + 1;
			mapLoc loc = origin;
#else
			int maxExt = clearance[c];
#endif

			for (int i = 1; i <= clearance[d]; i++)	// Go along the diagonal, from each cell on the diagonal ..
			{
#ifdef USE_CLEARANCES

				loc += deltaMapLoc[d];
				int cardClearance = GetTotalClearance(loc, c);
				//int cardClearance = GetClearance(loc,c);
				if (cardClearance < maxExt)
				{
					//if (cardClearance == CLEARANCE_LIMIT)
					//{
					//	cardClearance = GetTotalClearance(loc,c);
					//}

					mapLoc loc2 = loc + deltaMapLoc[c] * cardClearance;
					xyLoc g = ToXYLoc(loc2);

					if (IsSubgoal(loc2))
						subgoals.push_back(subGoalIdDir(ToSubgoalId(loc2), d));
				}

				maxExt = (maxExt < cardClearance) ? (maxExt) : (cardClearance);
#else
				mapLoc loc = origin + deltaMapLoc[d] * i + deltaMapLoc[c];	// .. start sweeping along the cardinal direction
				int ext = 0;	// we may not have to explore the whole area

				while (ext < maxExt && IsTraversable(loc) && !IsSubgoal(loc))	// may be sped up with precomputed cardinal clearances
				{
					loc += deltaMapLoc[c];
					ext++;
				}

				if (ext < maxExt && IsSubgoal(loc))
					subgoals.push_back(ToSubgoalId(loc));

				if (ext < maxExt)
					maxExt = ext;
#endif
			}
		}
	}
}
cost BL_JPS_SUBGOAL_EXP::CostOtherPath(subgoalId & sg, subgoalId & sg1, subgoalId & sg2, cost limit)
{

	// If sg2 is already pruned, make sure it is reachable by adding the relevant global-to-local edges
	if (IsPruned(sg2))
	{
		for (unsigned int i = 0; i < edgeVector[sg2].size(); i++)
		{
			subGoalIdDir tempSg = edgeVector[sg2].at(i);
			edgeVector[tempSg.id].push_back(subGoalIdDir(sg2,8));
		}
	}

	// Temporarily remove all the outgoing edges from sg, essentially removing it from the search// Temporarily remove all the outgoing edges from sg, essentially removing it from the search
	std::vector<subGoalIdDir> edges = edgeVector[sg];
	edgeVector[sg].clear();

	/* Do the search and get the cost. Since we are only checking if we can do better
	* than costThrough, add a search limit to stop the search if we exceed costThrough
	*/
	cost alternateCost = SubgoalAStarSearch(sg1, sg2, limit);
	//cost alternateCost = SubgoalAStarSearch(sg1,sg2);

	// Add back the outgoing edges from sg
	edgeVector[sg] = edges;

	// If sg2 is already pruned, make sure to remove the global-to-local edges we added earlier
	if (IsPruned(sg2))
	{
		for (unsigned int i = 0; i < edgeVector[sg2].size(); i++)
		{
			edgeVector[edgeVector[sg2].at(i).id].pop_back();
		}
	}
	return alternateCost;
}
bool BL_JPS_SUBGOAL_EXP::IsNecessaryToConnect(subgoalId sg, subgoalId sg1, subgoalId sg2)
{
	/* We want to make sure that if sg is pruned, then sg1 and sg2 will still be somehow connected optimally.
	*/

	/* Method 1: If h(sg1,sg2) == h(sg1,sg) + h(sg,sg2), then that means
	* sg1 and sg2 are h-reachable, therefore sg is not necessary to connect
	*/
	cost costThrough = HCost(sg1, sg) + HCost(sg, sg2);
#ifdef USE_FLOAT_DIST
	if (abs(costThrough - HCost(sg1, sg2))<0.1)
#else
	if (costThrough == HCost(sg1, sg2))

#endif
		return false;
	vector<Coordinate> sol;
	if (directSolution(location[sg1].x, location[sg1].y, location[sg2].x, location[sg2].y, sol))
	//if (IsHReachable(location[sg1], location[sg2]))
		return false;

	/* Method 5 (last resort): Remove sg from the graph, do an A* search from sg1 to sg2.
	* If the path found is larger then h(sg1,sg) + h(sg,sg2), then sg is necessary to connect
	*/

#ifdef USE_FLOAT_DIST
	//cost costOther = CostOtherPath(sg, sg1, sg2, costThrough + 1);
	//if (costOther < costThrough || abs(costOther - costThrough)<0.1)

#else
	//if (CostOtherPath(sg, sg1, sg2, costThrough + 1) <= costThrough)


#endif
	//	return false;

	return true;
}
void BL_JPS_SUBGOAL_EXP::PruneSubgoal(subgoalId sg, char dir, char checkDirs, char checkDirsReversed)
{
	SetPruned(sg,dir);
	if (isCompletlyPruned(sg))
	{
		nGlobalSubgoals--;
				nLocalSubgoals++;

	}

	std::vector<subGoalIdDir> &neighbors = neighborhoodVector[sg];
	std::vector<subGoalIdDir> &incomingNeighbors = incomingConnectionsVector[sg];
	vector<bool> removeFlags, removeFlags2;
	removeFlags.resize(incomingNeighbors.size(), false);
	removeFlags2.resize(neighbors.size(), false);
	// Add the necessary connections between surrounding subgoals
	for (unsigned int i = 0; i  < neighbors.size(); i++)	// Try to find a pair of subgoals that needs this one
		if ( neighbors[i].dir==dir)
			for (unsigned int j = 0; j < incomingNeighbors.size(); j++)
				if ((1<<incomingNeighbors[j].dir2)& checkDirsReversed)
	//for (unsigned int i = 0; i + 1 < neighborhoodVector[sg].size(); i++)
	//	for (unsigned int j = i + 1; j < neighborhoodVector[sg].size(); j++)
		{
			subGoalIdDir sg1 = neighbors.at(i);
			subGoalIdDir sg2 = incomingNeighbors.at(j);

			//if (!IsPruned(sg1.id, dir) || !IsPruned(sg2.id, dir))
			{
				cost hCost = HCost(sg1.id, sg2.id);
				// Only connect sg1-sg2 if sg1 and sg2 are h-reachable and sg is on an h-reachable path
#ifdef USE_FLOAT_DIST
				bool connect = abs(hCost - (HCost(sg, sg1.id) + HCost(sg, sg2.id)))<0.1;
#else
				bool connect = (hCost == HCost(sg, sg1.id) + HCost(sg, sg2.id));
#endif

#ifdef USE_FLOAT_DIST
				if (connect && abs(CostOtherPath(sg, sg1.id, sg2.id, hCost + 1) - hCost)>0.1)
#else
				if (connect )//&& (CostOtherPath(sg, sg1.id, sg2.id, hCost + 1) != hCost))
#endif
				{
					removeFlags[j] = true;
					removeFlags2[i] = true;
					//AddEdge(sg1, sg2);
					AddEdge(sg2, sg1);
				}
			}

		}
	// Since this is now a local subgoal, remove all incoming edges
	for (unsigned int i = 0; i< neighbors.size(); i++)
		//if (removeFlags2[i])
		if (neighbors.at(i).dir == dir)
			RemoveEdge(neighbors.at(i).id, sg, dir);	// Note that they will only be removed from the edgeVectors

	for (unsigned int i = 0; i< incomingNeighbors.size(); i++)
		if (removeFlags[i])
		//if (incomingNeighbors.at(i).dir2 == dir)
			RemoveEdge(incomingNeighbors.at(i).id, sg, dir);


	// Finally, remove all the local-to-local edges
	/*for (unsigned int i = 0; i< edgeVector[sg].size(); i++)
	{
		if (IsPruned(edgeVector[sg].at(i).id, dir) && edgeVector[sg].at(i).dir==dir)
		{
			edgeVector[sg].at(i) = edgeVector[sg].back();
			edgeVector[sg].pop_back();
			i--;
		}
	}*/
}
void BL_JPS_SUBGOAL_EXP::AddEdge(subGoalIdDir sg1, subGoalIdDir sg2)
{
	for (unsigned int i = 0; i < neighborhoodVector[sg1.id].size(); i++)
	{
		if (neighborhoodVector[sg1.id].at(i).id == sg2.id)
			return;
	}
	int originalDir = getDir(sg1.id, sg2.id);// sg2.dir;
	incomingConnectionsVector[sg2.id].push_back( subGoalIdDir(sg1.id,getDir(sg1.id,sg2.id),sg2.dir2,sg2.followUpDirs)  );
	sg2.dir =  sg1.dir;
	sg2.dir2 = sg1.dir2;
	neighborhoodVector[sg1.id].push_back(sg2);
	//sg2.dir = originalDir;// sg1.dir;

//	if (!IsPruned(sg2.id, originalDir))
	{
		edgeVector[sg1.id].push_back(sg2);
	}
}
void BL_JPS_SUBGOAL_EXP::RemoveEdge(subgoalId sg1, subgoalId sg2,char dir)
{
	for (unsigned int i = 0; i < edgeVector[sg1].size(); i++)
	{
		if (edgeVector[sg1].at(i).id == sg2)
		{
			if (edgeVector[sg1].at(i).dir2 == dir)
			{
				edgeVector[sg1].at(i) = edgeVector[sg1].back();
				edgeVector[sg1].pop_back();
			}
			break;
		}
	}
	/*for (unsigned int i = 0; i < incomingConnectionsVector[sg1].size(); i++)
		if (incomingConnectionsVector[sg1].at(i).id == sg2)
		{
			if (incomingConnectionsVector[sg1].at(i).dir == (dir))
			{
				incomingConnectionsVector[sg1].at(i) = incomingConnectionsVector[sg1].back();
				incomingConnectionsVector[sg1].pop_back();
			}
			return;

		}*/
}


void BL_JPS_SUBGOAL_EXP::AddToOpen(subgoalId sg, cost fVal, char dir,unsigned char followUpDir)
{
#ifdef REPLACE_POPPED
	if (canReplaceTop)	// If the top element of the heap can be replaced,
	{
		theHeap[0] = (heapElement(sg, fVal, dir, followUpDir));	// .. replace it
		PercolateDown(0);		// and percolate down
		canReplaceTop = false;	// the top element is no longer replaceable
	}
	else
#endif
	{	// add element as usual
		theHeap.push_back(heapElement(sg, fVal, dir, followUpDir));
		PercolateUp(theHeap.size() - 1);
	}
}
void BL_JPS_SUBGOAL_EXP::PopMin()
{
#ifdef REPLACE_POPPED
	canReplaceTop = true;	// Don't pop it immediately, just mark it as replaceable
#else
	theHeap[0] = theHeap.back();
	theHeap.pop_back();
	PercolateDown(0);
#endif
}
#ifdef REPLACE_POPPED
void BL_JPS_SUBGOAL_EXP::PopReplacableTop()
{	// Force the heap to remove the top element, without waiting for a replacement
	if (canReplaceTop)
	{
		theHeap[0] = theHeap.back();
		theHeap.pop_back();
		if (theHeap.size()>0)
			PercolateDown(0);
		canReplaceTop = false;
	}
}
#endif
heapElement BL_JPS_SUBGOAL_EXP::GetMin()
{
	return theHeap.front();
}

void BL_JPS_SUBGOAL_EXP::ResetSearch()
{
	// Set last search and generated values to 0, so that when the search is incremented, all the states will be not-generated
	search = 0;
	memset(generated, 0, (nSubgoals + 2) * sizeof(uint16_t));
}
void BL_JPS_SUBGOAL_EXP::PercolateUp(int index)
{
	heapElement elem = theHeap[index];
	int parent;
	parent = (index - 1) >> 1;

	while (index > 0 && theHeap[parent].fVal > elem.fVal)
	{
		theHeap[index] = theHeap[parent];
		index = parent;
		parent = (index - 1) >> 1;
	}

	theHeap[index] = elem;
}
void BL_JPS_SUBGOAL_EXP::PercolateDown(int index)
{
	heapElement elem = theHeap[index];
	int maxSize = theHeap.size();

	while (true)
	{
		int child1 = (index << 1) + 1;
		if (child1 >= maxSize)
			break;

		int child2 = child1 + 1;

		// If the first child has smaller key
#ifdef USE_FLOAT_DIST
		if (child2 == maxSize || theHeap[child1].fVal < theHeap[child2].fVal || abs(theHeap[child1].fVal - theHeap[child2].fVal)<0.1)

#else
		if (child2 == maxSize || theHeap[child1].fVal <= theHeap[child2].fVal)

#endif
		{
			if (theHeap[child1].fVal < elem.fVal)
			{
				theHeap[index] = theHeap[child1];
				index = child1;
			}
			else
				break;
		}

		else if (theHeap[child2].fVal < elem.fVal)
		{
			theHeap[index] = theHeap[child2];
			index = child2;
		}
		else
			break;
	}

	theHeap[index] = elem;
}

void BL_JPS_SUBGOAL_EXP::ToXYLocPath(std::vector<mapLoc> & mapLocPath, std::vector<xyLoc> & xyLocPath)
{
	xyLocPath.clear();
	for (unsigned int i = 0; i < mapLocPath.size(); i++)
	{
		xyLocPath.push_back(ToXYLoc(mapLocPath[i]));
	}
}




unsigned char BL_JPS_SUBGOAL_EXP::forcedNeighbours(const xyLoc &coord, const int dir)
{
	if (dir > 7)
		return 0;

	unsigned char dirs = 0;
#define ENTERABLE(n) IsTraversable (ToMapLoc( nextCoordinate (coord, (dir + (n)) &7)))
	if (dir&1) {
		if (!implies(ENTERABLE(6), ENTERABLE(5)))
			dirs = addDirectionToSet(dirs, (dir + 6) % 8);
		if (!implies(ENTERABLE(2), ENTERABLE(3)))
			dirs = addDirectionToSet(dirs, (dir + 2) % 8);

	}
	else {
#ifdef DIAG_UNBLOCKED
		if (!implies(ENTERABLE(2), ENTERABLE(3)))
		{
			dirs = addDirectionToSet(dirs, (dir + 2) % 8);
			dirs = addDirectionToSet(dirs, (dir + 1) % 8);
		}
		if (!implies(ENTERABLE(6), ENTERABLE(5)))
		{
			dirs = addDirectionToSet(dirs, (dir + 6) % 8);
			dirs = addDirectionToSet(dirs, (dir + 7) % 8);
		}
#else
		if (!implies(ENTERABLE(7), ENTERABLE(6)))
			dirs = addDirectionToSet(dirs, (dir + 7) % 8);
		if (!implies(ENTERABLE(1), ENTERABLE(2)))
			dirs = addDirectionToSet(dirs, (dir + 1) % 8);
#endif
	}
#undef ENTERABLE

	return dirs;
}



cost BL_JPS_SUBGOAL_EXP::SubgoalAStarSearch(subgoalId & start, subgoalId & goal, cost searchLimit, std::vector<subgoalId> & abstractPath)
{
	if (search >= MAX_SEARCH)
		ResetSearch();

	search++;
	theHeap.clear();
	theStack.clear();

#ifdef REPLACE_POPPED
	canReplaceTop = false;
#endif

	generated[start] = search;
	gCost[start] = 0;
	SetOpen(start);
	AddToOpen(start, HCost(start, goal),8,0xFF);

	generated[goal] = search;
	gCost[goal] = searchLimit;	// If this point is reached, end the search early (assume no path found)
	SetOpen(goal);
	// DO NOT ADD GOAL TO THE OPEN LIST YET

#ifdef SG_STATISTICS
	int nHeapExpansions = 0;
#ifdef USE_STACK
	int nStackExpansions = 0;
#endif
#endif

	subGoalIdDir* successors;
	unsigned int nSuccessors;
	subGoalIdDir sg;
	int expands = 0;
	int added = 0;
#ifdef USE_STACK
	//heapElement* bestElement = &theHeap[0];
	cost currFCost;

	//while(bestElement && bestElement->fVal < gCost[goal])
	while ((!theStack.empty() && gCost[goal] > theStack[0].fVal) || (!theHeap.empty() && gCost[goal] > theHeap[0].fVal))	// 0-1 Expansions per loop
#else
	while (!theHeap.empty() && gCost[goal] > theHeap[0].fVal)	// 0-1 Expansions per loop
#endif
	{
		expands++;
		// Select the element to expand
#ifdef USE_STACK
		if (!theStack.empty())	// If the stack has elements, expand the top one
		{
			sg = theStack.back().sg;
			currFCost = theStack.back().fVal;
			theStack.pop_back();
#ifdef SG_STATISTICS canReplaceTop
			nStackExpansions++;
#endif
		}
		else	// Expand from the heap
		{
			sg = theHeap[0].sg;
			currFCost = theHeap[0].fVal;
			PopMin();
#ifdef SG_STATISTICS
			nHeapExpansions++;
#endif
		}
#else	// We are only using the heap
		sg = subGoalIdDir(theHeap[0].sg, theHeap[0].dir, 0, theHeap[0].followUpDir);
		PopMin();
#ifdef SG_STATISTICS
		nHeapExpansions++;
#endif
#endif

		if (IsOpen(sg.id))	// If it has been closed already, don't re-expand it
		{
			// Expand the state

			SetClosed(sg.id);
			unsigned char expandDirs = 0xFF;
			//vector<pair<subGoalIdDir,xyLoc>> allNeighbours;

			// Get the successors
			if (finalized)	// If the graph is finalized, use 'neighbors' to get the successors
			{
				successors = neighbors[sg.id];
				nSuccessors = nNeighbors[sg.id];
				//unsigned char expandDirs2 = forcedNeighbours(location[sg.id], sg.dir) | naturalNeighbours(sg.dir);
				//expandDirs2 = sg.followUpDirs == 0xff ? 0xff : expandDirs2;
				expandDirs = sg.followUpDirs;
				//if (expandDirs2 != sg.followUpDirs && sg.followUpDirs!=0xFF)
				//	int a = 1;
				//for (int i = 0; i < nSuccessors; i++)
				//	allNeighbours.push_back(pair<subGoalIdDir, xyLoc>(successors[i], location[successors[i].id]));
				int a = 1;
			}
			else	// Else, use 'edgeVector' to get the successors
			{
				successors = edgeVector[sg.id].data();
				nSuccessors = edgeVector[sg.id].size();
			}
			// Go over all the successors
			for (unsigned int i = 0; i < nSuccessors; i++)
			{
				subGoalIdDir succ = successors[i];
			//	if (getDir2(sg.id, succ.id) != succ.dir2 && succ.dir2 != 8)
			//		int a = 1;
				if ((1<<succ.dir2) &expandDirs || succ.dir2==8)
				{
					// Generate the successor, if it has not been generated for this search
					if (generated[succ.id] != search)
					{
						generated[succ.id] = search;
						gCost[succ.id] = gCost[sg.id] + HCost(sg.id, succ.id);
						parent[succ.id] = sg.id;
						SetOpen(succ.id);

						// Insert it into the open list
	#ifdef USE_STACK
	#ifdef USE_FLOAT_DIST
						if (abs(gCost[succ] + HCost(succ, goal) - currFCost)<0.1)
	#else
						if (gCost[succ] + HCost(succ, goal) == currFCost)

	#endif
							theStack.push_back(heapElement(succ, gCost[succ] + HCost(succ, goal)));
						else
							AddToOpen(succ, gCost[succ] + HCost(succ, goal));
	#else

						AddToOpen(succ.id, gCost[succ.id] + HCost(succ.id, goal), succ.dir2, succ.followUpDirs);
	#endif
						added++;
					}

					// If it was already generated and is not closed
					else if (IsOpen(succ.id))
					{
						cost newGCost = gCost[sg.id] + HCost(sg.id, succ.id);
						if (newGCost < gCost[succ.id])
						{
							gCost[succ.id] = newGCost;
							parent[succ.id] = sg.id;

							// Insert it into the open list (may add code to delete the old version with the higher f-value from the open list)
	#ifdef USE_STACK
	#ifdef USE_FLOAT_DIST
							if (abs(gCost[succ] + HCost(succ, goal) - currFCost)<0.1)
	#else
							if (gCost[succ] + HCost(succ, goal) == currFCost)

	#endif
								//if (gCost[succ] + HCost(succ, goal) == currFCost)
								theStack.push_back(heapElement(succ, gCost[succ] + HCost(succ, goal)));
							else
								AddToOpen(succ, gCost[succ] + HCost(succ, goal));
	#else
							AddToOpen(succ.id, gCost[succ.id] + HCost(succ.id, goal), succ.dir2, succ.followUpDirs);//succ.dir
	#endif
							added++;
						}
					}
				}
			}
		}
#ifdef REPLACE_POPPED
		PopReplacableTop();
#endif
	}

#ifdef SG_STATISTICS
#ifdef SG_STATISTICS_PER_SEARCH
	if (finalized)
	{
		std::cout << "Heap expansions: " << nHeapExpansions << std::endl;
#ifdef USE_STACK
		std::cout << "Stack expansions: " << nStackExpansions << std::endl;
#endif
		std::cout << "Open list size: " << theHeap.size() + theStack.size() << std::endl;
	}
#endif
#endif

	if (gCost[goal] != searchLimit)	// Follow the parent pointers to extract the path
	{
		abstractPath.clear();
		subgoalId cur = goal;
		while (cur != start)
		{
			abstractPath.push_back(cur);
			cur = parent[cur];
		}

		abstractPath.push_back(cur);
		//std::reverse(abstractPath.begin(), abstractPath.end());

		return gCost[goal];
	}
	else
	{
		return INFINITE_COST;
	}
}
char BL_JPS_SUBGOAL_EXP::getDir(int from, int to)
{
	xyLoc fromLoc = location[from];
	xyLoc toLoc = location[to];
	if (fromLoc.x == toLoc.x)
		return fromLoc.y < toLoc.y ? 4 : 0;
	if (fromLoc.y == toLoc.y)
		return fromLoc.x < toLoc.x ? 2 : 6;
	if (fromLoc.y < toLoc.y)
		return fromLoc.x < toLoc.x ? 3 : 5;
	else
		return fromLoc.x < toLoc.x ? 1 : 7;
}
char BL_JPS_SUBGOAL_EXP::getDir2(int from, int to)
{
	xyLoc fromLoc = location[from];
	xyLoc toLoc = location[to];
	int dx = toLoc.x - fromLoc.x;
	int dy = toLoc.y - fromLoc.y;
	if (abs(dx) == abs(dy))
		if (dy>0)
			return dx>0 ? 3 : 5;
		else
			return dx>0 ? 1 : 7;
	if (abs(dx)>abs(dy))
		return dx>0 ? 2 : 6;
	return dy>0 ? 4 : 0;
}
char BL_JPS_SUBGOAL_EXP::getDir(const Coordinate& c, const Coordinate& nxt)
{
	int xDif = nxt.x - c.x;
	int yDif = nxt.y - c.y;
	if (xDif == 0)
		return yDif < 0 ? 0 : 4;
	if (yDif == 0)
		return xDif < 0 ? 6 : 2;
	return xDif<0 ? (yDif<0 ? 7 : 5) : yDif<0 ? 1 : 3;
}
char BL_JPS_SUBGOAL_EXP::getDir2(const Coordinate& c, const Coordinate& nxt)
{
	int xDif = nxt.x - c.x;
	int yDif = nxt.y - c.y;

	return xDif<0 ? (yDif<0 ? 7 : 5) : yDif<0 ? 1 : 3;
}
void BL_JPS_SUBGOAL_EXP::ConnectStartAndGoalToGraph(
	xyLoc & start, xyLoc & goal, subgoalId & sgStart, subgoalId & sgGoal,
	std::vector<subGoalIdDir> & startDirectHReachableSubgoals, std::vector<subGoalIdDir> & goalDirectHReachableSubgoals, std::vector<subgoalId> & extraEdgeList)
{
	mapLoc mapStart = ToMapLoc(start);
	mapLoc mapGoal = ToMapLoc(goal);


	//startDirectHReachableSubgoals.clear();
	if (IsSubgoal(mapStart))	// If the start location is already a subgoal, use it
		sgStart = cellInfo[mapStart];

	else	// Create a new subgoal for it, with id nSubgoals (at the end of all the actual subgoals)
	{
		sgStart = nSubgoals;
		location[sgStart] = start;

		for (int i = 0; i < 8; i++)
			jumpNew(start, i, startDirectHReachableSubgoals);

		neighbors[sgStart] = startDirectHReachableSubgoals.data();
		nNeighbors[sgStart] = startDirectHReachableSubgoals.size();
	}
	//if (goalDirectHReachableSubgoals.size())
	//	return;
	{
		sgGoal = nSubgoals + 1;
		location[sgGoal] = goal;
		//GetDirectHReachableSubgoals(goal, goalDirectHReachableSubgoals);
		getAllEndNodes(location[sgGoal].x, location[sgGoal].y, goalDirectHReachableSubgoals);
		// First, add incoming edges from all the directHReachableSubgoals
		for (unsigned int i = 0; i < goalDirectHReachableSubgoals.size(); i++)
		{
			subGoalIdDir sg = goalDirectHReachableSubgoals[i];

			neighbors[sg.id][nNeighbors[sg.id]] = subGoalIdDir(sgGoal, 8);
			nNeighbors[sg.id]++;
			SetExtraEdgeFlag(sg.id);
			extraEdgeList.push_back(sg.id);
			gCost[sg.id] = HCost(sg.id, sgGoal);	// Also use the g-value of that state to store its h-value (needed for the next step)
		}
		gCost[sgGoal] = 0;
		SetExtraEdgeFlag(sgGoal); // So that in the next step nobody tries to add new edges for sgGoal

		/* Next, make sure that all the pruned subgoals among the directHReachableSubgoals have incoming edges.
		* Note that, if we arbitrarily add edges, we may need to add more than one edge for a subgoal.
		* Instead, we store only one edge, who can be followed to optimally reach the goal.
		*/
		for (unsigned int i = 0; i < goalDirectHReachableSubgoals.size(); i++)
		{
			subGoalIdDir sg = goalDirectHReachableSubgoals[i];
		//	sg.dir = (sg.dir + 4) & 7;
			//if (IsPruned(sg.id,(sg.dir+4)&7))	// Only do the additional step for pruned subgoals
			{
			//	for (unsigned int j = 0; j < nNeighbors[sg.id]; j++)
				char newDir = getDir(sgGoal, sg.id);
				newDir = forcedNeighbours(location[sg.id], newDir) | naturalNeighbours(newDir);
				newDir = reverseForced(newDir);
				std::vector<subGoalIdDir> & neighbours = incomingConnectionsVector[sg.id];
				for (unsigned int j = 0; j < neighbours.size(); j++)

				{
					//subGoalIdDir sg2 = neighbors[sg.id][j];
					subGoalIdDir sg2 = neighbours[j];
					if (newDir&(1<<sg2.dir2))
					{
						if (!HasExtraEdge(sg2.id))	// Has no other extra edge, just make sg2->sg the extra edge
						{
							neighbors[sg2.id][nNeighbors[sg2.id]] = subGoalIdDir(sg.id, getDir(sg2.id, sg.id));
							nNeighbors[sg2.id]++;
							SetExtraEdgeFlag(sg2.id);
							extraEdgeList.push_back(sg2.id);
							gCost[sg2.id] = HCost(sg.id, sg2.id) + gCost[sg.id];	// Store the cost of the best known path to the goal
						}

						else	// Already has an extra edge, make sg2->sg the extra edge only if it yields a shorter path
						{
							cost newCost = HCost(sg.id, sg2.id) + gCost[sg.id];
							if (newCost < gCost[sg2.id])
							{
								neighbors[sg2.id][nNeighbors[sg2.id] - 1] = subGoalIdDir(sg.id, getDir(sg2.id, sg.id));	// We subtract 1 because nNeighbors is already updated
								gCost[sg2.id] = newCost;
							}
						}
					}
				}

				if (keepLocalEdges)	// Also add the local-to-local edges
					for (unsigned int j = 0; j < nLocalNeighbors[sg.id]; j++)
					{
						subGoalIdDir sg2 = localNeighbors[sg.id][j];

						if (!HasExtraEdge(sg2.id))	// Has no other extra edge, just make sg2->sg the extra edge
						{
							neighbors[sg2.id][nNeighbors[sg2.id]] = subGoalIdDir(sg.id, getDir(sg2.id, sg.id));
							nNeighbors[sg2.id]++;
							SetExtraEdgeFlag(sg2.id);
							extraEdgeList.push_back(sg2.id);
							gCost[sg2.id] = HCost(sg.id, sg2.id) + gCost[sg.id];	// Store the cost of the best known path to the goal
						}

						else	// Already has an extra edge, make sg2->sg the extra edge only if it yields a shorter path
						{
							cost newCost = HCost(sg.id, sg2.id) + gCost[sg.id];
							if (newCost < gCost[sg2.id])
							{
								neighbors[sg2.id][nNeighbors[sg2.id] - 1] = subGoalIdDir(sg.id, getDir(sg2.id, sg.id));	// We subtract 1 because nNeighbors is already updated
								gCost[sg2.id] = newCost;
							}
						}
					}
			}
		}
	}	// Initialized

}

cost BL_JPS_SUBGOAL_EXP::CheckCommonLocal(subgoalId & sgStart, subgoalId & sgGoal,
	std::vector<subGoalIdDir> & startDirectHReachableSubgoals, std::vector<subGoalIdDir> & goalDirectHReachableSubgoals, std::vector<subgoalId> & localPath)
{
	static std::vector<subgoalId> locals;
	locals.clear();
	localPath.clear();
	cost minCost = INFINITE_COST;
	subgoalId connectingSg;


	for (int i = 0; i < goalDirectHReachableSubgoals.size(); i++)
		if (IsPruned(goalDirectHReachableSubgoals[i].id))
			locals.push_back(goalDirectHReachableSubgoals[i].id);

	for (int i = 0; i < startDirectHReachableSubgoals.size(); i++)
		if (IsPruned(startDirectHReachableSubgoals[i].id))
		{
			for (int j = 0; j < goalDirectHReachableSubgoals.size(); j++)
			{
				if (goalDirectHReachableSubgoals[j].id == startDirectHReachableSubgoals[i].id)
				{
					cost costThrough = HCost(sgStart, goalDirectHReachableSubgoals[j].id) + HCost(goalDirectHReachableSubgoals[j].id, sgGoal);
					if (costThrough < minCost)
					{
						minCost = costThrough;
						connectingSg = goalDirectHReachableSubgoals[j].id;
					}
					break;
				}
			}
		}

	if (minCost != INFINITE_COST)
	{
		localPath.push_back(sgStart);
		localPath.push_back(connectingSg);
		localPath.push_back(sgGoal);
	}
	return minCost;
}

cost BL_JPS_SUBGOAL_EXP::CheckAllLocalPairs(subgoalId & sgStart, subgoalId & sgGoal,
	std::vector<subgoalId> & startDirectHReachableSubgoals, std::vector<subgoalId> & goalDirectHReachableSubgoals, std::vector<subgoalId> & localPath)
{
	// check for any connecting pair (don't try to find the best one)
	//static std::vector<subgoalId> locals;
	//locals.clear();
	//localPath.clear();
	//cost minCost = INFINITE_COST;

	for (int i = 0; i < startDirectHReachableSubgoals.size(); i++)
		if (IsPruned(startDirectHReachableSubgoals[i]))
		{
			for (int j = 0; j < goalDirectHReachableSubgoals.size(); j++)
			{
				if (IsHReachable(location[goalDirectHReachableSubgoals[j]], location[startDirectHReachableSubgoals[i]]))
				{
					localPath.clear();
					localPath.push_back(sgStart);
					localPath.push_back(startDirectHReachableSubgoals[i]);
					localPath.push_back(goalDirectHReachableSubgoals[j]);
					localPath.push_back(sgGoal);

					return 	HCost(sgStart, startDirectHReachableSubgoals[i]) +
						HCost(startDirectHReachableSubgoals[i], goalDirectHReachableSubgoals[j]) +
						HCost(goalDirectHReachableSubgoals[j], sgGoal);
				}
			}
		}

	return INFINITE_COST;
}
cost BL_JPS_SUBGOAL_EXP::TryLocalPair(subgoalId & sgStart, subgoalId & sgGoal,
	std::vector<subgoalId> & startDirectHReachableSubgoals, std::vector<subgoalId> & goalDirectHReachableSubgoals, std::vector<subgoalId> & localPath)
{
	localPath.clear();

	if ((sgStart == nSubgoals || IsPruned(sgStart)) && (sgGoal == nSubgoals + 1 || IsPruned(sgGoal)))
	{
		subgoalId startLocal;
		cost startCost;

		if (sgStart != nSubgoals)	// then start itself is the local subgoal we were looking for
		{
			startLocal = sgStart;
			startCost = 0;
		}
		else	// then start is a newly added subgoal, look through its local neighbors
		{
			startLocal = nSubgoals + 2; // uninitialized
			startCost = INFINITE_COST;

			for (unsigned int i = 0; i < startDirectHReachableSubgoals.size(); i++)
			{
				if (IsPruned(startDirectHReachableSubgoals[i]))
				{
					cost costThrough = HCost(sgStart, startDirectHReachableSubgoals[i]) + HCost(startDirectHReachableSubgoals[i], sgGoal);
					if (costThrough < startCost)
					{
						startLocal = startDirectHReachableSubgoals[i];
						startCost = costThrough;
					}
				}
			}
		}
		if (startLocal == nSubgoals + 2)
			return INFINITE_COST;

		subgoalId goalLocal;
		cost goalCost;

		if (sgGoal != nSubgoals + 1)	// then start itself is the local subgoal we were looking for
		{
			goalLocal = sgGoal;
			goalCost = 0;
		}
		else	// then start is a newly added subgoal, look through its local neighbors
		{
			goalLocal = nSubgoals + 2; // uninitialized
			goalCost = INFINITE_COST;

			for (unsigned int i = 0; i < goalDirectHReachableSubgoals.size(); i++)
			{
				if (IsPruned(goalDirectHReachableSubgoals[i]))
				{
					cost costThrough = HCost(sgStart, goalDirectHReachableSubgoals[i]) + HCost(goalDirectHReachableSubgoals[i], sgGoal);
					if (costThrough < goalCost)
					{
						goalLocal = goalDirectHReachableSubgoals[i];
						goalCost = costThrough;
					}
				}
			}
		}

		if (goalLocal == nSubgoals + 2)
			return INFINITE_COST;

		if (startLocal == goalLocal)	// then the same path will also be found by the actual subgoal graph search
			return INFINITE_COST;

		if (IsHReachable(location[startLocal], location[goalLocal]))
			//if (IsLookaheadHReachable(location[startLocal], location[goalLocal]))
		{
			cost localCost = HCost(startLocal, goalLocal);
			if (startLocal != sgStart)
			{
				localPath.push_back(sgStart);
				localCost += HCost(sgStart, startLocal);
			}

			localPath.push_back(startLocal);
			localPath.push_back(goalLocal);

			if (goalLocal != sgGoal)
			{
				localPath.push_back(sgGoal);
				localCost += HCost(goalLocal, sgGoal);
			}

			return localCost;
		}
	}

	return INFINITE_COST;
}

void BL_JPS_SUBGOAL_EXP::AppendOptimalPath(subgoalId sg1, subgoalId sg2, std::vector<subgoalId> & path)
{
	if (((sg1>sg2) ? dist[sg1][sg2] : dist[sg2][sg1]) == INFINITE_COST)
		return;
	std::vector<subgoalId> endPoints;
	while (sg1 != sg2)
	{
		cost c = (sg1>sg2) ? dist[sg1][sg2] : dist[sg2][sg1];
		bool added = false;
		for (int i = 0; i < nNeighbors[sg1]; i++)
		{
			int sg = neighbors[sg1][i].id;
			cost c1 = (sg>sg1) ? dist[sg][sg1] : dist[sg1][sg];
			cost c2 = (sg>sg2) ? dist[sg][sg2] : dist[sg2][sg];
#ifdef USE_FLOAT_DIST
			if (abs(c1 + c2 - c)<0.1)
#else
			if (c1 + c2 == c)
#endif
			{

				sg1 = sg;
				path.push_back(sg1);

				added = true;
				break;
			}
		}
		/*if (!added)
		{
			for (int i = 0; i < incomingConnectionsVector[sg1].size(); i++)
			{
				int sg = incomingConnectionsVector[sg1][i].id;
				cost c1 = (sg>sg1) ? dist[sg][sg1] : dist[sg1][sg];
				cost c2 = (sg>sg2) ? dist[sg][sg2] : dist[sg2][sg];
#ifdef USE_FLOAT_DIST
				if (abs(c1 + c2 - c)<0.1)
#else
				if (c1 + c2 == c)
#endif
				{
					//path.push_back(sg1);

					sg1 = sg;
					path.push_back(sg1);

					added = true;
					break;
				}
			}
			//sg1 = sg2;
		}
		if (!added)
		{
			for (int i = 0; i < incomingConnectionsVector[sg2].size(); i++)
			{
				int sg = incomingConnectionsVector[sg2][i].id;
				cost c1 = (sg>sg1) ? dist[sg][sg1] : dist[sg1][sg];
				cost c2 = (sg>sg2) ? dist[sg][sg2] : dist[sg2][sg];
#ifdef USE_FLOAT_DIST
				if (abs(c1 + c2 - c)<0.1)
#else
				if (c1 + c2 == c)
#endif
				{
					//path.push_back(sg1);
					endPoints.push_back(sg2);

					sg2 = sg;
					added = true;
					break;
				}
			}
			//sg1 = sg2;
		}*/
		if (!added)
		{
			for (int i = 0; i < nNeighbors[sg2]; i++)
			{
				int sg = neighbors[sg2][i].id;
				cost c1 = (sg>sg1) ? dist[sg][sg1] : dist[sg1][sg];
				cost c2 = (sg>sg2) ? dist[sg][sg2] : dist[sg2][sg];
#ifdef USE_FLOAT_DIST
				if (abs(c1 + c2 - c)<0.1)
#else
				if (c1 + c2 == c)
#endif
				{
					endPoints.push_back(sg2);
					sg2 = sg;
					added = true;
					break;
				}
			}
			//sg1 = sg2;
		}


	}
	for (int i = (int)(endPoints.size()) - 1; i > -1; i--)
		path.push_back(endPoints[i]);

}
void BL_JPS_SUBGOAL_EXP::GetGlobalConnections(bool start, xyLoc & loc, subgoalId & locSg, std::vector<subGoalIdDir> & directConnections, std::vector<cost> & goalGlobalCosts)
{
	directConnections.clear();
	goalGlobalCosts.clear();
	//distToOrigin.clear();
	//linkToLocal.clear();

	mapLoc mLoc = ToMapLoc(loc);

	if (start &&IsSubgoal(mLoc))
	{
		locSg = cellInfo[mLoc];
		location[locSg] = loc;
		if (locSg < nGlobalSubgoals)	// It is a global subgoal
		{
			directConnections.push_back(subGoalIdDir(locSg,8));
			goalGlobalCosts.push_back(0);
		}

	}
	else	// not a subgoal
	{
		// use locSg's default value for the subgoal position
		location[locSg] = loc;
		//GetDirectHReachableSubgoals(loc, directConnections);
		if (start)
			for (int i = 0; i < 8; i++)
				jumpNew(loc, i, directConnections);
		else
			getAllEndNodes(loc.x, loc.y, directConnections);

		for (unsigned int i = 0; i < directConnections.size(); i++)
			goalGlobalCosts.push_back(HCost(directConnections[i].id, locSg));
		// First use the hasExtraEdge, gCost, and the extra edge slots to connect 'loc' to all the necessary global subgoals
		// We don't actually add or remove edges, we just use them for book keeping

		// Go over all the direct connections
		/*for (unsigned int i = 0; i < directConnections.size(); i++)
		{
			subgoalId sg = directConnections[i].id;
			if (!IsPruned(sg))	// If it is a global subgoal, say that its directly connected to the start
			{
				neighbors[sg][nNeighbors[sg]] = subGoalIdDir(locSg,8);	// meanint it is not a subgoal
				SetExtraEdgeFlag(sg);
				//globalSubgoals.push_back(sg);
			}
			gCost[sg] = HCost(sg, locSg);	// Also use the g-value of that state to store its h-value (needed for the next step)
		}

		for (unsigned int i = 0; i < directConnections.size(); i++)	// Go over all the direct connections again
		{
			subgoalId sg = directConnections[i].id;

			if (IsPruned(sg))	// If it is a local subgoal, check its global connections
			{
				for (unsigned int j = 0; j < nNeighbors[sg]; j++)
				{
					subGoalIdDir sg2 = neighbors[sg][j];

					if (!HasExtraEdge(sg2.id))	// If it is not yet connected to the 'loc' (either directly or through a local subgoal)
					{
						neighbors[sg2.id][nNeighbors[sg2.id]] = subGoalIdDir(sg,8);	// Make sg the best known connection
						SetExtraEdgeFlag(sg2.id);	// Mark it as connected
						gCost[sg] = HCost(sg, sg2.id) + gCost[sg];	// Store the cost of the best known path to the 'loc'
						//globalSubgoals.push_back(sg2.id);
					}

					else	// Already has a connection, check if a connection through sg is better
					{
						cost newCost = HCost(sg, sg2.id) + gCost[sg];
						if (newCost < gCost[sg2.id])	// Update the connection if necessary
						{
							neighbors[sg2.id][nNeighbors[sg2.id]] = subGoalIdDir(sg, 8);
							gCost[sg2.id] = newCost;
						}
					}
				}
			}
		}*/

		// Now, go over all the found global connections and collect the information (and clear the extra edge flags)
		/*for (int i = 0; i < globalSubgoals.size(); i++)
		{
			subgoalId sg = globalSubgoals[i];
			RemoveExtraEdgeFlag(sg);
//			distToOrigin.push_back(gCost[sg]);
			//linkToLocal.push_back(neighbors[sg][nNeighbors[sg]].id);
		}*/
	}
}
cost BL_JPS_SUBGOAL_EXP::LookupOptimalPath(xyLoc & startLoc, xyLoc & goalLoc, subgoalId & start, subgoalId & goal, std::vector<subGoalIdDir> & startDirectConnections, std::vector<subGoalIdDir> & goalDirectConnections, std::vector<subgoalId> & path)
{
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif

	path.clear();

	// First step: Find the global subgoals connected to the start and goal (and their connection costs)
	static std::vector<subgoalId> startSubgoals;
	static std::vector<subgoalId> goalSubgoals;
	static std::vector<cost> startGlobalSubgoalCosts;
	static std::vector<cost> goalGlobalSubgoalCosts;
	//static std::vector<subgoalId> startLinkToLocal;
	//static std::vector<subgoalId> goalLinkToLocal;

	start = nSubgoals;	// First, assume these will be the subgoal locations
	goal = nSubgoals + 1;

	GetGlobalConnections(true, startLoc, start, startDirectConnections,startGlobalSubgoalCosts);
	GetGlobalConnections(false, goalLoc, goal, goalDirectConnections, goalGlobalSubgoalCosts);

#ifdef SG_STATISTICS
	double elapsed = t.EndTimer();
	initializeTime += elapsed;
	searchTime -= elapsed;
#endif

	/*if (start < nGlobalSubgoals && goal < nGlobalSubgoals)	// If both start and goal are already global subgoals
	{
		cost minCost = (start > goal) ? dist[start][goal] : dist[goal][start];
		if (minCost != INFINITE_COST)
			AppendOptimalPath(start, goal, path);

		return minCost;
	}*/

	/*if (start < nGlobalSubgoals)	// If start is already a global subgoal but goal is not
	{
		cost minCost = INFINITE_COST;
		int connectingSg = 0;

		for (unsigned int i = 0; i < goalSubgoals.size(); i++)
		{
			subgoalId sg = goalSubgoals[i];
			cost currCost = ((start > sg) ? dist[start][sg] : dist[sg][start]);
			currCost = (currCost < INFINITE_COST) ? (currCost + goalGlobalSubgoalCosts[i]) : currCost;

			if (currCost < minCost)
			{
				minCost = currCost;
				connectingSg = i;
			}
		}

		if (minCost != INFINITE_COST)
		{
			AppendOptimalPath(start, goalSubgoals[connectingSg], path);
			if (goalLinkToLocal[connectingSg] != goal)
				path.push_back(goalLinkToLocal[connectingSg]);
			path.push_back(goal);
		}

		return minCost;
	}*/

	/*if (goal < nGlobalSubgoals)	// If goal is already in the graph
	{
		cost minCost = INFINITE_COST;
		int connectingSg = 0;

		for (unsigned int i = 0; i < startSubgoals.size(); i++)
		{
			subgoalId sg = startSubgoals[i];
			cost currCost = ((goal > sg) ? dist[goal][sg] : dist[sg][goal]);
			currCost = (currCost < INFINITE_COST) ? (currCost + startGlobalSubgoalCosts[i]) : currCost;

			if (currCost < minCost)
			{
				minCost = currCost;
				connectingSg = i;
			}
		}

		if (minCost != INFINITE_COST)
		{
			path.push_back(start);
			if (startLinkToLocal[connectingSg] != start)
				path.push_back(startLinkToLocal[connectingSg]);
			AppendOptimalPath(startSubgoals[connectingSg], goal, path);
		}

		return minCost;
	}*/

	// Both start and goal are not global subgoals
	cost minCost = INFINITE_COST;
	int connectingSg1 = 0;
	int connectingSg2 = 0;
	int connectingSg3 = -1;
	int currentStartSize = startDirectConnections.size();
	vector<int> map;

	for (unsigned int i = 0; i < currentStartSize; i++)
	{
		subgoalId sg1 = startDirectConnections[i].id;
		for (unsigned int j = 0; j < nNeighbors[sg1]; j++)
			if (((1 << neighbors[sg1][j].dir)&pruned[neighbors[sg1][j].id]))
			{
				startDirectConnections.push_back(neighbors[sg1][j]);
				startGlobalSubgoalCosts.push_back(startGlobalSubgoalCosts[i] + HCost(neighbors[sg1][j].id,sg1));
				map.push_back(sg1);
			}
	}


	for (unsigned int i = 0; i < startDirectConnections.size(); i++)
	{
		subgoalId sg1 = startDirectConnections[i].id;
		cost hStart =  startGlobalSubgoalCosts[i];

		for (unsigned int j = 0; j < goalDirectConnections.size(); j++)
		{
			subgoalId sg2 = goalDirectConnections[j].id;
			cost currCost = ((sg1 > sg2) ? dist[sg1][sg2] : dist[sg2][sg1]);
			currCost = (currCost < INFINITE_COST) ? (hStart + currCost + goalGlobalSubgoalCosts[j]) : currCost;

			if (currCost < minCost)
			{
				minCost = currCost;
				connectingSg1 = i;
				connectingSg2 = j;
				connectingSg3 = -1;
			}
				if (IsPruned(sg2))
				{
					for (int k = 0; k < incomingConnectionsVector[sg2].size(); k++)
						if (((1<<incomingConnectionsVector[sg2][k].dir2)&pruned[sg2]))
						{

							subgoalId sg3 = incomingConnectionsVector[sg2][k].id;
							cost currCost = ((sg1 > sg3) ? dist[sg1][sg3] : dist[sg3][sg1]);
							cost goalConnectionCost = HCost(sg2,sg3);
							currCost = (currCost < INFINITE_COST) ? (hStart + currCost + goalGlobalSubgoalCosts[j] + goalConnectionCost) : currCost;

							if (currCost < minCost)
							{
								minCost = currCost;
								connectingSg1 = i;
								connectingSg2 = j;
								connectingSg3 = sg3;
							}

						}
				}
		}
	}
	//unsigned char prune = pruned[connectingSg2];
	if (minCost != INFINITE_COST)
	{
		path.push_back(start);
		if (connectingSg1 >= currentStartSize)  path.push_back(map[connectingSg1 - currentStartSize]);
		if (startDirectConnections[connectingSg1].id != start)	path.push_back(startDirectConnections[connectingSg1].id);

		AppendOptimalPath(startDirectConnections[connectingSg1].id, connectingSg3 == -1 ? goalDirectConnections[connectingSg2].id : connectingSg3, path);
		//if (connectingSg3 != -1)		path.push_back(connectingSg3);

		if (goalDirectConnections[connectingSg2].id != goal &&goalDirectConnections[connectingSg2].id != path.back())		path.push_back(goalDirectConnections[connectingSg2].id);
		path.push_back(goal);
	}

	return minCost;
}
int BL_JPS_SUBGOAL_EXP::getSpaceIdY(short spaceX, short spaceY)
{
	for (int i = 0; i<yBoundaryPoints[spaceX].size() - 1; i++)
		if (yBoundaryPoints[spaceX][i] <= spaceY && yBoundaryPoints[spaceX][i + 1]>spaceY)
			return i;
	return -1;
}

bool BL_JPS_SUBGOAL_EXP::isSpaceIdY(int spaceId, short spaceX, short spaceY)
{
	if (spaceId == -1)
	{
		if (yBoundaryPoints[spaceX].size()>1)
		{
			//int i=yBoundaryPoints[spaceX].size()-2;
			if (yBoundaryPoints[spaceX][0]>spaceY)
				return true;
			else
				return false;
		}
		else
			return true;
	}
	else
		if (yBoundaryPoints[spaceX][spaceId] <= spaceY && yBoundaryPoints[spaceX][spaceId + 1]>spaceY)
			return true;
	return false;
}

int BL_JPS_SUBGOAL_EXP::getSpaceIdX(short spaceX, short spaceY)
{
	for (int i = 0; i<xBoundaryPoints[spaceY].size() - 1; i++)
		if (xBoundaryPoints[spaceY][i] <= spaceX && xBoundaryPoints[spaceY][i + 1]>spaceX)
			return i;
	return -1;
}

bool BL_JPS_SUBGOAL_EXP::isSpaceIdX(int spaceId, short spaceX, short spaceY)
{
	if (spaceId == -1)
	{
		if (xBoundaryPoints[spaceY].size()>1)
		{
			//int i=xBoundaryPoints[spaceY].size()-3;
			if (xBoundaryPoints[spaceY][0]>spaceX)
				return true;
			else
				return false;
		}
		else
			return true;
	}
	else
		if (xBoundaryPoints[spaceY][spaceId] <= spaceX && xBoundaryPoints[spaceY][spaceId + 1]>spaceX)
			return true;
	return false;
}

bool BL_JPS_SUBGOAL_EXP::directSolution(short sX, short sY, short eX, short eY, vector<Coordinate> & sol)
{
	if (sY == eY)
	{
		if (isSpaceIdX(getSpaceIdX(sX, sY), eX, eY))
		{
			sol.push_back(Coordinate(eX, eY));
			sol.push_back(Coordinate(sX, sY));

			return true;
		}
}
	else if (sX == eX)
	{
		if (isSpaceIdY(getSpaceIdY(sX, sY), eX, eY))
		{
			sol.push_back(Coordinate(eX, eY));
			sol.push_back(Coordinate(sX, sY));

			return true;
		}
	}
	else
	{
		bool isDiagOnly = (abs(sX - eX) - abs(sY - eY)) == 0;

		int diagMovements = min(abs(sX - eX), abs(sY - eY)) - 1;//we -1 as we don't need to check the original or destination points as they are checked earlier
		int mx = sX - eX<0 ? -1 : 1;
		int my = sY - eY<0 ? -1 : 1;
		Coordinate offset(sX - eX<0 ? 1 : -1, sY - eY<0 ? 1 : -1);
		Coordinate check(sX, sY);
		bool bPass = true;
		if (!isDiagOnly)
		{
			Coordinate check2(sX + offset.x*(diagMovements + 1), sY + offset.y*(diagMovements + 1));
			if (abs(sX - eX) < abs(sY - eY))
			{
				if (!isSpaceIdY(getSpaceIdY(check2.x, check2.y), eX, eY))
					bPass = false;
			}
			else
				if (!isSpaceIdX(getSpaceIdX(check2.x, check2.y), eX, eY))
					bPass = false;
		}
		if (bPass)
		{
			while (diagMovements)
			{
				diagMovements--;
				bool b = true;
#ifdef DIAG_UNBLOCKED
				b = (IsTraversable(Coordinate(check.x + offset.x, check.y)) && IsTraversable(Coordinate(check.x, check.y + offset.y)));
#endif

				check.add(offset);
				if (!IsTraversable(check) || !b)
				{
					bPass = false;
					diagMovements = 0;
					break;
				}
			}
#ifdef DIAG_UNBLOCKED
			if (bPass && min(abs(sX - eX), abs(sY - eY)) != 0)
				bPass = (IsTraversable(Coordinate(check.x + offset.x, check.y)) && IsTraversable(Coordinate(check.x, check.y + offset.y)));
#endif

			if (bPass)
				if (isDiagOnly)//only diagonal movement
				{
					sol.push_back(Coordinate(eX, eY));
					sol.push_back(Coordinate(sX, sY));
					return true;
				}
				else //diagonal movement and then horiz/vertic
				{
					check.add(offset);
					sol.push_back(Coordinate(eX, eY));

					sol.push_back(Coordinate(check.x, check.y));
					sol.push_back(Coordinate(sX, sY));

					return true;
				}
		}
		if (isDiagOnly)
			return false;
		bPass = true;
		diagMovements = min(abs(sX - eX), abs(sY - eY)) - 1;
		offset.x *= -1;
		offset.y *= -1;
		check = Coordinate(eX, eY);
		Coordinate check2(eX + offset.x*(diagMovements + 1), eY + offset.y*(diagMovements + 1));
		if (abs(sX - eX) < abs(sY - eY))
		{
			if (!isSpaceIdY(getSpaceIdY(check2.x, check2.y), sX, sY))
				bPass = false;
		}
		else
			if (!isSpaceIdX(getSpaceIdX(check2.x, check2.y), sX, sY))
				bPass = false;

		if (bPass)
		{
			while (diagMovements)
			{
				diagMovements--;
				bool b = true;
#ifdef DIAG_UNBLOCKED
				b = (IsTraversable(Coordinate(check.x + offset.x, check.y)) && IsTraversable(Coordinate(check.x, check.y + offset.y)));
#endif
				check.add(offset);
				if (!IsTraversable(check) || !b)
				{
					bPass = false;
					diagMovements = 0;
					break;
				}
			}
#ifdef DIAG_UNBLOCKED
			if (bPass && min(abs(sX - eX), abs(sY - eY)) != 0)
				bPass = (IsTraversable(Coordinate(check.x + offset.x, check.y)) && IsTraversable(Coordinate(check.x, check.y + offset.y)));
#endif
			if (bPass)
			{
				check.add(offset);
				sol.push_back(Coordinate(eX, eY));

				sol.push_back(Coordinate(check.x, check.y));
				sol.push_back(Coordinate(sX, sY));

				return true;
			}
		}



	}
	return false;
}
void BL_JPS_SUBGOAL_EXP::preprocessBoundaryLookupTables()
{
	for (int y = 1; y < height-1; y++)
	{
		bool currentPassable = false;
		if (xBoundaryPoints.size() <= y)
			xBoundaryPoints.push_back(vector<short>());
		else
			xBoundaryPoints[y-1].clear();
		for (int x = 1; x < width-1; x++)
		{
			if (IsTraversable(Coordinate(x-1 , y -1)) != currentPassable)
			{
				xBoundaryPoints[y-1].push_back(x - 1);
				currentPassable = !currentPassable;
			}
		}
		//if (currentPassable)
		xBoundaryPoints[y-1].push_back(width-2);
	}

	for (int x = 1; x < width-1; x++)
	{
		bool currentPassable = false;
		if (yBoundaryPoints.size() <= x)
			yBoundaryPoints.push_back(vector<short>());
		else
			yBoundaryPoints[x-1].clear();
		for (int y = 1; y < height-1; y++)
		{
			if (IsTraversable(Coordinate(x - 1, y-1 )) != currentPassable)
			{
				yBoundaryPoints[x-1].push_back(y - 1);
				currentPassable = !currentPassable;
			}
		}
		//if (currentPassable)
		yBoundaryPoints[x-1].push_back(height-2);
	}
}
cost BL_JPS_SUBGOAL_EXP::SubgoalAStarSearch(xyLoc  start, xyLoc  goal, cost searchLimit, std::vector<xyLoc> & thePath)
{
#ifdef SG_STATISTICS
	Timer t;
	t.StartTimer();
#endif
	thePath.clear();
	if (!IsTraversable(start) || !IsTraversable(goal))
		return 0;
	if (directSolution(start.x, start.y, goal.x,goal.y, thePath))
	{
       for (int i =0;i<((int)thePath.size())-1;i++)
       {
                int yDif = thePath[i+1].y-thePath[i].y;
                int xDif = thePath[i+1].x-thePath[i].x;
                int yStep = yDif==0?0:(yDif<0?-1:1);
                int xStep = xDif==0?0:(xDif<0?-1:1);
                yDif= abs(yDif);
                xDif= abs(xDif);
                int stepRange = max(yDif,xDif);
                for (int j =1;j<stepRange;j++)
                    thePath.insert(thePath.begin()+(i+j),xyLoc(thePath[i].x+xStep*j,thePath[i].y+yStep*j));

                i+=stepRange-1;
      }

		return 0;
    }
	static std::vector<subgoalId> abstractPath;
	static std::vector<subgoalId> localPath;
	static std::vector<mapLoc> mapLocPath;

	/*// First, check if there is a direct path between start and goal
	if (IsQuickHReachable(start, goal, mapLocPath))
	{
		ToXYLocPath(mapLocPath, thePath);
#ifdef SG_STATISTICS
		initializeTime += t.EndTimer();
		//std::cout<<"Quick path found with cost: "<<HCost(start,goal)/(double)CARD_COST<<std::endl;
#endif
		return HCost(start, goal);
	}
	else
		mapLocPath.clear();*/

	// If not, find a high level path between them
	subgoalId sgStart, sgGoal;
	static std::vector<subGoalIdDir> startDirectHReachableSubgoals;
	static std::vector<subGoalIdDir> goalDirectHReachableSubgoals;
	static std::vector<subgoalId> extraEdgeList;
	cost abstractCost, localCost;

	if (usePairwise > 0)	// If we have the global pairwise distances, use it
	{
		abstractCost = LookupOptimalPath(start, goal, sgStart, sgGoal, startDirectHReachableSubgoals, goalDirectHReachableSubgoals, abstractPath);

		/*localCost = CheckCommonLocal(sgStart, sgGoal, startDirectHReachableSubgoals, goalDirectHReachableSubgoals, localPath);
		if (localCost < abstractCost)
		{
		#ifdef SG_STATISTICS
		std::cout << "Shorter path found through common local: " << localCost / (double)CARD_COST << " < " << abstractCost / (double)CARD_COST << std::endl;
		#endif
		abstractCost = localCost;
		abstractPath = localPath;
	}*/
	}
	else	// Do a search over the subgoal graph
	{
		// Add the relevant edges to the graph
		//if (startDirectHReachableSubgoals.size()==0)
		//if (startDirectHReachableSubgoals.size()==0)
		ConnectStartAndGoalToGraph(start, goal, sgStart, sgGoal, startDirectHReachableSubgoals, goalDirectHReachableSubgoals, extraEdgeList);
		/*vector<xyLoc> tempLocs;
		for (int i = 0; i < goalDirectHReachableSubgoals.size(); i++)
			tempLocs.push_back(location[goalDirectHReachableSubgoals[i].id]);
		cost minSharedStartGoalCost = INFINITE_COST;
		subgoalId lowestCommonNode = 0;
		for (int i = 0; i < startDirectHReachableSubgoals.size(); i++)
		{
			for (int j = 0; j < goalDirectHReachableSubgoals.size(); j++)
				if (startDirectHReachableSubgoals[i].id == goalDirectHReachableSubgoals[j].id)
				{
					cost tempCost = HCost(sgGoal, startDirectHReachableSubgoals[i].id);
					tempCost += HCost(sgStart, startDirectHReachableSubgoals[i].id);
					if (tempCost < minSharedStartGoalCost)
					{
						lowestCommonNode = startDirectHReachableSubgoals[i].id;
						minSharedStartGoalCost = tempCost;
					}

					//minSharedStartGoalCost = min(minSharedStartGoalCost, HCost(sgGoal, startDirectHReachableSubgoals[i].id));
					//minSharedStartGoalCost = min(minSharedStartGoalCost, HCost(sgStart, startDirectHReachableSubgoals[i].id));
				}
		}*/
#ifdef SG_STATISTICS
		initializeTime += t.EndTimer();
		t.StartTimer();
#endif

		// Do an A* search over the modified subgoal graph
		abstractCost = SubgoalAStarSearch(sgStart, sgGoal, INFINITE_COST, abstractPath);
	}
	// Now we should have a path (if there is any), either from a direct lookup or an A* search

	/*if (!keepLocalEdges)	// If we have discarded local-to-local edges, we might be missing a shorter path
	{
		// Check if there is a shorter path through two local subgoals
		localCost = TryLocalPair(sgStart, sgGoal, startDirectHReachableSubgoals, goalDirectHReachableSubgoals, localPath);
		if (localCost < abstractCost)
		{
#ifdef SG_STATISTICS
			std::cout << "Shorter local path found: " << localCost / (double)CARD_COST << " < " << abstractCost / (double)CARD_COST << std::endl;
#endif
			abstractCost = localCost;
			abstractPath = localPath;
		}

		if (abstractCost == INFINITE_COST)	//Still haven't found a path
		{
			// Try to find any local-local connection
			CheckAllLocalPairs(sgStart, sgGoal, startDirectHReachableSubgoals, goalDirectHReachableSubgoals, abstractPath);
		}
	}
	// Now we should definitely have a high level path (if any)*/

	// Restore the original graph
	if (usePairwise == 0)
	{
		for (unsigned int i = 0; i < extraEdgeList.size(); i++)	// Remove all the extra edges
		{
			subgoalId sg = extraEdgeList[i];
			if (HasExtraEdge(sg))
			{
				RemoveExtraEdgeFlag(sg);
				nNeighbors[sg]--;	// Don't include the extra edge in the range
			}
		}
		extraEdgeList.clear();
	}
	startDirectHReachableSubgoals.clear();
	goalDirectHReachableSubgoals.clear();

	// Find the low level path (path of xyLoc's) from the high level path (path of subgoalId's)
	if (abstractCost != INFINITE_COST)
	{
		xyLoc from, to;

		if (abstractPath.size() > 0)
			to = location[abstractPath[0]];

		mapLocPath.clear();
		mapLocPath.push_back(ToMapLoc(to));

		for (unsigned int i = 1; i < abstractPath.size(); i++)
		{
			from = to;
			to = location[abstractPath[i]];
			if (!IsHReachable(from, to, mapLocPath, true))	// Append
			{
#ifdef SG_STATISTICS
				std::cout << "Hmmm! ";
				std::cout << "From: (" << from.x << " " << from.y << ") ";
				std::cout << "to: (" << to.x << " " << to.y << ") " << std::endl;
#endif
			}
		}
		ToXYLocPath(mapLocPath, thePath);
	}

#ifdef SG_STATISTICS
	finalizeTime += t.EndTimer();

#ifdef SG_STATISTICS_PER_SEARCH
	std::cout << "OPTIMIZ Found a path of length " << abstractCost / (double)CARD_COST << " in " << (initializeTime + searchTime + finalizeTime) * 1000 << "ms." << std::endl;
	std::cout << "Time spent initializing:          " << initializeTime * 1000 << "ms." << std::endl;
	std::cout << "Time spent finding abstract path: " << searchTime * 1000 << "ms." << std::endl;
	std::cout << "Time spent finalizing:            " << finalizeTime * 1000 << "ms." << std::endl;
	initializeTime = 0;		searchTime = 0;		finalizeTime = 0;	// reset for the next search. comment out for cumulative statistics
#endif
#endif

	return abstractCost;
}
void BL_JPS_SUBGOAL_EXP::findSolution(int sX, int sY, int _eX, int _eY, std::vector<xyLoc> & thePath )
{
	SubgoalAStarSearch(xyLoc(sX, sY), xyLoc(_eX, _eY), INFINITE_COST, thePath);
	//std::reverse(thePath.begin(), thePath.end());
}

/*cost BL_JPS_SUBGOAL_EXP::findSolution(xyLoc & start, xyLoc & goal, std::vector<xyLoc> & thePath)
{
	return SubgoalAStarSearch(start, goal, INFINITE_COST, thePath);
}*/

#ifdef SG_RUNNING_IN_HOG
void BL_JPS_SUBGOAL_EXP::OpenGLDraw(const MapEnvironment *env)
{
	//*
	for (subgoalId s = 0; s < nSubgoals; s++)
	{
		if (IsPruned(s))
			env->SetColor(1, 0, 0);
		else
			env->SetColor(0, 0, 1);

		env->OpenGLDraw(location[s]);

		/*
		std::vector<subgoalId> neighbors = edgeVector[s];
		for (unsigned int i = 0; i < neighbors.size(); i++)
		{
		if (neighbors[i] > s)
		{
		env->SetColor(0,0,1);
		//env->GLDrawColoredLine(location[s], location[neighbors[i]]);
		}
		}
		*/
	}

	for (unsigned int i = 0; i < defaultXYPath.size(); i++)
	{
		env->SetColor(1, 1, 0);
		env->OpenGLDraw(defaultXYPath[i]);
		//env->GLDrawColoredLine(location[defaultAbstractPath[i]], location[defaultAbstractPath[i+1]]);
	}
	/*/
	for (mapLoc loc = 0; loc < mapSize; loc++)
	{
	if (IsSubgoal(loc))
	{
	env->SetColor(0,0,1);
	env->OpenGLDraw(ToXYLoc(loc));
	}
	}
	//*/
}
#endif
void BL_JPS_SUBGOAL_EXP::PrintGraphStatistics()
{
	std::cout << "--------------------" << std::endl;

	int nGlobalSubgoals = 0;
	int nLocalSubgoals = 0;

	int nTotalGlobalToGlobalEdge = 0;
	int nTotalLocalToGlobalEdge = 0;
	int nTotalGlobalToLocalEdge = 0;
	int nTotalLocalToLocalEdge = 0;

	int nLocalNotConnectedToGlobal = 0;

	double sumGlobalToGlobalEdgeCost = 0.0;
	double sumLocalToGlobalEdgeCost = 0.0;

	double maxGlobalToGlobalEdgeCost = 0.0;
	double maxLocalToGlobalEdgeCost = 0.0;

	for (subgoalId sg1 = 0; sg1 < nSubgoals; sg1++)
	{
		int nGlobalToGlobalEdge = 0;
		int nLocalToGlobalEdge = 0;
		int nGlobalToLocalEdge = 0;
		int nLocalToLocalEdge = 0;

		if (IsPruned(sg1))
			nLocalSubgoals++;

		else
			nGlobalSubgoals++;

		std::vector<subGoalIdDir> neighbors = neighborhoodVector[sg1];
		for (unsigned int j = 0; j < neighbors.size(); j++)
		{
			subgoalId sg2 = neighbors[j].id;
			cost hCost = HCost(sg1, sg2);

			if (!IsPruned(sg1) && !IsPruned(sg2))
			{
				nGlobalToGlobalEdge++;
				sumGlobalToGlobalEdgeCost += hCost;
				if (maxGlobalToGlobalEdgeCost < hCost)
					maxGlobalToGlobalEdgeCost = hCost;
			}

			if (IsPruned(sg1) && !IsPruned(sg2))
			{
				nLocalToGlobalEdge++;
				sumLocalToGlobalEdgeCost += hCost;
				if (maxLocalToGlobalEdgeCost < hCost)
					maxLocalToGlobalEdgeCost = hCost;
			}

			if (!IsPruned(sg1) && IsPruned(sg2))
				nGlobalToLocalEdge++;

			if (IsPruned(sg1) && IsPruned(sg2))
				nLocalToLocalEdge++;
		}

		if (nLocalToGlobalEdge == 0 && IsPruned(sg1))
			nLocalNotConnectedToGlobal++;

		nTotalGlobalToGlobalEdge += nGlobalToGlobalEdge;
		nTotalLocalToGlobalEdge += nLocalToGlobalEdge;
		nTotalGlobalToLocalEdge += nGlobalToLocalEdge;
		nTotalLocalToLocalEdge += nLocalToLocalEdge;
	}
	/*
	std::cout<<"Total number of subgoals: "<<nGlobalSubgoals+nLocalSubgoals<<std::endl;
	std::cout<<"Total number of global subgoals: "<<nGlobalSubgoals<<std::endl;
	std::cout<<"Total number of local subgoals: "<<nLocalSubgoals<<std::endl;
	std::cout<<std::endl;
	std::cout<<"Total number of edges: "<<nTotalGlobalToGlobalEdge + nTotalLocalToGlobalEdge + nTotalGlobalToLocalEdge + nTotalLocalToLocalEdge<<std::endl;
	std::cout<<"Total number of global to global edges: "<<nTotalGlobalToGlobalEdge<<std::endl;
	std::cout<<"Total number of local to global edges: "<<nTotalLocalToGlobalEdge<<std::endl;
	std::cout<<"Total number of global to local edges: "<<nTotalGlobalToLocalEdge<<std::endl;
	std::cout<<"Total number of local to local edges: "<<nTotalLocalToLocalEdge<<std::endl;
	std::cout<<std::endl;
	*/
	std::cout << "Subgoals: " << nGlobalSubgoals + nLocalSubgoals << "\t(" << nGlobalSubgoals << " G + " << nLocalSubgoals << " L)\n";
	//std::cout<<"Edges: "<<nTotalGlobalToGlobalEdge + nTotalLocalToGlobalEdge + nTotalGlobalToLocalEdge + nTotalLocalToLocalEdge<<"\t(";
	std::cout << "Edges: " << nTotalGlobalToGlobalEdge + nTotalLocalToGlobalEdge << "\t(";
	std::cout << nTotalGlobalToGlobalEdge << " GG + ";
	std::cout << nTotalLocalToGlobalEdge << " LG + ";
	//	std::cout<<nTotalGlobalToLocalEdge<<" GL + ";
	std::cout << nTotalLocalToLocalEdge << " LL";
	std::cout << ")\n";
	std::cout << "Number of local subgoals not connected to a global subgoal: " << nLocalNotConnectedToGlobal << std::endl;
	std::cout << "Branching factor of the global subgraph: " << 1.0*nTotalGlobalToGlobalEdge / nGlobalSubgoals << std::endl;
	std::cout << "Maximum edge cost: " << maxGlobalToGlobalEdgeCost / (double)CARD_COST << " GG, " << maxLocalToGlobalEdgeCost / (double)CARD_COST << " LG" << std::endl;
	std::cout << "Average edge cost: " << (1.0*sumGlobalToGlobalEdgeCost / nTotalGlobalToGlobalEdge) / (double)CARD_COST << " GG, " << (1.0*sumLocalToGlobalEdgeCost / nTotalLocalToGlobalEdge) / (double)CARD_COST << " LG" << std::endl;
	//	std::cout<<"Size: "<<sizeof graph<<std::endl;
	//	std::cout<<"Size: "<<sizeof(std::vector<char>)<<std::endl;
	std::cout << "--------------------" << std::endl;


	return;

	unsigned int bitsPerMapCell = 8 + 8 + 32; //1 + 1 + 32;
	//traversable;
	//subgoal;
	//cellInfo;

	unsigned int bitsPerSubgoal = 32 + 32 + 16 + 32 + 32 + 32 + 16;
	// location + neighborPtr + nNeighbor + extraEdge + gCost + parent + search counter (+ flags)
	//32 + 1 + 32 + 16 + 1 + 32 + 16 + 32 + 32 + 1;
	//std::vector<xyLoc> location;
	//std::vector<bool> pruned;
	//std::vector<subgoalId*> neighbors;
	//std::vector<uint16_t> nNeighbors;
	//std::vector<bool> hasExtraEdge;
	// + 32 for the space allocated for extra edge
	//std::vector<uint16_t> generated;
	//std::vector<cost> gCost;
	//std::vector<subgoalId> parent;
	//std::vector<bool> open;

	unsigned int bitsPerSubgoalNoSearch = 32 + 32 + 16 + 32;
	//location + neighborPtr + nNeighbor + extraEdge (pruned + hasExtraEdge merged into nNeighbor)

	unsigned int bitsPerEdge = 32;
	double mapMemory = (bitsPerMapCell*height*width) / 8192.0;
	double subgoalMemory = (bitsPerSubgoal*(nGlobalSubgoals + nLocalSubgoals)) / 8192.0;
	double edgeMemory = (bitsPerEdge*(nTotalGlobalToGlobalEdge + nTotalLocalToGlobalEdge)) / 8192.0;
	double totalMemory = mapMemory + subgoalMemory + edgeMemory;

	double subgoalNoSearchMemory = (bitsPerSubgoalNoSearch*(nGlobalSubgoals + nLocalSubgoals)) / 8192.0;
	double totalNoSearchMemory = mapMemory + subgoalNoSearchMemory + edgeMemory;

	double inflation = (2048 * 2048) / (double(height - 1) * double(height - 1));

	std::cout << "Estimated memory usage:\n";
	std::cout << "Map info: " << mapMemory << "kb." << std::endl;
	std::cout << "Subgoals: " << subgoalMemory << "kb." << std::endl;
	std::cout << "Edges: " << edgeMemory << "kb." << std::endl;
	std::cout << "Total: " << totalMemory << "kb." << std::endl;
	std::cout << "Estimated memory for 2048x2048 map: " << totalMemory*inflation / 1024 << "mb." << std::endl;

	std::cout << std::endl;
	std::cout << "Subgoals (no search): " << subgoalNoSearchMemory << "kb." << std::endl;
	std::cout << "Total (no search): " << totalNoSearchMemory << "kb." << std::endl;
	std::cout << "Estimated memory for 2048x2048 map (no search): " << totalNoSearchMemory*inflation / 1024 << "mb." << std::endl;

	std::cout << "--------------------" << std::endl;

}
