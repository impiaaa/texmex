#include "containers.h"
#include "../compression/compression.h"
#include "../libtex.h"
#include "../pixfmts.h"
#include <stdio.h>
#include <string.h>

TMContainerType tmTgaContainer = {
	.name = "tga",
	.longname = "Truevision TGA (TARGA)",
	.mime_types = "image/x-targa,image/x-tga",
	.extensions = "tga,tpic,vda,icb,vst"
};

#define byte unsigned char

// http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf

typedef struct __attribute__((packed)) TMTGAColorMapSpecification {
	unsigned short firstEntryIndex;
	unsigned short colorMapLength;
	byte colorMapEntrySize;
} TMTGAColorMapSpecification;

typedef struct __attribute__((packed)) TMTGAImageSpecification {
	short xOrigin; // from lower-left
	short yOrigin;
	unsigned short width;
	unsigned short height;
	byte pixelDepth;
	byte imageDescriptor;
} TMTGAImageSpecification;

typedef struct __attribute__((packed)) TMTGAHeader {
	byte idLength;
	byte colorMapType;
	byte imageType;
	TMTGAColorMapSpecification colorMapSpecification;
	TMTGAImageSpecification imageSpecification;
} TMTGAHeader;

typedef struct __attribute__((packed)) TMTGAFooter {
	int extensionAreaOffset;
	int developerDirectoryOffset;
	char signature[18];
} TMTGAFooter;

typedef struct __attribute__((packed)) TMTGAExtensionArea {
	short extensionSize;
	char authorName[41];
	char authorComments[324];
	short dateTimeStamp[6];
	char jobNameId[41];
	short jobTime[3];
	char softwareId[41];
	byte softwareVersion[3];
	long keyColor;
	short pixelAspectRatio[2];
	short gammaValue[2];
	long colorCorrectionOffset;
	long postageStampOffset;
	long scanLineOffset;
	byte attributesType;
} TMTGAExtensionArea;

typedef enum TMTGASubFormat {
	TMTGAOldFormat,
	TMTGANewFormat
} TMTGASubFormat;

char tmTgaValidateFooter(TMTGAFooter footer) {
	return strncmp(footer.signature, "TRUEVISION-XFILE.", 18) == 0;
}

TMTextureCollection *tmTgaRead(FILE *inStream) {
	TMTGAHeader header;
	TMTGAFooter footer;
	TMTGASubFormat subFormat;
	TMTextureCollection *ret;
	TMSequence *seq;
	TMTexture *tex;
	size_t read;

	read = fread(&header, sizeof(TMTGAHeader), 1, inStream);
	if (read != 1) {
		fprintf(stderr, "Could not load header\n");
		return NULL;
	}
	
	fseek(inStream, -sizeof(TMTGAFooter), SEEK_END);
	read = fread(&footer, sizeof(TMTGAFooter), 1, inStream);
	if (read != 1) {
		fprintf(stderr, "Could not load footer\n");
		return NULL;
	}
	
	if (tmTgaValidateFooter(footer)) {
		subFormat = TMTGANewFormat;
	}
	else {
		subFormat = TMTGAOldFormat;
	}
		
	ret = TMMalloc(sizeof(TMTextureCollection));
	memset(ret, 0, sizeof(TMTextureCollection));
	char isGray, isCompressed;
	switch (header.imageType) {
		case 0:
			// no image data
			return ret;
		case 1:
		case 9:
			fprintf(stderr, "Color-mapped images not supported\n");
			TMFree(ret);
			return NULL;
		case 2:
			isCompressed = 0;
			isGray = 0;
			break;
		case 3:
			isCompressed = 0;
			isGray = 1;
			break;
		case 10:
			isCompressed = 1;
			isGray = 0;
			break;
		case 11:
			isCompressed = 1;
			isGray = 1;
			break;
		default:
			fprintf(stderr, "Unknown image type, %d\n", header.imageType);
			TMFree(ret);
			return NULL;
	}
	seq = TMMalloc(sizeof(TMSequence));
	ret->sequences = TMMalloc(sizeof(void*));
	ret->sequences[0] = seq;
	ret->sequenceCount = 1;
	
	seq->frameCount = 0;
	seq->startFrame = 0;
	seq->frames = TMMalloc(sizeof(void*));
	tex = TMMalloc(sizeof(TMTexture));
	seq->frames[0] = tex;
	
	tex->compression = isCompressed ? RLE : TMNoCompression;
	if (isGray) {
		switch (header.imageSpecification.pixelDepth) {
			case 4:
				tex->pixfmt = I4;
				break;
			case 8:
				tex->pixfmt = I8;
				break;
			default:
				fprintf(stderr, "Grayscale not supported with pixel depth %d\n", header.imageSpecification.pixelDepth);
				TMFree(ret);
				TMFree(seq);
				TMFree(tex);
				return NULL;
		}
	}
	else {
		switch (header.imageSpecification.pixelDepth) {
			case 16:
				tex->pixfmt = BGR565;
				break;
			case 24:
				tex->pixfmt = BGR888;
				break;
			case 32:
				tex->pixfmt = BGRA8888;
				break;
			default:
				fprintf(stderr, "pixel depth %d not supported\n", header.imageSpecification.pixelDepth);
				TMFree(ret);
				TMFree(seq);
				TMFree(tex);
				return NULL;
		}
	}
	tex->width = header.imageSpecification.width;
	tex->height = header.imageSpecification.height;
	tex->depth = 1;
	tex->xOffset = header.imageSpecification.xOrigin;
	tex->yOffset = tex->height-header.imageSpecification.yOrigin;
	tex->zOffset = 0;
	tex->mipmapCount = 1;
	unsigned long imageDataSize = (header.imageSpecification.width*header.imageSpecification.height*header.imageSpecification.pixelDepth)>>3;
	tex->mipmaps = TMMalloc(imageDataSize);
	fseek(inStream, sizeof(TMTGAHeader)+header.idLength+((header.colorMapSpecification.colorMapLength*header.colorMapSpecification.colorMapEntrySize)>>3), SEEK_SET);
	read = fread(tex->mipmaps, imageDataSize, 1, inStream);
	
	if (subFormat == TMTGANewFormat && footer.extensionAreaOffset != 0) {
		TMTGAExtensionArea ext;
		fseek(inStream, footer.extensionAreaOffset, SEEK_SET);
		read = fread(&ext, sizeof(TMTGAExtensionArea), 1, inStream);
		if (read == 1) {
			// postage stamp
			if (ext.postageStampOffset != 0) {
				// TODO
			}
			else {
				ret->thumbnail = NULL;
			}
		}
		else {
			ret->thumbnail = NULL;
			fprintf(stderr, "Could not load extension area\n");
		}
	}
	return ret;
}

void tmTgaWrite(FILE *outStream, TMTextureCollection *collection) {
	TMTGAHeader header;
	TMTGAFooter footer;
	size_t wrote;
	
	header.idLength = 0;
	header.colorMapType = 0;
	if (collection->sequences[0]->frames[0]->pixfmt == I8 || collection->sequences[0]->frames[0]->pixfmt == I4) {
		header.imageType = 3;
	}
	else if (collection->sequences[0]->frames[0]->pixfmt == BGR565 || collection->sequences[0]->frames[0]->pixfmt == BGR888 || collection->sequences[0]->frames[0]->pixfmt == BGRA8888) {
		header.imageType = 2;
	}
	else {
		fprintf(stderr, "Unsupported pixfmt %d\n", collection->sequences[0]->frames[0]->pixfmt);
		return;
	}
	
	header.colorMapSpecification.firstEntryIndex = 0;
	header.colorMapSpecification.colorMapLength = 0;
	header.colorMapSpecification.colorMapEntrySize = 0;
	
	header.imageSpecification.xOrigin = collection->sequences[0]->frames[0]->xOffset;
	header.imageSpecification.yOrigin = collection->sequences[0]->frames[0]->height-collection->sequences[0]->frames[0]->yOffset;
	header.imageSpecification.width = collection->sequences[0]->frames[0]->width;
	header.imageSpecification.height = collection->sequences[0]->frames[0]->height;
	switch (collection->sequences[0]->frames[0]->pixfmt) {
		case I4:
			header.imageSpecification.pixelDepth = 4;
			break;
		case I8:
			header.imageSpecification.pixelDepth = 8;
			break;
		case BGR565:
			header.imageSpecification.pixelDepth = 16;
			break;
		case BGR888:
			header.imageSpecification.pixelDepth = 24;
			break;
		case BGRA8888:
			header.imageSpecification.pixelDepth = 32;
			break;
		default:
			fprintf(stderr, "Insane condition\n");
			return;
	}
	header.imageSpecification.imageDescriptor = 0;
	
	wrote = fwrite(&header, sizeof(TMTGAHeader), 1, outStream);
	if (wrote != 1) {
		fprintf(stderr, "Could not write header\n");
		return;
	}
	
	unsigned long imageDataSize = (collection->sequences[0]->frames[0]->width*collection->sequences[0]->frames[0]->height*header.imageSpecification.pixelDepth)>>3;
	wrote = fwrite(collection->sequences[0]->frames[0]->mipmaps, imageDataSize, 1, outStream);
	if (wrote != 1) {
		fprintf(stderr, "Could not write image data\n");
		return;
	}
}
