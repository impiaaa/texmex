typedef enum TMCodecType {
	// Raw pixel formats
	A8,
	ABGR8888,
	ARGB8888,
	BGR565,
	BGR888,
	BGRA4444,
	BGRA5551,
	BGRA8888,
	BGRX5551, // X = unused
	BGRX8888,
	I8,
	IA88,
	RGB565,
	RGB888,
	RGBA8888,
	// Compressed formats
	DXT1 = 'DXT1',
	DXT2 = 'DXT2',
	DXT3 = 'DXT3',
	DXT4 = 'DXT4',
	DXT5 = 'DXT5',
	PVRTC = 'PVR!',
	// Lossless compression used in TGA
	RLE,
} TMCodecType;
