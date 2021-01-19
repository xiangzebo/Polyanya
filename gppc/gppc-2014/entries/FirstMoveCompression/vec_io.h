#ifndef VEC_IO_H
#define VEC_IO_H

#include <cstdio>
#include <stdexcept>

template<class T>
void save_vector(std::FILE*file, const std::vector<T>&v){
	int s = v.size();
	if(std::fwrite(&s, sizeof(s), 1, file) != 1)
		throw std::runtime_error("std::fwrite failed");
	if(std::fwrite(&v[0], sizeof(T), v.size(), file) != v.size())
		throw std::runtime_error("std::fwrite failed");
}

template<class T>
std::vector<T>load_vector(std::FILE*file){
	int s;
	if(std::fread(&s, sizeof(s), 1, file) != 1)
		throw std::runtime_error("std::fread failed");
	std::vector<T>v(s);
	if((int)std::fread(&v[0], sizeof(T), s, file) != s)
		throw std::runtime_error("std::fread failed");

	return v; // NVRO
}

#endif
