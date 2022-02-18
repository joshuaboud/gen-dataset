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

#include "getopts.hpp"
#include <iostream>
#include <regex>
#include <cmath>
#include <climits>
extern "C"{
	#include <getopt.h>
	#include <unistd.h>
	#include <sys/stat.h>
}

static void usage(void){
	std::cout << 
	"gen-dataset Copyright (C) 2021 Josh Boudreau <jboudreau@45drives.com>\n"
	"usage:\n"
	"  gen-dataset  -c [-b -d -s -S -t -w -y] [path]\n"
	"\n"
	"flags:\n"
	"  -b, --branches <int>              - number of subdirectories per directory\n"
	"  -c, --count <int>                 - total number of files to create\n"
	"  -d, --depth <int>                 - number of directory levels\n"
	"  -s, --size <float[K..T][i]B>      - file size\n"
	"  -S, --buff-size <float[K..T][i]B> - write buffer size (default=1M)\n"
	"  -t, --threads <int>               - number of parallel file creation threads\n"
	"  -w, --max-wait <float (seconds)>  - max random wait between file creation\n"
	"  -y, --yes                         - don't prompt before creating files\n"
	<< std::endl;
}

static unsigned long parse_size(const std::string &arg){
	std::smatch m;
	if(regex_search(arg, m, std::regex("^(\\d+\\.?\\d*)\\s*([kmgt]?)(i?)b?$", std::regex_constants::icase))){
		double num;
		try{
			num = std::stod(m[1]);
		}catch(const std::invalid_argument &){
			std::cerr << "Invalid file/buffer size. Must be #[kKmMgGtT][i]B." << std::endl;
			exit(EXIT_FAILURE);
		}
		char prefix = (m.str(2).empty())? 0 : m.str(2).front();
		double base = (m.str(3).empty())? 1000.0 : 1024.0;
		double exp;
		switch(prefix){
			case 0:
				exp = 0.0;
				break;
			case 'k':
			case 'K':
				exp = 1.0;
				break;
			case 'm':
			case 'M':
				exp = 2.0;
				break;
			case 'g':
			case 'G':
				exp = 3.0;
				break;
			case 't':
			case 'T':
				exp = 4.0;
				break;
			default:
				std::cerr << "Invalid file/buffer size. Must be #[kKmMgGtT][i]B." << std::endl;
				exit(EXIT_FAILURE);
		}
		unsigned long long bytes = num * pow(base, exp);
		if (bytes > ULONG_MAX) {
			std::cerr << "File/buffer size too big." << std::endl;
			exit(EXIT_FAILURE);
		}
		return bytes;
	}
	std::cerr << "Invalid file/buffer size. Must be #[kKmMgGtT][i]B." << std::endl;
	exit(EXIT_FAILURE);
}

static void check_opts(const Options &opts){
	bool errors = false;
	if(opts.depth < 0){
		std::cerr << "Invalid depth: " << opts.depth << std::endl;
		errors = true;
	}
	if(opts.branches < 0){
		std::cerr << "Invalid branches: " << opts.branches << std::endl;
		errors = true;
	}
	if(opts.count < 0){
		std::cerr << "Invalid count: " << opts.count << std::endl;
		errors = true;
	}
	if(opts.max_wait_ms < 0){
		std::cerr << "Invalid max random wait: " << opts.max_wait_ms << std::endl;
		errors = true;
	}
	if(opts.threads < 1){
		std::cerr << "Invalid number of threads: " << opts.threads << std::endl;
		errors = true;
	}
	if(errors)
		exit(EXIT_FAILURE);
}

unsigned long calc_total_dirs(int depth, int branches){
	unsigned long sum = 0;
	
	for(int i = 0; i < depth; ++i){
		sum += pow(branches, i+1);
	}
	
	return sum;
}

Options get_opts(int argc, char *argv[]){
	int opt;
	int option_ind = 0;
	
	Options opts;
	
	static struct option long_options[] = {
		{"--depth",          required_argument, 0, 'd'},
		{"--branches",       required_argument, 0, 'b'},
		{"--count",          required_argument, 0, 'c'},
		{"--size",           required_argument, 0, 's'},
		{"--buff-size",      required_argument, 0, 'S'},
		{"--max-wait",       required_argument, 0, 'w'},
		{"--threads",        required_argument, 0, 't'},
		{"--yes",            no_argument,       0, 'y'},
		{"--help",           no_argument,       0, 'h'},
		{0, 0, 0, 0}
	};
	
	while((opt = getopt_long(argc, argv, "d:b:c:s:S:w:t:yh", long_options, &option_ind)) != -1){
		switch(opt){
			case 'd':
				try{
					opts.depth = std::stoi(optarg);
				}catch(const std::invalid_argument &){
					std::cerr << "Invalid depth. Must be integer." << std::endl;
					usage();
					exit(EXIT_FAILURE);
				}
				break;
			case 'b':
				try{
					opts.branches = std::stoi(optarg);
				}catch(const std::invalid_argument &){
					std::cerr << "Invalid branches per node. Must be integer." << std::endl;
					usage();
					exit(EXIT_FAILURE);
				}
				break;
			case 'c':
				try{
					opts.count = std::stoul(optarg);
				}catch(const std::invalid_argument &){
					std::cerr << "Invalid number of files. Must be integer." << std::endl;
					usage();
					exit(EXIT_FAILURE);
				}
				break;
			case 's':
				opts.size = parse_size(optarg);
				break;
			case 'S':
				opts.buff_sz = parse_size(optarg);
				break;
			case 'w':
				try{
					opts.max_wait_ms = (unsigned long)(std::stod(optarg) * 1000.0);
				}catch(const std::invalid_argument &){
					std::cerr << "Invalid max random wait. Must be floating point." << std::endl;
					usage();
					exit(EXIT_FAILURE);
				}
				break;
			case 't':
				try{
					opts.threads = std::stoi(optarg);
				}catch(const std::invalid_argument &){
					std::cerr << "Invalid number of threads. Must be integer." << std::endl;
					usage();
					exit(EXIT_FAILURE);
				}
				break;
			case 'y':
				opts.no_prompt = true;
				break;
			case 'h':
				usage();
				exit(EXIT_SUCCESS);
			default:
				usage();
				exit(EXIT_FAILURE);
		}
	}
	
	check_opts(opts);
	
	bool path_passed = (optind != argc);
	
	if(!opts.no_prompt){
		char pwd[PATH_MAX];
		if(getcwd(pwd, sizeof(pwd)) == nullptr){
			int error = errno;
			std::cerr << "Error getting current directory : " << strerror(error) << std::endl;
			exit(EXIT_FAILURE);
		}
		
		unsigned long total_dirs = calc_total_dirs(opts.depth, opts.branches);
		
		std::string dest_name = (path_passed)? argv[optind] : pwd;
		std::cout << "Create " << total_dirs << " directories and " << opts.count << " files in " << dest_name << "? [y/N] ";
		char response = getchar();
		if(!(response == 'y' || response == 'Y'))
			exit(EXIT_SUCCESS);
	}
	
	if(path_passed){
		int res;
		res = mkdir(argv[optind], 0755);
		if(res){
			int error = errno;
			if(error != EEXIST){
				std::cerr << "Error creating directory " << argv[optind] << ": " << strerror(error) << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		res = chdir(argv[optind]);
		if(res){
			int error = errno;
			std::cerr << "Error changing directory to " << argv[optind] << ": " << strerror(error) << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	
	return opts;
}
