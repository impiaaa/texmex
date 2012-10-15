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
