#pragma once

class CameraSample;

class Sampler {
	public:
		Sampler(int xstart, int xend, int ystart, int yend);
		const int xPixelStart, xPixelEnd, yPixelStart, yPixelEnd;
		//dimensions in terms of pixels
		int dx, dy;

		CameraSample *getSamples(int _count);
		CameraSample *sampleForEachPixel();

		void StratifiedSample2D(float *samp, int _nx, int _ny);
};

