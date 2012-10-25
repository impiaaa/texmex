#include <stdlib.h>
#include <stdint.h>
#include "../libtex.h"

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
	g = (weight * g0 + g1) / divisor;
	b = (weight * b0 + b1) / divisor;
	return (r << 11) | (g << 5) | (b);
}

uint32_t rgba8888interp(uint32_t color0, uint32_t color1, char weight, char divisor) {
	unsigned char r, g, b, a;
	unsigned char r0, g0, b0, a0;
	unsigned char r1, g1, b1, a1;
	r0 = (color0 & 0xFF000000) >> 24;
	g0 = (color0 & 0x00FF0000) >> 16;
	b0 = (color0 & 0x0000FF00) >>  8;
	a0 = (color0 & 0x000000FF) >>  0;
	r1 = (color1 & 0xFF000000) >> 24;
	g1 = (color1 & 0x00FF0000) >> 16;
	b1 = (color1 & 0x0000FF00) >>  8;
	a1 = (color1 & 0x000000FF) >>  0;
	r = (weight * r0 + r1) / divisor;
	g = (weight * g0 + g1) / divisor;
	b = (weight * b0 + b1) / divisor;
	a = (weight * a0 + a1) / divisor;
	return (r << 24) | (g << 16) | (b << 8) | a;
}

uint32_t rgb565expand(uint16_t color0) {
	return ((color0 & 0xF800) << 16) | ((color0 & 0x07E0) << 13) | ((color0 & 0x001F) << 11) | 0xFF;
}

char *DXT1toRGB565(char *dataIn, unsigned short width, unsigned short height) {
	int x, y;
	uint16_t colors[4];
	uint32_t pixels;
	int chunkIdx;
	char *dataOut = malloc(2*width*height);
	for (y = 0; y < height; y += 4) {
		for (x = 0; x < width; x += 4) {
			chunkIdx = ((y * width / 2) + x * 2);
			colors[0] = (((uint32_t)(dataIn[chunkIdx+1]))<<8)|(((uint32_t)(dataIn[chunkIdx+0])));
			colors[1] = (((uint32_t)(dataIn[chunkIdx+3]))<<8)|(((uint32_t)(dataIn[chunkIdx+2])));
			pixels  = (dataIn[chunkIdx+4  ])<<24;
			pixels |= (dataIn[chunkIdx+4+1])<<16;
			pixels |= (dataIn[chunkIdx+4+2])<< 8;
			pixels |= (dataIn[chunkIdx+4+3]);
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
					int shamt = (((3-ys) * 4) + xs)*2;
					uint32_t pixel = colors[(pixels&(3<<shamt))>>shamt];
					char *outp = dataOut + (((y + ys) * width) + (x + xs))*4;
					*(dataOut+(x+(y*width))*2) = pixel;
					*(dataOut+(x+(y*width))*2+1) = pixel>>8;
				}
			}
		}
	}
	return dataOut;
}

char *DXT1AtoRGBA8888(unsigned char *dataIn, unsigned short width, unsigned short height) {
	int x, y;
	uint32_t colors[4];
	memset(colors, 0, 4*sizeof(uint32_t));
	uint32_t pixels;
	uint32_t chunkIdx;
	char *dataOut = malloc(4*width*height);
	for (y = 0; y < height; y += 4) {
		for (x = 0; x < width; x += 4) {
			chunkIdx = ((y * width / 2) + x * 2);
			colors[0] = rgb565expand((((uint32_t)(dataIn[chunkIdx+1]))<<8)|(((uint32_t)(dataIn[chunkIdx+0]))));
			colors[1] = rgb565expand((((uint32_t)(dataIn[chunkIdx+3]))<<8)|(((uint32_t)(dataIn[chunkIdx+2]))));
			pixels  = (dataIn[chunkIdx+4  ])<<24;
			pixels |= (dataIn[chunkIdx+4+1])<<16;
			pixels |= (dataIn[chunkIdx+4+2])<< 8;
			pixels |= (dataIn[chunkIdx+4+3]);
			if (colors[0] > colors[1]) {
				colors[2] = rgba8888interp(colors[0], colors[1], 2, 3);
				colors[3] = rgba8888interp(colors[1], colors[2], 2, 3);
			}
			else {
				colors[2] = rgba8888interp(colors[1], colors[2], 1, 2);
				colors[3] = 0;
			}
			int xs, ys;
			for (ys = 0; ys < 4; ys++) {
				for (xs = 0; xs < 4; xs++) {
					int shamt = (((3-ys) * 4) + xs)*2;
					uint32_t pixel = colors[(pixels&(3<<shamt))>>shamt];
					char *outp = dataOut + (((y + ys) * width) + (x + xs))*4;
					*(outp  ) = pixel>> 8;
					*(outp+1) = pixel>>16;
					*(outp+2) = pixel>>24;
					*(outp+3) = pixel;
				}
			}
		}
	}
	return dataOut;
}
