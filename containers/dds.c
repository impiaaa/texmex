#include "containers.h"
#include "../compression/compression.h"
#include "../libtex.h"
#include "../pixfmts.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// http://msdn.microsoft.com/en-us/library/windows/desktop/bb943991(v=vs.85).aspx

TMContainerType tmDdsContainer = {
	.name = "dds",
	.longname = "DirectDraw Surface (DDS)",
	.mime_types = "image/x-dds",
	.extensions = "dds",
	.reader = tmDdsRead
};

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
		TMLogError("Could not load header");
		return NULL;
	}
	if (header.magic != ' SDD') {
		TMLogError("Incorrect magic 0x%08X", header.magic);
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
					TMLogError("Alpha+luminance bit width %d unsupported", header.pixelFormat.rgbBitCount);
					goto fail;
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
					TMLogError("Alpha-only bit width %d unsupported", header.pixelFormat.rgbBitCount);
					goto fail;
				}
			}
		}
		else if (header.pixelFormat.flags & TMDDS_FOURCC) {
			if (header.pixelFormat.fourCC == '1TXD') {
				ret->sequences[i]->frames[0]->compression = DXT1;
				ret->sequences[i]->frames[0]->pixfmt = RGB565;
			}
			else if (header.pixelFormat.fourCC == '2TXD') {
				ret->sequences[i]->frames[0]->compression = DXT2;
				ret->sequences[i]->frames[0]->pixfmt = RGB565;
			}
			else if (header.pixelFormat.fourCC == '3TXD') {
				ret->sequences[i]->frames[0]->compression = DXT3;
				ret->sequences[i]->frames[0]->pixfmt = RGB565;
			}
			else if (header.pixelFormat.fourCC == '4TXD') {
				ret->sequences[i]->frames[0]->compression = DXT4;
				ret->sequences[i]->frames[0]->pixfmt = RGB565;
			}
			else if (header.pixelFormat.fourCC == '5TXD') {
				ret->sequences[i]->frames[0]->compression = DXT5;
				ret->sequences[i]->frames[0]->pixfmt = RGB565;
			}
			else {
				TMLogError("Unknown FOURCC 0x%08X", header.pixelFormat.fourCC);
				goto fail;
			}
		}
		else if (header.pixelFormat.flags & TMDDS_RGB) {
			if (header.pixelFormat.rgbBitCount == 16) {
				ret->sequences[i]->frames[0]->compression = TMNoCompression;
				if ((header.pixelFormat.rBitMask == 0x001f) && (header.pixelFormat.gBitMask == 0x07e0) && (header.pixelFormat.bBitMask == 0xf800))
					ret->sequences[i]->frames[0]->pixfmt = BGR565;
				else if ((header.pixelFormat.rBitMask == 0x00f0) && (header.pixelFormat.gBitMask == 0x0f00) && (header.pixelFormat.bBitMask == 0xf000) && (header.pixelFormat.aBitMask == 0x000f))
					ret->sequences[i]->frames[0]->pixfmt = BGRA4444;
				else if ((header.pixelFormat.rBitMask == 0x003e) && (header.pixelFormat.gBitMask == 0x07c0) && (header.pixelFormat.bBitMask == 0xf800) && (header.pixelFormat.aBitMask == 0x0001))
					ret->sequences[i]->frames[0]->pixfmt = BGRA5551;
				else if ((header.pixelFormat.rBitMask == 0x003e) && (header.pixelFormat.gBitMask == 0x07c0) && (header.pixelFormat.bBitMask == 0xf800))
					ret->sequences[i]->frames[0]->pixfmt = BGRX5551;
				else if ((header.pixelFormat.rBitMask == 0xf800) && (header.pixelFormat.gBitMask == 0x07e0) && (header.pixelFormat.bBitMask == 0x001f))
					ret->sequences[i]->frames[0]->pixfmt = RGB565;
				else {
					TMLogError("Unknown 16-bit color format 0x%04X, 0x%04X, 0x%04X", header.pixelFormat.rBitMask, header.pixelFormat.gBitMask, header.pixelFormat.bBitMask);
					goto fail;
				}
			}
			else if (header.pixelFormat.rgbBitCount == 24) {
				ret->sequences[i]->frames[0]->compression = TMNoCompression;
				if ((header.pixelFormat.rBitMask == 0x0000ff) && (header.pixelFormat.gBitMask == 0x00ff00) && (header.pixelFormat.bBitMask == 0xff0000))
					ret->sequences[i]->frames[0]->pixfmt = BGR888;
				else if ((header.pixelFormat.rBitMask == 0xff0000) && (header.pixelFormat.gBitMask == 0x00ff00) && (header.pixelFormat.bBitMask == 0x0000ff))
					ret->sequences[i]->frames[0]->pixfmt = RGB888;
				else {
					TMLogError("Unknown 24-bit color format 0x%06X, 0x%06X, 0x%06X", header.pixelFormat.rBitMask, header.pixelFormat.gBitMask, header.pixelFormat.bBitMask);
					goto fail;
				}
			}
			else if (header.pixelFormat.rgbBitCount == 32) {
				ret->sequences[i]->frames[0]->compression = TMNoCompression;
				if (header.pixelFormat.rBitMask == 0xff000000) {
					if ((header.pixelFormat.gBitMask == 0x00ff0000) && (header.pixelFormat.bBitMask == 0x0000ff00) && (header.pixelFormat.aBitMask == 0x000000ff)) {
						ret->sequences[i]->frames[0]->pixfmt = RGBA8888;
					}
					else {
						TMLogError("Unknown color format 0x%06X, 0x%06X, 0x%06X", header.pixelFormat.rBitMask, header.pixelFormat.gBitMask, header.pixelFormat.bBitMask);
						goto fail;
					}
				}
				else if (header.pixelFormat.bBitMask == 0xff000000) {
					if ((header.pixelFormat.gBitMask == 0x00ff0000) && (header.pixelFormat.aBitMask == 0x0000ff00)) {
						if (header.pixelFormat.aBitMask == 0x000000ff)
							ret->sequences[i]->frames[0]->pixfmt = BGRA8888;
						else
							ret->sequences[i]->frames[0]->pixfmt = BGRX8888;
					}
					else {
						TMLogError("Unknown color format 0x%06X, 0x%06X, 0x%06X", header.pixelFormat.rBitMask, header.pixelFormat.gBitMask, header.pixelFormat.bBitMask);
						goto fail;
					}
				}
				else if (header.pixelFormat.aBitMask == 0xff000000) {
					if ((header.pixelFormat.bBitMask == 0x00ff0000) && (header.pixelFormat.gBitMask == 0x0000ff00) && (header.pixelFormat.rBitMask == 0x000000ff))
						ret->sequences[i]->frames[0]->pixfmt = ABGR8888;
					else if ((header.pixelFormat.rBitMask == 0x00ff0000) && (header.pixelFormat.gBitMask == 0x0000ff00) && (header.pixelFormat.bBitMask == 0x000000ff))
						ret->sequences[i]->frames[0]->pixfmt = ARGB8888;
					else {
						TMLogError("Unknown color format 0x%06X, 0x%06X, 0x%06X", header.pixelFormat.rBitMask, header.pixelFormat.gBitMask, header.pixelFormat.bBitMask);
						goto fail;
					}
				}
				else {
					TMLogError("Unknown color format 0x%06X, 0x%06X, 0x%06X", header.pixelFormat.rBitMask, header.pixelFormat.gBitMask, header.pixelFormat.bBitMask);
					goto fail;
				}
			}
			else {
				TMLogError("Unknown bit width %d", header.pixelFormat.rgbBitCount);
				goto fail;
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
				TMLogError("Luminance bit width %d unsupported", header.pixelFormat.rgbBitCount);
				goto fail;
			}
		}
		else {
			TMLogError("Unknown pixel format 0x%08X", header.pixelFormat.flags);
			goto fail;
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
		
		unsigned long imageDataMipSize, imageDataSize = 0;
		if (header.pixelFormat.flags & TMDDS_FOURCC) {
			imageDataMipSize = max(1, ((header.width+3)/4));
			if (header.pixelFormat.fourCC == '1TXD')
				imageDataMipSize *= 2;
			else
				imageDataMipSize *= 4;
		}
		else
			imageDataMipSize = (header.width * header.pixelFormat.rgbBitCount + 7) / 8;
		imageDataMipSize *= header.height;
		
		int mip;
		for (mip = 0; mip < header.mipMapCount; mip++) {
			imageDataSize += imageDataMipSize;
			imageDataMipSize /= 4;
		}
		ret->sequences[i]->frames[0]->mipmaps = TMMalloc(imageDataSize);
		memset(ret->sequences[i]->frames[0]->mipmaps, 0, imageDataSize);
		fread(ret->sequences[i]->frames[0]->mipmaps, imageDataSize, 1, inStream);
	}
	return ret;
fail:
	TMFree(ret->sequences[i]->frames[0]);
	TMFree(ret->sequences[i]->frames);
	TMFree(ret->sequences[i]);
	TMFree(ret->sequences);
	TMFree(ret);
	return NULL;
}
