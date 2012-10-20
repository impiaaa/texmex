#ifndef TM_COMPRESSION_H
#define TM_COMPRESSION_H

typedef enum TMCompressionType {
	TMNoCompression = 0,
	RLE,
	DXT1 = 'DXT1',
	DXT2 = 'DXT2',
	DXT3 = 'DXT3',
	DXT4 = 'DXT4',
	DXT5 = 'DXT5',
	PVRTC = 'PVR!',
	ThreeDc = 'ATI2',
	ThreeDcPlus = 'ATI1',
} TMCompressionType;

#endif // TM_COMPRESSION_H
