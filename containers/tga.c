#include "containers.h"
#include "../libtex.h"
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

typedef struct TMTGAColorMapSpecification {
	unsigned short firstEntryIndex;
	unsigned short colorMapLength;
	byte colorMapEntrySize;
} TMTGAColorMapSpecification;

typedef struct TMTGAImageSpecification {
	short xOrigin; // from lower-left
	short yOrigin;
	unsigned short width;
	unsigned short height;
	byte pixelDepth;
	byte imageDescriptor;
} TMTGAImageSpecification;

typedef struct TMTGAHeader {
	byte idLength;
	byte colorMapType;
	byte imageType;
	TMTGAColorMapSpecification colorMapSpecification;
	TMTGAImageSpecification imageSpecification;
} TMTGAHeader;

typedef struct TMTGAFooter {
	long extensionAreaOffset;
	long developerDirectoryOffset;
	char signature[18];
} TMTGAFooter;

typedef struct TMTGAExtensionArea {
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
	char isNewFormat;

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
		
	ret = (*tmDefaultAllocator.malloc)(sizeof(TMTextureCollection));
	memset(ret, 0, sizeof(TMTextureCollection));
	char isCompressed, isGray;
	switch (header.imageType) {
		case 0:
			// no image data
			return ret;
		case 1:
		case 9:
			fprintf(stderr, "Color-mapped images not supported\n");
			(*tmDefaultAllocator.free)(ret);
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
			(*tmDefaultAllocator.free)(ret);
			return NULL;
	}
	seq = (*tmDefaultAllocator.malloc)(sizeof(TMSequence));
	ret->sequences = (*tmDefaultAllocator.malloc)(sizeof(void*));
	ret->sequences[0] = seq;
	ret->sequenceCount = 1;
	
	tex = (*tmDefaultAllocator.malloc)(sizeof(TMTexture));
	seq->frameCount = 0;
	seq->startFrame = 0;
	seq->frames = (*tmDefaultAllocator.malloc)(sizeof(void*));
	seq->frames[0] = tex;
	
	//tex->codec = ;
	tex->width = header.imageSpecification.width;
	tex->height = header.imageSpecification.height;
	tex->depth = 1;
	tex->xOffset = header.imageSpecification.xOrigin;
	tex->yOffset = tex->height-header.imageSpecification.yOrigin;
	tex->zOffset = 0;
	tex->mipmapCount = 1;
	
	if (subFormat == TMTGANewFormat && footer.extensionAreaOffset != 0) {
		TMTGAExtensionArea ext;
		fseek(inStream, footer.extensionAreaOffset, 0);
		read = fread(&ext, sizeof(TMTGAExtensionArea), 1, inStream);
		if (read != 1) {
			fprintf(stderr, "Could not load extension area\n");
			return NULL;
		}
		// postage stamp
		if (ext.postageStampOffset != 0) {
			// TODO
		}
		else {
			ret->thumbnail = NULL;
		}
	}
	return ret;
}
