#include "ray_marching.hpp"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

	unsigned int texture_w = atoi(argv[1]);
	unsigned int texture_h = atoi(argv[2]);
	unsigned int intervals = atoi(argv[3]);
	unsigned int tiles = atoi(argv[4]);
	run(texture_w, texture_h, intervals, tiles);
	return 0;
}
