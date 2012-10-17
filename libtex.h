#ifndef LIBTEX_H
#define LIBTEX_H

typedef struct Texture {
	int codecId;
	unsigned short width;
	unsigned short height;
	unsigned short depth; // 3D textures use 2D slices
	short xOffset;
	short yOffset;
	short zOffset;
	unsigned char mipmapCount;
	void *mipmaps; // Each mipmap contains (depth) slices, and then the 2D image is codec-dependent. Data is contiguous.
} Texture;

typedef struct Sequence {
	unsigned short frameCount; // 0 for non-animations
	unsigned short startFrame;
	Texture **frames;
	// All formats I've seen either have per-frame delays or a global FPS.
	// In the case of global FPS, set delays to NULL.
	double *delays;
	unsigned int globalFpsNumerator;
	unsigned int globalFpsDenominator;
} Sequence;

typedef struct TextureCollection {
	// to be loaded from a file
	Texture *thumbnail; // set to NULL for no thumbnail
	unsigned short sequenceCount; // some files have multiple textures, or faces for a cubemap
	Sequence **sequences;
}
TextureCollection;

#endif