#ifndef TM_PIXFMT_H
#define TM_PIXFMT_H

typedef enum TMPixelFormat {
	TMPixelFormatNone,
	A4,       //  4-bit
	A8,       //  8-bit
	ABGR8888, // 32-bit
	ARGB8888, // 32-bit
	BGR565,   // 16-bit
	BGR888,   // 24-bit
	BGRA4444, // 16-bit
	BGRA5551, // 16-bit
	BGRA8888, // 32-bit
	BGRX5551, // 16-bit // X = unused
	BGRX8888, // 32-bit
	I4,       //  4-bit
	I8,       //  8-bit
	IA44,     //  8-bit
	IA88,     // 16-bit
	RGB565,   // 16-bit
	RGB888,   // 24-bit
	RGBA8888, // 32-bit
	RGBA53,   // 16-bit // GC exclusive - "RGB5A3 is RGB5 if color value is negative and RGB4A3 otherwise." - yagcd
} TMPixelFormat;

#endif // TM_PIXFMT_H
