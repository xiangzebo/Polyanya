#include "mapper.h"
#include <cstdio>
using namespace std;

void dump_map(const Mapper&map, const char*file){
	FILE* f = fopen(file, "w");

	for(int y=0; y<map.height(); ++y){
		for(int x=0; x<map.width(); ++x){
			fprintf(f, "%5d", map(xyLoc{static_cast<std::int16_t>(x), static_cast<std::int16_t>(y)}));
		}
		fprintf(f, "\n");
	}
	fclose(f);
}

