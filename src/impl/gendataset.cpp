/*
 *    Copyright (C) 2021 Joshua Boudreau <jboudreau@45drives.com>
 *    
 *    This file is part of gen-dataset.
 * 
 *    gen-dataset is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    gen-dataset is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with gen-dataset.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "gendataset.hpp"
#include "getopts.hpp"
#include <string>
#include <iostream>
#include <iomanip>
#include <climits>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <thread>
#include <atomic>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/discrete_distribution.hpp>

boost::mt19937 gen;

extern "C" {
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
}

struct Status{
	std::atomic<bool> running;
	std::atomic<int> number_created;
};

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

void gen_file_paths(std::vector<std::string> &file_names, int depth, int branches, long int count, std::string *dir_names){
	srand(time(NULL));
	boost::random::discrete_distribution<> biased_depth = get_geometric_biased_rand_generator(depth, branches);
	for(long int i = 0; i < count; ++i){
		file_names[i] = "";
		int depth_placement = biased_depth(gen);
		for(int j = 0; j < depth_placement; ++j){
			int branch_choice = rand() % branches;
			file_names[i] += dir_names[branch_choice];
		}
		file_names[i] += "file" + std::to_string(i) + ".img";
	}
}

inline void create_file(const char *name, const unsigned char *buff, unsigned long buff_sz, unsigned long file_sz){
	int fd = creat(name, 0777);
	if(fd == -1){
		int error = errno;
		std::cerr << "Error creating file: " << strerror(error) << std::endl;
		delete[] buff;
		exit(EXIT_FAILURE);
	}
	if(buff){
		unsigned long bytes_left = file_sz;
		while(bytes_left > 0){
			ssize_t bytes_written = write(fd, buff, std::min(buff_sz, bytes_left));
			if(bytes_written == -1){
				int error = errno;
				std::cerr << "Error writing to file: " << strerror(error) << std::endl;
				delete[] buff;
				exit(EXIT_FAILURE);
			}
			bytes_left -= bytes_written;
		}
	}
	int res = close(fd);
	if(res == -1){
		int error = errno;
		std::cerr << "Error closing file: " << strerror(error) << std::endl;
		exit(EXIT_FAILURE);
	}
}

void touch_files(const std::vector<std::string> &file_names, const unsigned char *buff, unsigned long buff_sz, unsigned long file_sz, unsigned long max_wait_ms, Status &status){
	for(const std::string &file_name : file_names){
		create_file(file_name.c_str(), buff, buff_sz, file_sz);
		status.number_created++;
		if(max_wait_ms)
			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (max_wait_ms+1)));
	}
}

void launch_threads(int num_threads, const std::vector<std::string> &file_names, const unsigned char *buff, unsigned long buff_sz, unsigned long file_sz, unsigned long max_wait_ms, Status &status){
	std::vector<std::vector<std::string>> partitions(
		num_threads,
		std::vector<std::string>()
	);
	// round robin distribute files
	int thread_ind = 0;
	for(unsigned i = 0; i < file_names.size(); ++i){
		partitions[thread_ind].push_back(file_names[i]);
		thread_ind = (thread_ind+1)%num_threads;
	}
	std::vector<std::thread> threads;
	for(const std::vector<std::string> &partition : partitions){
		threads.emplace_back(touch_files, partition, buff, buff_sz, file_sz, max_wait_ms, std::ref(status));
	}
	for(std::thread &thread : threads){
		thread.join();
	}
}

void report_status(const Status &status, long int count){
	long int num_created = 0;
	const int prog_bar_w = 40;
	std::string prog_bar(prog_bar_w, ' ');
	int prog_ind = 0;
	int next_ind = 0;
	while(status.running || num_created != count){
		num_created = status.number_created;
		std::cout << "Files created: " << num_created << '/' << count << std::endl;
		next_ind = (num_created * (prog_bar_w-1) / count);
		while(prog_ind < next_ind){
			prog_bar[prog_ind++] = '=';
		}
		prog_bar[prog_ind] = '>';
		std::cout << "[" << prog_bar << "]" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		std::cout << "\033[2A\r";
	}
	std::cout << std::endl << std::endl;
}

void gen_dataset(const Options &opts){
	std::string *dir_names = gen_dir_names(opts.branches);
	gen_dirs(opts.depth, opts.branches, dir_names, "");
	std::vector<std::string> file_names(opts.count, "");
	gen_file_paths(file_names, opts.depth+1, opts.branches, opts.count, dir_names);
	unsigned char *buff = nullptr;
	unsigned long buff_sz = opts.buff_sz;
	if(opts.size){
		if (opts.size < buff_sz) {
			buff_sz = opts.size; // limit buffer size to file size
		}
		buff = new unsigned char[buff_sz];
		memset(buff, 0, buff_sz*sizeof(unsigned char));
	}
	Status status;
	status.running = true;
	status.number_created = 0;
	std::thread status_thread(report_status, std::ref(status), opts.count);
	if(opts.threads > 1)
		launch_threads(opts.threads, file_names, buff, buff_sz, opts.size, opts.max_wait_ms, std::ref(status));
	else
		touch_files(file_names, buff, buff_sz, opts.size, opts.max_wait_ms, std::ref(status));
	status.running = false;
	status_thread.join();
	if(opts.size)
		delete[] buff;
	delete[] dir_names;
}
