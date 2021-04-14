
#pragma once

struct Options{
	int depth = -1;
	int branches = -1;
	int count = -1;
	int size = 0;
	int max_wait = 0;
};

Options get_opts(int argc, char *argv[]);
