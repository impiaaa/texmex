#ifndef TM_PIXFMT_H
#define TM_PIXFMT_H

typedef enum TMPixelFormat {
	TMPixelFormatNone,
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
	I4,
	I8,
	IA44,
	IA88,
	RGB565,
	RGB888,
	RGBA8888,
	RGB5A3, // GC exclusive - "RGB5A3 is RGB5 if color value is negative and RGB4A3 otherwise." - yagcd
} TMPixelFormat;

#endif // TM_PIXFMT_H
