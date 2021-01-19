#include <stdint.h>
#include <list>
struct xyLoc {
  int16_t x;
  int16_t y;
};

/**
 * @struct cells
 * @brief A struct that represents a cell and its fCost.
 */
struct cells {
	int currentCell;
	float fCost;
};  

void PreprocessMap(std::vector<bool> &bits, int width, int height, const char *filename);
void *PrepareForSearch(std::vector<bool> &bits, int width, int height, const char *filename);
bool GetPath(void *data, xyLoc s, xyLoc g, std::vector<xyLoc> &path);
const char *GetName();

