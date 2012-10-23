#include "containers.h"
#include "../compression/compression.h"
#include "../libtex.h"
#include "../pixfmts.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TMContainerType tmDdsContainer = {
	.name = "dds",
	.longname = "DirectDraw Surface (DDS)",
	.mime_types = "image/x-dds",
	.extensions = "dds"
};

// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943991(v=vs.85).aspx

enum TMDDSFlags {
	TMDDS_CAPS = 0x1,
	TMDDS_HEIGHT = 0x2,
	TMDDS_WIDTH = 0x4,
	TMDDS_PITCH = 0x8,
	TMDDS_PIXELFORMAT = 0x1000,
	TMDDS_MIPMAPCOUNT = 0x20000,
	TMDDS_LINEARSIZE = 0x80000,
	TMDDS_DEPTH = 0x800000,
};

enum TMDDSCaps2Flags {
	TMDDS_CUBEMAP = 0x200,
	TMDDS_CUBEMAP_POSITIVEX = 0x400,
	TMDDS_CUBEMAP_NEGATIVEX = 0x800,
	TMDDS_CUBEMAP_POSITIVEY = 0x1000,
	TMDDS_CUBEMAP_NEGATIVEY = 0x2000,
	TMDDS_CUBEMAP_POSITIVEZ = 0x4000,
	TMDDS_CUBEMAP_NEGATIVEZ = 0x8000,
	TMDDS_VOLUME = 0x200000
};

enum TMDDSPixelFormatFlags {
	TMDDS_ALPHAPIXELS = 0x1,
	TMDDS_ALPHA = 0x2,
	TMDDS_FOURCC = 0x4,
	TMDDS_RGB = 0x40,
	TMDDS_YUV = 0x200,
	TMDDS_LUMINANCE = 0x20000
};

typedef struct __attribute__((packed)) TMDDSPixelFormat {
	uint32_t structSize;
	uint32_t flags;
	uint32_t fourCC;
	uint32_t rgbBitCount;
	uint32_t rBitMask;
	uint32_t gBitMask;
	uint32_t bBitMask;
	uint32_t aBitMask;
} TMDDSPixelFormat;

typedef struct __attribute__((packed)) TMDDSHeader {
	uint32_t magic;
	uint32_t structSize;
	uint32_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitchOrLinearSize;
	uint32_t depth;
	uint32_t mipMapCount;
	uint32_t reserved1[11];
	TMDDSPixelFormat pixelFormat;
	uint32_t caps;
	uint32_t caps2;
	uint32_t caps3;
	uint32_t caps4;
	uint32_t reserved2;
} TMDDSHeader;

TMTextureCollection *tmDdsRead(FILE *inStream) {
	TMDDSHeader header;
	TMTextureCollection *ret;
	size_t read;

	read = fread(&header, sizeof(TMDDSHeader), 1, inStream);
	if (read != 1) {
		fprintf(stderr, "Could not load header\n");
		return NULL;
	}
	if (header.magic != ' SDD') {
		fprintf(stderr, "Incorrect magic 0x%08X\n", header.magic);
		return NULL;
	}
	
	ret = TMMalloc(sizeof(TMTextureCollection));
	ret->sequenceCount = 1;
	if (header.caps2 & TMDDS_CUBEMAP) {
		if (header.caps2 & TMDDS_CUBEMAP_POSITIVEX)
			ret->sequenceCount++;
		if (header.caps2 & TMDDS_CUBEMAP_POSITIVEY)
			ret->sequenceCount++;
		if (header.caps2 & TMDDS_CUBEMAP_POSITIVEZ)
			ret->sequenceCount++;
		if (header.caps2 & TMDDS_CUBEMAP_NEGATIVEX)
			ret->sequenceCount++;
		if (header.caps2 & TMDDS_CUBEMAP_NEGATIVEY)
			ret->sequenceCount++;
		if (header.caps2 & TMDDS_CUBEMAP_NEGATIVEZ)
			ret->sequenceCount++;
	}
	ret->sequences = TMCalloc(ret->sequenceCount, sizeof(void*));
	
	int i;
	for (i = 0; i < ret->sequenceCount; i++) {
		ret->sequences[i] = TMMalloc(sizeof(TMSequence));
		ret->sequences[i]->frameCount = 0;
		ret->sequences[i]->startFrame = 0;
		ret->sequences[i]->frames = TMMalloc(sizeof(void*));
		ret->sequences[i]->frames[0] = TMMalloc(sizeof(TMTexture));
		
		if (header.pixelFormat.flags & TMDDS_ALPHA) {
			if (header.pixelFormat.flags & TMDDS_ALPHA) {
				if (header.pixelFormat.rgbBitCount == 8) {
					ret->sequences[i]->frames[0]->compression = TMNoCompression;
					ret->sequences[i]->frames[0]->pixfmt = IA44;
				}
				else if (header.pixelFormat.rgbBitCount == 4) {
					ret->sequences[i]->frames[0]->compression = TMNoCompression;
					ret->sequences[i]->frames[0]->pixfmt = IA88;
				}
				else {
					fprintf(stderr, "Alpha+luminance bit width %d unsupported\n", header.pixelFormat.rgbBitCount);
					TMFree(ret->sequences[i]->frames[0]);
					TMFree(ret->sequences[i]->frames);
					TMFree(ret->sequences[i]);
					TMFree(ret->sequences);
					TMFree(ret);
					return NULL;
				}
			}
			else {
				if (header.pixelFormat.rgbBitCount == 8) {
					ret->sequences[i]->frames[0]->compression = TMNoCompression;
					ret->sequences[i]->frames[0]->pixfmt = A8;
				}
				else if (header.pixelFormat.rgbBitCount == 4) {
					ret->sequences[i]->frames[0]->compression = TMNoCompression;
					ret->sequences[i]->frames[0]->pixfmt = A4;
				}
				else {
					fprintf(stderr, "Alpha-only bit width %d unsupported\n", header.pixelFormat.rgbBitCount);
					TMFree(ret->sequences[i]->frames[0]);
					TMFree(ret->sequences[i]->frames);
					TMFree(ret->sequences[i]);
					TMFree(ret->sequences);
					TMFree(ret);
					return NULL;
				}
			}
		}
		else if (header.pixelFormat.flags & TMDDS_FOURCC) {
			if (header.pixelFormat.fourCC == '1TXD') {
				ret->sequences[i]->frames[0]->compression = DXT1;
				ret->sequences[i]->frames[0]->pixfmt = RGB565;
			}
			else {
				fprintf(stderr, "Unknown FOURCC 0x%08X\n", header.pixelFormat.fourCC);
				TMFree(ret->sequences[i]->frames[0]);
				TMFree(ret->sequences[i]->frames);
				TMFree(ret->sequences[i]);
				TMFree(ret->sequences);
				TMFree(ret);
				return NULL;
			}
		}
		else if (header.pixelFormat.flags & TMDDS_RGB) {
			if (header.pixelFormat.rgbBitCount == 16) {
				
			}
			else if (header.pixelFormat.rgbBitCount == 24) {
				
			}
			else if (header.pixelFormat.rgbBitCount == 32) {
				
			}
			else {
				fprintf(stderr, "Unknown bit width %d\n", header.pixelFormat.rgbBitCount);
				TMFree(ret->sequences[i]->frames[0]);
				TMFree(ret->sequences[i]->frames);
				TMFree(ret->sequences[i]);
				TMFree(ret->sequences);
				TMFree(ret);
				return NULL;
			}
		}
		else if (header.pixelFormat.flags & TMDDS_LUMINANCE) {
			if (header.pixelFormat.rgbBitCount == 8) {
				ret->sequences[i]->frames[0]->compression = TMNoCompression;
				ret->sequences[i]->frames[0]->pixfmt = I8;
			}
			else if (header.pixelFormat.rgbBitCount == 4) {
				ret->sequences[i]->frames[0]->compression = TMNoCompression;
				ret->sequences[i]->frames[0]->pixfmt = I4;
			}
			else {
				fprintf(stderr, "Luminance bit width %d unsupported\n", header.pixelFormat.rgbBitCount);
				TMFree(ret->sequences[i]->frames[0]);
				TMFree(ret->sequences[i]->frames);
				TMFree(ret->sequences[i]);
				TMFree(ret->sequences);
				TMFree(ret);
				return NULL;
			}
		}
		else {
			fprintf(stderr, "Unknown pixel format 0x%08X\n", header.pixelFormat.flags);
			TMFree(ret->sequences[i]->frames[0]);
			TMFree(ret->sequences[i]->frames);
			TMFree(ret->sequences[i]);
			TMFree(ret->sequences);
			TMFree(ret);
			return NULL;
		}
		
		ret->sequences[i]->frames[0]->width = header.width;
		ret->sequences[i]->frames[0]->height = header.height;
		if (header.flags & TMDDS_DEPTH)
			ret->sequences[i]->frames[0]->depth = header.depth;
		else
			ret->sequences[i]->frames[0]->depth = 1;
		ret->sequences[i]->frames[0]->xOffset = 0;
		ret->sequences[i]->frames[0]->yOffset = 0;
		ret->sequences[i]->frames[0]->mipmapCount = header.mipMapCount;
		
		unsigned long imageDataSize = ((max(1, ((header.width+3)/4)) * header.pixelFormat.rgbBitCount)/8)*header.height;
		ret->sequences[i]->frames[0]->mipmaps = TMMalloc(imageDataSize);
		fread(ret->sequences[i]->frames[0]->mipmaps, imageDataSize, 1, inStream);
	}
	return ret;
}
