#include "containers.h"
#include "utils.h"

TMContainer tm_tga_container = {
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

char tmTgaValidateFooter(TMTGAFooter footer) {
	return strncmp(signature, "TRUEVISION-XFILE.", 18) == 0;
}
