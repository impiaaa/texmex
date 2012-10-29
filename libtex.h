#ifndef LIBTEX_H
#define LIBTEX_H

#include <stdlib.h>
#include "pixfmts.h"
#include "compression/compression.h"

#define max(a, b) ((a < b) ? b : a)

typedef enum TMLogLevel {
	TMLogLevelQuiet, // Don't use when logging something. Intended to silence all output.
	TMLogLevelError,
	TMLogLevelWarning,
	TMLogLevelInfo,
	TMLogLevelDebug,
	TMLogLevelAll // NOT for normal use. Choose one of the above instead.
} TMLogLevel;

extern TMLogLevel TMLogGlobalLevel;

void TMLogSetLevel(TMLogLevel i);
void TMLog(TMLogLevel level, char *message, ...);

#define TMLogError(message, ...) TMLog(TMLogLevelError, message, __VA_ARGS__)
#define TMLogWarning(message, ...) TMLog(TMLogLevelWarning, message, __VA_ARGS__)
#define TMLogInfo(message, ...) TMLog(TMLogLevelInfo, message, __VA_ARGS__)
#define TMLogDebug(message, ...) TMLog(TMLogLevelDebug, message, __VA_ARGS__)

typedef struct TMContainerType {
	const char *name;
	const char *longname;
	const char *mime_types; // comma-separated
	const char *extensions; // comma-separated
} TMContainerType;

typedef struct TMTexture {
	TMCompressionType compression;
	TMPixelFormat pixfmt;
	unsigned short width;
	unsigned short height;
	unsigned short depth; // 3D textures use 2D slices
	short xOffset; // from upper-left
	short yOffset;
	short zOffset; // from starting slice
	unsigned char mipmapCount;
	void *mipmaps; // Each mipmap contains (mipmap depth) slices, and then the 2D image is codec-dependent. Data is contiguous. Width, height, and depth decrease by a power of two each time.
} TMTexture;

typedef struct TMSequence {
	unsigned short frameCount; // 0 for non-animations
	unsigned short startFrame;
	TMTexture **frames; // array of pointers
	// All formats I've seen either have per-frame delays or a global FPS.
	// In the case of global FPS, set delays to NULL.
	double *delays;
	unsigned int globalFpsNumerator;
	unsigned int globalFpsDenominator;
} TMSequence;

typedef struct TMTextureCollection {
	// to be loaded from a file
	TMTexture *thumbnail; // set to NULL for no thumbnail
	unsigned short sequenceCount; // some files have multiple textures, or faces for a cubemap
	TMSequence **sequences; // array of pointers
} TMTextureCollection;

#define TMCalloc calloc
#define TMFree free
#define TMMalloc malloc
#define TMRealloc realloc

#endif
