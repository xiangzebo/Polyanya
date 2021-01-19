#include "HogConfig.h"
#include <cstring>

HogConfig::HogConfig()
	: filename(0), map(0), scenario(0), alg(0), gui(true)
{
}

HogConfig::HogConfig(const char* filename_)
	: filename(0), map(0), scenario(0), alg(0), gui(true)
{
	if(filename_ != 0)
		this->filename = stringCopy(filename_);
}

HogConfig::~HogConfig()
{
	if(filename)
		delete [] this->filename;
	if(alg)
		delete [] this->alg;
}

char* 
HogConfig::stringCopy(const char* src)
{
	char* dest = 0;
	if(src)
	{
		int len=0;
		while(src[len] != '\0')
			len++;
		dest = new char[len];
		strcpy(dest, src);
	}
	return dest;
}

void 
HogConfig::readFile(const char* filename)
{
	if(this->filename)
	{
		delete [] this->filename;
		this->filename = 0;
		this->filename = stringCopy(filename);
	}
}
void
HogConfig::setAlg(const char* alg)
{
	if(this->alg)
	{
		delete [] this->alg;
		this->alg = 0;
		this->alg = stringCopy(alg);
	}
}

void
HogConfig::setMap(const char* map)
{
	if(this->map)
	{
		delete [] this->map;
		this->map = 0;
		this->map = stringCopy(map);
	}
}

void
HogConfig::setScenario(const char* scenario)
{
	if(this->scenario)
	{
		delete [] this->scenario;
		this->scenario = 0;
		this->scenario = stringCopy(scenario);
	}
}
