#include "sampler.hpp"
#include "sample.hpp"
#include <iostream>
#include <cstdlib>

Sampler::Sampler(int _xstart, int _xend, int _ystart, int _yend) : 
	xPixelStart(_xstart), xPixelEnd(_xend), yPixelStart(_ystart), yPixelEnd(_yend)
{

	dx = _xend - _xstart;
	dy = _yend - _ystart;

}

CameraSample *Sampler::getSamples(int _count) {

	//number of tiles in each dimension
	CameraSample *array = new CameraSample[_count];
	int ntx = _count, nty = 1;
	while ((ntx & 0x1) == 0 && 2 * dx * nty < dy * ntx) {
		ntx >>= 1;
		nty <<= 1;
	}
	//if (yPos == yPixelEnd) return 0;
	std::cout<<ntx*nty<<'\n';
	float sampleBuf[_count*2];
	//determine number of tiles from which to take sample in each dimension
	//generate stratified samples, currently obsolete
	StratifiedSample2D(sampleBuf, ntx, nty);
	//move samples their locations
	for(int i = 0; i < _count; i++) {
		array[i].imageX = sampleBuf[2*i];
		array[i].imageY = sampleBuf[2*i+1];
	}
	/*
	   if (++xPos == xPixelEnd) {
	   xPos = xPixelStart;
	   ++yPos;
	   }
	   */
	return array;
}

CameraSample *Sampler::sampleForEachPixel() {
	CameraSample *array = new CameraSample[dx*dy];
	float sampleBuf[dx*dy*2];
	StratifiedSample2D(sampleBuf, dx, dy);
	for(int i = 0; i < dx*dy; i++) {
		array[i].imageX = sampleBuf[2*i];
		array[i].imageY = sampleBuf[2*i+1];
	}
	return array;

}

void Sampler::StratifiedSample2D(float *samp, int _nx, int _ny) {
	//float dx = 1.f/nx, dy = 1.f / ny;
	for(int y = 0; y < _ny; y++) {
		for(int x = 0; x < _nx; x++) {
			//todo add jitter
			*samp++ =  (dx/_nx) * (x + 0.5);//(x + 0.5) *dx;
			*samp++ =  (dy/_ny) * (y + 0.5);//(y + 0.5) *dy;
		}
	}
}
