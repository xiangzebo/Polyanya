#ifndef HOGCONFIG_H
#define HOGCONFIG_H

// HogConfig.h
//
// A class for configuring HOG.
// Configurations can be created on the fly 
// or read from a configuration file
//
// @author: dharabor
// @created: 02/06/2010


class HogConfig
{
	public: 
		HogConfig();
		HogConfig(const char* filename);
		~HogConfig();

		void readFile(const char* filename);

		inline void setGUI(bool gui) { this->gui = gui; }
		inline bool getGUI() { return gui; }

		void setAlg(const char* alg); 
		inline const char* getAlg() { return alg; }

		void setMap(const char* map); 
		inline const char* getMap() { return map; }

		void setScenario(const char* scenario); 
		inline const char* getScenario() { return scenario; }

		inline const char* getConfigFilename() { return filename; }

	private:
		char* filename; // config filename
		char* map; // map filename
		char* scenario; // scenario filename
		char* alg; // name of algorithm to run
		bool gui;

		char* stringCopy(const char* src);
};

#endif

