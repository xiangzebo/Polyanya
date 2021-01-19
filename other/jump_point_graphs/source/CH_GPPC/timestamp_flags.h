#ifndef TIMESTAMP_FLAGS_H
#define TIMESTAMP_FLAGS_H

#include <vector>
#include <cassert>
#include <algorithm>

namespace ch_gppc {
class TimestampFlags{
public:
	TimestampFlags(){}
	explicit TimestampFlags(int flag_count)
		:timestamp(flag_count, 0), current_timestamp(1){
	}

	int flag_count()const{
		return timestamp.size();
	}

	void set(int f){
		assert(flag_count () != 0 && "You forgot to pass the flag_count to the constructor");
		assert(0 <= f && f < flag_count() && "flag is out of bounds");
		timestamp[f] = current_timestamp;
	}

	void unset(int f){
		assert(flag_count () != 0 && "You forgot to pass the flag_count to the constructor");
		assert(0 <= f && f < flag_count() && "flag is out of bounds");
		timestamp[f] = current_timestamp-1;
	}

	bool is_set(int f)const{
		assert(flag_count () != 0 && "You forgot to pass the flag_count to the constructor");
		assert(0 <= f && f < flag_count() && "flag is out of bounds");
		return timestamp[f] == current_timestamp;
	}

	void unset_all(){
		++current_timestamp;
		// Check if there was an overflow, and fix it if it happend
		if(current_timestamp == 0){
			++current_timestamp;
			std::fill(timestamp.begin(), timestamp.end(), 0);
		}
	}

private:
	std::vector<short>timestamp;
	short current_timestamp;
};
}
#endif

