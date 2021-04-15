
#include "gendataset.hpp"
#include "getopts.hpp"
#include <string>
#include <iostream>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <thread>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

boost::mt19937 gen;

extern "C" {
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
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
			if(error != EEXIST){
				std::cerr << "Error creating directory: " << strerror(error) << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		gen_dirs(depth - 1, branches, dir_names, next_dir);
	}
}

boost::random::discrete_distribution<> get_geometric_biased_rand_generator(int depth, int branches){
	std::vector<double> probabilities(depth, 0.0);
	
	double base_prob = (1.0 - double(branches)) / (1.0 - pow(branches, depth));
	
	for(double i = 0.0; i < depth; i += 1.0){
		probabilities[i] = pow(branches, i) * base_prob;
	}
	
	boost::random::discrete_distribution<> dist(probabilities);
	return dist;
}

std::string *gen_file_paths(int depth, int branches, int count, std::string *dir_names){
	std::string *file_names = new std::string[count];
	srand(time(NULL));
	boost::random::discrete_distribution<> biased_depth = get_geometric_biased_rand_generator(depth, branches);
	for(int i = 0; i < count; ++i){
		file_names[i] = "";
		int depth_placement = biased_depth(gen);
		for(int j = 0; j < depth_placement; ++j){
			int branch_choice = rand() % branches;
			file_names[i] += dir_names[branch_choice];
		}
		file_names[i] += "file" + std::to_string(i) + ".img";
	}
	return file_names;
}

void touch_files(std::string *file_names, int count, int size, int max_wait_ms){
	if(size){
		// create and write
		int buff_sz = std::min(size, 1024 * 1024);
		unsigned char *buff = new unsigned char[buff_sz];
		memset(buff, 0, buff_sz*sizeof(unsigned char));
		for(int i = 0; i < count; ++i){
			int fd = creat(file_names[i].c_str(), 0777);
			if(fd == -1){
				int error = errno;
				std::cerr << "Error creating file: " << strerror(error) << std::endl;
				delete[] buff;
				exit(EXIT_FAILURE);
			}
			int bytes_left = size;
			while(bytes_left > 0){
				int bytes_written = write(fd, buff, std::min(buff_sz, bytes_left));
				if(bytes_written == -1){
					int error = errno;
					std::cerr << "Error writing to file: " << strerror(error) << std::endl;
					delete[] buff;
					exit(EXIT_FAILURE);
				}
				bytes_left -= bytes_written;
			}
			int res = close(fd);
			if(res == -1){
				int error = errno;
				std::cerr << "Error closing file: " << strerror(error) << std::endl;
				exit(EXIT_FAILURE);
			}
			if(max_wait_ms)
				std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (max_wait_ms+1)));
		}
		delete[] buff;
	}else{
		// only create
		for(int i = 0; i < count; ++i){
			int fd = creat(file_names[i].c_str(), 0777);
			if(fd == -1){
				int error = errno;
				std::cerr << "Error creating file: " << strerror(error) << std::endl;
				exit(EXIT_FAILURE);
			}
			int res = close(fd);
			if(res == -1){
				int error = errno;
				std::cerr << "Error closing file: " << strerror(error) << std::endl;
				exit(EXIT_FAILURE);
			}
			if(max_wait_ms)
				std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (max_wait_ms+1)));
		}
	}
}

void gen_dataset(const Options &opts){
	std::string *dir_names = gen_dir_names(opts.branches);
	gen_dirs(opts.depth, opts.branches, dir_names, "");
	std::string *file_names = gen_file_paths(opts.depth, opts.branches, opts.count, dir_names);
	touch_files(file_names, opts.count, opts.size, opts.max_wait_ms);
	delete[] file_names;
	delete[] dir_names;
}
