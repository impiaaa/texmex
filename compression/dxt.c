#include <stdlib.h>
#include <stdint.h>

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

char *DXT1toRGB565(char *dataIn, unsigned short width, unsigned short height) {
	int x, y;
	uint16_t colors[4];
	uint32_t pixels;
	int chunkIdx;
	char *dataOut = malloc(16*width*height);
	for (y = 0; y < height; y += 4) {
		for (x = 0; x < width; x += 4) {
			chunkIdx = (x+(y*width))/2;
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
					*(dataOut+(x+(y*width))*2) = pixel;
				}
			}
		}
	}
	return dataOut;
}
