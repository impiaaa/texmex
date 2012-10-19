#include "libtex.h"

TMAllocator tmDefaultAllocator = {
	.calloc = &calloc,
	.free = &free,
	.malloc = &malloc,
	.realloc = &realloc,
};
