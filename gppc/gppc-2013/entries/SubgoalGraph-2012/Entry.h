#include <stdio.h>
#include <stdint.h>
#include <deque>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "SubgoalGraph.h"
#include "QuickBucket.h"

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename);
void *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename);
bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path);
const char *GetName();
void CleanUp();

