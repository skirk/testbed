#pragma once
#include <vector>

class LightSample;

class Sampler {
	public:
		Sampler(int xstart, int xend, int ystart, int yend);
		const int xPixelStart, xPixelEnd, yPixelStart, yPixelEnd;
		//dimensions in terms of pixels
		int dx, dy;

		//get _count many samples distributed evenly on normalized aread
		LightSample *getSamples(int _count);
		//get a sample for each pixel
		std::vector<LightSample> *sampleForEachPixel();
		//divide image into tiles and create a subsampler for each tile and return them
		void getSubSamplers(std::vector<Sampler*> *_samplers, int num);

	private:
		void stratifiedSample2D(float *samp, int _nx, int _ny);
		void computeSubWindow(int _num, int _count, int *newXstart, int *newXend, int *newYstart, int *newYend) const;
		Sampler *getSubSampler(int num, int count);
};

