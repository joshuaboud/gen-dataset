
#pragma once

struct Options{
	int depth = -1;
	int branches = -1;
	int count = -1;
	int size = 0;
	int max_wait_ms = 0;
	bool no_prompt = false;
};

Options get_opts(int argc, char *argv[]);
