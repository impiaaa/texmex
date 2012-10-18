#include "containers.h"
#include "utils.h"
#include "libtex.h"
#include <stdio.h>

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
	short xOrigin;
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
	char[18] signature;
} TMTGAFooter;

typedef struct TMTGAExtensionArea {
	short extensionSize;
	char[41] authorName;
	char[324] authorComments;
	short[6] dateTimeStamp;
	char[41] jobNameId;
	short[3] jobTime;
	char[41] softwareId;
	byte[3] softwareVersion;
	long keyColor;
	short[2] pixelAspectRatio;
	short[2] gammaValue;
	long colorCorrectionOffset;
	long postageStampOffset;
	long scanLineOffset;
	byte attributesType;
}

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
	TextureCollection *ret;
	TMSequence *seq;
	TMTexture *tex;
	size_t read;
	char isNewFormat;

	read = fread(&header, sizeof(TMTGAHeader), 1, inStream);
	if (read != 1) {
		fprintf(stderr, "Could not load header\n");
		return NULL;
	}
	
	fseek(inStream, 26, SEEK_END);
	read = fread(&footer, sizeof(TMTGAFooter), 1, inStream);
	if (read != 1) {
		fprintf(stderr, "Could not load footer\n");
		return NULL;
	}
	
	if (tmTgaValidateFooter(TMTGAFooter footer)) {
		subFormat = TMTGANewFormat;
	}
	else {
		subFormat = TMTGAOldFormat;
	}
		
	ret = (*tmDefaultAllocator.malloc)(sizeof(TextureCollection));
	memset(ret, 0, sizeof(TextureCollection));
	switch (header.imageType) {
		case 0:
			// no image data
			// TODO: Try to do postage stamp anyway
			return ret;
		case ￼1:
		case 9:
			￼fprintf(stderr, "Color-mapped images not supported\n");
			return NULL;
		case ￼2:
			￼
			break;
		default:
			￼fprintf(stderr, "Unknown image type, %d\n", header.imageType);
			return NULL;
			break;
	}
	ret->sequences = malloc(sizeof(void*));
	ret->sequences[0] = seq;
	ret->sequenceCount = 1;

	// postage stamp
	if (subFormat == TMTGANewFormat && footer.postageStampOffset != 0) {
		// TODO
	}
	else {
		ret->thumbnail = NULL;
	}
}
