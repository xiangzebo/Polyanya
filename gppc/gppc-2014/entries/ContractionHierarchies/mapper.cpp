#include "mapper.h"
#include <cstdio>
using namespace std;

void dump_map(const Mapper&map, const char*file){
	FILE* f = fopen(file, "w");

	for(int y=0; y<map.height(); ++y){
		for(int x=0; x<map.width(); ++x){
			int id = map(xyLoc{static_cast<std::int16_t>(x), static_cast<std::int16_t>(y)});
			/*if(id > 500){
				fprintf(f, "X");
				//fprintf(f, "%4d!", id);
			}else if(id > 0)
				fprintf(f, ".");
			else
				fprintf(f, " ");*/
			fprintf(f, "%5d", id);
		}
		fprintf(f, "\n");
	}
	fclose(f);
}

