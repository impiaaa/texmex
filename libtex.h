#ifndef LIBTEX_H
#define LIBTEX_H

#include <stdlib.h>
#include "codecs/codecs.h"

typedef struct TMContainerType {
	const char *name;
	const char *longname;
	const char *mime_types; // comma-separated
	const char *extensions; // comma-separated
} TMContainerType;

typedef struct TMTexture {
	TMCodecType codec;
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
	TMTexture **frames;
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
	TMSequence **sequences;
} TMTextureCollection;

typedef struct TMAllocator {
	void * (*calloc)(size_t, size_t);
	void (*free)(void *);
	void * (*malloc)(size_t);
	void * (*realloc)(void *, size_t);
} TMAllocator;

TMAllocator tmDefaultAllocator;

#endif
