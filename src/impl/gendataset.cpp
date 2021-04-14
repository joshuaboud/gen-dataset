
#include "gendataset.hpp"
#include "getopts.hpp"
#include <string>
#include <iostream>
#include <climits>

extern "C" {
	#include <sys/stat.h>
	#include <string.h>
}

std::string *gen_dir_names(int branches){
	std::string *dir_names = new std::string[branches];
	for(int i = 0; i < branches; ++i){
		dir_names[i] = std::to_string(i) + "/";
	}
	return dir_names;
}

void gen_dirs(int depth, int branches, std::string *dir_names, std::string curr_dir){
	if(depth == 0)
		return;
	for(int i = 0; i < branches; ++i){
		std::string next_dir = curr_dir + dir_names[i];
		int res = mkdir(next_dir.c_str(), 0777);
		if(res){
			int error = errno;
			std::cerr << "Error creating directory: " << strerror(error) << std::endl;
			exit(EXIT_FAILURE);
		}
		gen_dirs(depth - 1, branches, dir_names, next_dir);
	}
}

void gen_dataset(const Options &opts){
	std::string *dir_names = gen_dir_names(opts.branches);
	gen_dirs(opts.depth, opts.branches, dir_names, "");
	delete[] dir_names;
}
