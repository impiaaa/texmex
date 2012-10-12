#include <stdlib.h>
#include <stdint.h>

size_t calcSize(int pixelBitWidth, int width, int height, int mipmapCount);

size_t _calcSize(int pixelBitWidth, int width, int height, int mipmapCount, size_t accum) {
	// woo tail recursion
	if (mipmapCount == 0)
		return accum;
	else
		return _calcSize(pixelBitWidth, width/2, height/2, mipmapCount-1, (width*height*pixelBitWidth)/8+accum);
}

size_t calcSize(int pixelBitWidth, int width, int height, int mipmapCount) {
	return _calcSize(pixelBitWidth, width, height, mipmapCount, 0);
}

uint16_t rgb565interp(uint16_t color0, uint16_t color1, char weight, char divisor) {
	char r, g, b;
	char r0, g0, b0;
	char r1, g1, b1;
	r0 = (color0 & 0xF800) >> 11;
	g0 = (color0 & 0x07E0) >>  5;
	b0 = (color0 & 0x001F);
	r1 = (color1 & 0xF800) >> 11;
	g1 = (color1 & 0x07E0) >>  5;
	b1 = (color1 & 0x001F);
	r = (weight * r0 + r1) / divisor;
	g = (weight * r0 + r1) / divisor;
	b = (weight * r0 + r1) / divisor;
	return (r<<11) | (g << 5) | (b);
}

char *DXT1toRGB565(char *dataIn, int width, int height, int mipmapCount) {
	int mipmap, x, y, mipWidth, mipHeight;
	uint16_t colors[4];
	uint32_t pixels;
	int mipIdxIn, mipIdxOut;
	int chunkIdx;
	char *dataOut = malloc(calcSize(16, width, height, mipmapCount));
	for (mipmap = 0; mipmap < mipmapCount; mipmap++) {
		mipIdxIn = calcSize(4, width, height, mipmap);
		mipIdxOut = calcSize(16, width, height, mipmap);
		mipWidth = width/(1<<mipmap);
		mipHeight = height/(1<<mipmap);
		for (y = 0; y < mipHeight; y += 4) {
			for (x = 0; x < mipWidth; x += 4) {
				chunkIdx = mipIdxIn+(x+(y*mipWidth))/2;
				colors[0] = *(dataIn+chunkIdx+0);
				colors[1] = *(dataIn+chunkIdx+2);
				pixels = *(dataIn+chunkIdx+4);
				if (colors[0] > colors[1]) {
					colors[2] = rgb565interp(colors[0], colors[1], 2, 3);
					colors[3] = rgb565interp(colors[1], colors[2], 2, 3);
				}
				else {
					colors[2] = rgb565interp(colors[1], colors[2], 1, 2);
					colors[3] = 0;
				}
				int xs, ys;
				for (ys = 0; ys < 4; ys++) {
					for (xs = 0; xs < 4; xs++) {
						int shamt = (y+(x*4))*2;
						uint16_t pixel = colors[(pixels&(3<<shamt))>>shamt];
						*(dataOut+(x+(y*mipWidth))*2) = pixel;
					}
				}
			}
		}
	}
	return dataOut;
}
