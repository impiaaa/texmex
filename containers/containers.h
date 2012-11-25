#ifndef TM_CONTAINER_H
#define TM_CONTAINER_H

#include "../libtex.h"
#include <stdio.h>

TMTextureCollection *tmDdsRead(FILE *inStream);
TMContainerType tmDdsContainer = {
	.name = "dds",
	.longname = "DirectDraw Surface (DDS)",
	.mime_types = "image/x-dds",
	.extensions = "dds",
	.reader = tmDdsRead
};

#endif // TM_CONTAINER_H
