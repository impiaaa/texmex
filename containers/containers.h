#ifndef TM_CONTAINER_H
#define TM_CONTAINER_H

#include "../libtex.h"
#include <stdio.h>

TMTextureCollection *tmDdsRead(FILE *inStream);
extern TMContainerType tmDdsContainer;
extern TMContainerType tmTgaContainer;
extern TMContainerType *tmAllContainers[];

#endif // TM_CONTAINER_H
