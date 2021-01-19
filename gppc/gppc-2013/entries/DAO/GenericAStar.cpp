/*
 * GenericAStar.cpp
 *
 * Copyright (c) 2007, Nathan Sturtevant
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Alberta nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY NATHAN STURTEVANT ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 

#include "GenericAStar.h"
#include "float.h"

using namespace GenericAStarUtil;
static const bool verbose = false;

/**
* GenericAStar::GetName()
 *
 * \brief Return the name of the algorithm
 *
 * \return none
 */
const char *GenericAStar::GetName()
{
    static char name[32];
    sprintf(name, "GenericAStar[]");
    return name;
}


/**
* GenericAStar::GetPath()
 *
 * \brief Find the optimal path between two states.
 *
 * Will find an optimal path between two states, assuming that the
 * heuristic function in the search environment is both admissible and
 * consistent. (We do not re-open closed nodes.)
 *
 * For efficiency purposes, we return the path in reverse order, since that
 * is how it is extracted.
 *
 * Since the search process can be run iteratively or in one fell swoop, we
 * just use the iterative functions here as helper functions.
 *
 * \param e A search environment
 * \param from The start node
 * \param to The goal node
 * \param thePath A vector for the final path.
 * \return none
 */
void GenericAStar::GetPath(SearchEnvironment *e, uint32_t from, uint32_t to,
                           std::vector<uint32_t> &thePath)
{
    if (verbose)
        printf("--------------------------------\n");
    if (!InitializeSearch(e, from, to, thePath))
        return;
    while (!DoSingleSearchStep(thePath)) {}
}

/**
* GenericAStar::InitializeSearch()
 *
 * \brief Initialize an A* search.
 *
 * See GenericAStar::GetPath for more details.
 *
 * \param e A search environment
 * \param from The start node
 * \param to The goal node
 * \param thePath A vector for the final path.
 * \return true if the search initialized properly
 */
bool GenericAStar::InitializeSearch(SearchEnvironment *e, uint32_t from, uint32_t to,
                                    std::vector<uint32_t> &thePath)
{
    env = e;
    assert(openQueue.size() == 0);
    assert(closedList.size() == 0);
    nodesTouched = nodesExpanded = 0;
    start = from;
    goal = to;
    
    if ((from == UINT32_MAX) || (to == UINT32_MAX) || (from == to))
    {
        thePath.resize(0);
        return false;
    }
    
    SearchNode first(env->HCost(goal, start), 0, start, start);
    openQueue.add(first);
    
    return true;
}

/**
* GenericAStar::DoSingleSearchStep()
 *
 * \brief Perform one node expansion in an A* search.
 *
 * Initialize search must be called and return true before DoSingleSearchStep
 * can be called. The function call will expand a single node in the A* search.
 *
 * See GenericAStar::GetPath for more details.
 *
 * \param thePath The return path, returned only if the goal is found.
 * \return true if the goal is found.
 */
bool GenericAStar::DoSingleSearchStep(std::vector<uint32_t> &thePath)
{
    uint32_t currentOpenNode = UINT32_MAX;
    
    // if the open list is empty, no nodes are left to open
    if (openQueue.size() == 0)
    {
        closedList.clear();
        openQueue.reset();
        thePath.resize(0); // no path found!
        return true;
    }
    
    // get top of queue
    currentOpenNode = GetNextNode();
    
    // if we have a goal, return the pasth
    if (env->GoalTest(currentOpenNode, goal))
    {
        ExtractPathToStart(currentOpenNode, thePath);
        closedList.clear();
        openQueue.reset();
        env = 0;
        return true;
    }
    
    if (verbose)
    {
        // this print format won't work in general, but it works
        // for all the examples environments here
        printf("Opening %d:%d\n",
               currentOpenNode>>16,
               currentOpenNode&0xFFFF);
    }
    
    if (currentOpenNode == UINT32_MAX)
        printf("Oh no! The current open node is NULL\n");
    
    neighbors.resize(0);
    env->GetNeighbors(currentOpenNode, neighbors);
    
    // iterate over all the children
    for (unsigned int x = 0; x < neighbors.size(); x++)
    {
        nodesTouched++;
        uint32_t neighbor = neighbors[x];
        assert(neighbor != UINT32_MAX);
        
        // it's already on the closed list
        if (closedList.find(neighbor) != closedList.end())
        {
            if (verbose)
            {
                printf("skipping node %d:%d\n", neighbor>>16, neighbor&0xFFFF);
            }
            continue;
        }
        // it's on the open list
        else if (openQueue.isIn(SearchNode(neighbor)))
        {
            if (verbose)
            {
                printf("updating node %d:%d\n", neighbor>>16, neighbor&0xFFFF);
            }
            UpdateWeight(currentOpenNode, neighbor);
        }
        // it's newly opened
        else {
            if (verbose)
            {
                printf("adding node %d:%d\n", neighbor>>16, neighbor&0xFFFF);
            }
            AddToOpenList(currentOpenNode, neighbor);
        }
    }
    return false;
}

/**
* GenericAStar::CheckNextNode()
 *
 * \brief Peek at the top of the open list.
 *
 * \return The top node on the open list.
 */
uint32_t GenericAStar::CheckNextNode()
{
    return openQueue.top().currNode;
}

/**
* GenericAStar::GetNextNode()
 *
 * \brief Remove and return the top item from the open list.
 *
 * The returned object is placed on the closed list after being removed
 * from the open list.
 *
 * \return node id of the top item on the open list
 */
uint32_t GenericAStar::GetNextNode()
{
    nodesExpanded++;
    uint32_t next;
    SearchNode it = openQueue.remove();
    next = it.currNode;
    closedList[next] = it;
    return next;
}

/**
* GenericAStar::UpdateWeight()
 *
 * \brief Check to see if we can update the weight of a node.
 *
 * When searching the neighbors of the currOpenNode, if we come across
 * a neighbor which is already opened, check to see if there is a shorter
 * path to this node through the current open node. If so, update
 * the parent of the neighbor and decrease its key in the open list.
 *
 * \param currOpenNode The possible new parent of the neighbor node
 * \param neighbor The node we might have a shorter path too
 * \return none
 */
void GenericAStar::UpdateWeight(uint32_t currOpenNode, uint32_t neighbor)
{
    SearchNode prev = openQueue.find(SearchNode(neighbor));
    SearchNode alt = closedList[currOpenNode];
    double edgeWeight = env->GCost(currOpenNode, neighbor);
    double altCost = alt.gCost+edgeWeight+(prev.fCost-prev.gCost);
    if (fgreater(prev.fCost, altCost))
    {
        if (verbose) 
        { printf("Updating %d:%d in openQueue, old f-cost is %f, new f-cost is %f\n",
                 neighbor>>16, neighbor&0xFFFF, prev.fCost, altCost); }
        prev.fCost = altCost;
        prev.gCost = alt.gCost+edgeWeight;
        prev.prevNode = currOpenNode;
        openQueue.decreaseKey(prev);

//        SearchNode test = openQueue.find(SearchNode(neighbor));
    }
}

/**
* GenericAStar::AddToOpenList()
 *
 * \brief Add a new neighbor to the open list.
 *
 * Add the neighbor node of the current open node to the open list.
 *
 * \param currOpenNode The parent of the new neighbor
 * \param neighbor The new node to add to the open list
 * \return none
 */
void GenericAStar::AddToOpenList(uint32_t currOpenNode, uint32_t neighbor)
{
    double edgeWeight = env->GCost(currOpenNode, neighbor);
    SearchNode n(closedList[currOpenNode].gCost+edgeWeight+env->HCost(neighbor, goal),
                 closedList[currOpenNode].gCost+edgeWeight,
                 neighbor, currOpenNode);
    if (verbose) 
    { printf("Adding %d:%d to openQueue, old size %u, f-cost is %f\n",
             neighbor>>16, neighbor&0xFFFF, openQueue.size(), n.fCost); }
    openQueue.add(n);
}

/**
* GenericAStar::ExtractPathToStart()
 *
 * \brief Extract a path from the goal to the start. NOTE: The path is reversed (goal->start).
 *
 * \param goalNode The final node in the search
 * \param thePath The path between the goal and the start
 * \return none
 */
void GenericAStar::ExtractPathToStart(uint32_t goalNode,
                                      std::vector<uint32_t> &thePath)
{
    SearchNode n;
    if (closedList.find(goalNode) != closedList.end())
    {
        n = closedList[goalNode];
    }
    else n = openQueue.find(SearchNode(goalNode));
    
    do {
        thePath.push_back(n.currNode);
        n = closedList[n.prevNode];
    } while (n.currNode != n.prevNode);
    thePath.push_back(n.currNode);
}

/**
* GenericAStar::PrintStats()
 *
 * \brief Print the size of the open/closed list
 *
 * \return none
 */
void GenericAStar::PrintStats()
{
    printf("%u items in closed list\n", (unsigned int)closedList.size());
    printf("%u items in open queue\n", (unsigned int)openQueue.size());
}

/**
* GenericAStar::GetMemoryUsage()
 *
 * \brief Get the combined size of the open & closed list.
 *
 * \return The number of items in the open & closed list combined.
 */
int GenericAStar::GetMemoryUsage()
{
    return (int)closedList.size()+openQueue.size();
}

/**
* GenericAStar::GetClosedListIter()
 *
 * \brief Get an iterator for the closed list.
 *
 * \return the iterator
 */
closedList_iterator GenericAStar::GetClosedListIter() const
{
    return closedList.begin();
}

/**
* GenericAStar::ClosedListIterNext()
 *
 * \brief Given the iterator, return the next node id
 *
 * While the iterator could be used directly, it is better to use this
 * function, as it hides the internal storage of the closed list.
 *
 * \param it An iterator returned from GenericAStar::GetClosedListIter()
 * \return Next node id on the closed list.
 */
uint32_t GenericAStar::ClosedListIterNext(closedList_iterator& it) const
{
    if (it == closedList.end())
        return UINT32_MAX;
    uint32_t val = (*it).first;
    it++;
    return val;
}
