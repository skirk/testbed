#include "sampler.hpp"
#include "sample.hpp"
#include <iostream>
#include <cstdlib>
#include <math.h>

Sampler::Sampler(int _xstart, int _xend, int _ystart, int _yend) : 
	xPixelStart(_xstart), xPixelEnd(_xend), yPixelStart(_ystart), yPixelEnd(_yend)
{

	dx = _xend - _xstart;
	dy = _yend - _ystart;

}

LightSample *Sampler::getSamples(int _count) {

	//number of tiles in each dimension
	LightSample *array = new LightSample[_count];
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
	stratifiedSample2D(sampleBuf, ntx, nty);
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

float Lerp(float t, float v1, float v2) {
	return (1.f - t) * v1 + t * v2;
}



void Sampler::computeSubWindow(int _num, int _count, int *_newXstart, int *_newXend,int *_newYstart, int *_newYend) const {

	int newx0= dx/_count * _num;
	int newx1= dx/_count * (_num+1);
	std::cout<<"new dimensions "<<newx0<<" "<<newx1<<'\n';
	std::cout<<"new dimensions "<<yPixelStart<<" "<<yPixelEnd<<'\n';

	//float tx0 = float(xo) / float(ntx), tx1 = float(xo + 1) / float(ntx);
	//float ty0 = float(yo) / float(nty), ty1 = float(yo + 1) / float(nty);
	*_newXstart = newx0;//(int)floorf(Lerp(tx0, xPixelStart, xPixelEnd));
	*_newXend =   newx1;//(int)floorf(Lerp(tx1, xPixelStart, xPixelEnd));
	*_newYstart = yPixelStart;//(int)floorf(Lerp(ty0, yPixelStart, yPixelEnd));
	*_newYend =   yPixelEnd;//(int)floorf(Lerp(ty1, yPixelStart, yPixelEnd));

}

Sampler* Sampler::getSubSampler(int num, int count) {
	int x0, x1, y0, y1;
	computeSubWindow(num, count, &x0, &x1, &y0, &y1);
	return new Sampler(x0, x1, y0, y1);


}

void Sampler::getSubSamplers(std::vector<Sampler*> *_samplers, int num) {
	for(int i = 0; i < num; i ++) {	
		_samplers->push_back(getSubSampler(i, num));
	}
}

LightSample *Sampler::sampleForEachPixel() {
	LightSample *array = new LightSample[dx*dy];
	float sampleBuf[dx*dy*2];
	stratifiedSample2D(sampleBuf, dx, dy);
	for(int i = 0; i < dx*dy; i++) {
		array[i].imageX = sampleBuf[2*i];
		array[i].imageY = sampleBuf[2*i+1];
	}
	return array;

}

void Sampler::stratifiedSample2D(float *samp, int _nx, int _ny) {
	//float dx = 1.f/nx, dy = 1.f / ny;
	for(int y = yPixelStart; y < _ny +yPixelStart; y++) {
		for(int x = xPixelStart; x < _nx + xPixelStart; x++) {
			//todo add jitter
			*samp++ =  (dx/_nx) * (x + 0.5);//(x + 0.5) *dx;
			*samp++ =  (dy/_ny) * (y + 0.5);//(y + 0.5) *dy;
		}
	}
}
