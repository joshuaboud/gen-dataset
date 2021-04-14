
#include "getopts.hpp"
#include <iostream>
#include <regex>
#include <cmath>
#include <climits>
extern "C"{
	#include <getopt.h>
	#include <unistd.h>
}

static void usage(void){
	std::cout << "gen-dataset Copyright (C) 2021 Josh Boudreau <jboudreau@45drives.com>" << std::endl;
	std::cout << "usage:" << std::endl;
	std::cout << "  gen-dataset -d <depth> -b <branches per node> -c <# files> [-s <file size> -m <max random wait>]" << std::endl;
}

static int parse_size(const std::string &arg){
	std::smatch m;
	if(regex_search(arg, m, std::regex("^(\\d+)\\s*([kKmMgGtTpPeEzZyY]?)(i?)[bB]$"))){
		double num;
		try{
			num = std::stod(m[1]);
		}catch(const std::invalid_argument &){
			std::cerr << "Invalid file size. Must be # [kKmMgGtT][i]B." << std::endl;
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
			case 'p':
			case 'P':
				exp = 5.0;
				break;
			case 'e':
			case 'E':
				exp = 6.0;
				break;
			case 'z':
			case 'Z':
				exp = 7.0;
				break;
			case 'y':
			case 'Y':
				exp = 8.0;
				break;
			default:
				std::cerr << "Invalid file size. Must be # [kKmMgGtT][i]B." << std::endl;
				exit(EXIT_FAILURE);
		}
		return int(num * pow(base, exp));
	}
	std::cerr << "Invalid file size. Must be # [kKmMgGtT][i]B." << std::endl;
	exit(EXIT_FAILURE);
}

static void check_opts(const Options &opts){
	bool errors = false;
	if(opts.depth < 1){
		std::cerr << "Invalid depth: " << opts.depth << std::endl;
		errors = true;
	}
	if(opts.branches < 1){
		std::cerr << "Invalid branches: " << opts.branches << std::endl;
		errors = true;
	}
	if(opts.count < 0){
		std::cerr << "Invalid count: " << opts.count << std::endl;
		errors = true;
	}
	if(opts.size < 0){
		std::cerr << "Invalid size: " << opts.size << std::endl;
		errors = true;
	}
	if(opts.max_wait < 0){
		std::cerr << "Invalid max random wait: " << opts.max_wait << std::endl;
		errors = true;
	}
	if(errors)
		exit(EXIT_FAILURE);
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
		{"--max-wait",       required_argument, 0, 'w'},
		{"--help",           no_argument,       0, 'h'},
		{0, 0, 0, 0}
	};
	
	while((opt = getopt_long(argc, argv, "d:b:c:s:w:h", long_options, &option_ind)) != -1){
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
					opts.count = std::stoi(optarg);
				}catch(const std::invalid_argument &){
					std::cerr << "Invalid number of files. Must be integer." << std::endl;
					usage();
					exit(EXIT_FAILURE);
				}
				break;
			case 's':
				opts.size = parse_size(optarg);
				break;
			case 'w':
				try{
					opts.branches = std::stoi(optarg);
				}catch(const std::invalid_argument &){
					std::cerr << "Invalid max random wait. Must be integer." << std::endl;
					usage();
					exit(EXIT_FAILURE);
				}
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
	
	if(optind != argc){
		int res = chdir(argv[optind]);
		if(res){
			int error = errno;
			std::cerr << "Error changing directory to " << argv[optind] << ": " << strerror(error) << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	
	char pwd[PATH_MAX];
	if(getcwd(pwd, sizeof(pwd)) == nullptr){
		int error = errno;
		std::cerr << "Error getting current directory : " << strerror(error) << std::endl;
		exit(EXIT_FAILURE);
	}
	std::cout << "Create " << opts.count << " files in " << pwd << "? [y/N] ";
	char response = getchar();
	if(!(response == 'y' || response == 'Y'))
		exit(EXIT_SUCCESS);
	
	return opts;
}
