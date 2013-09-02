#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "libtex.h"
#include "containers/containers.h"

TMTextureCollection *tmTgaRead(FILE *inStream);
void tmTgaWrite(FILE *outStream, TMTextureCollection *collection);
TMTextureCollection *tmDdsRead(FILE *inStream);
char *DXT1AtoRGBA8888(unsigned char *dataIn, unsigned short width, unsigned short height);

void usage(char *execname) {
	fprintf(stderr,
			"Usage:\n"
			"  %s: [vh] [-i infile]... [-o outfile]...\n",
			execname);
}

int main (int argc, char **argv) {
	static struct option long_options[] = {
		{"verbose", optional_argument, NULL, 'v'},
		{"in", required_argument, NULL, 'i'},
		{"out", required_argument, NULL, 'o'},
		{"help", no_argument, NULL, 'h'},
		{"containers", no_argument, NULL, 'n'},
		{"compression", no_argument, NULL, 'm'},
		{"pixfmts", no_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};
	int ch;
	char *inFiles[255];
	char *outFiles[255];
	unsigned char inFileIndex = 0;
	unsigned char outFileIndex = 0;
	TMLogGlobalLevel = TMLogLevelError;
	if (argc < 2) {
		usage(argv[0]);
	}
	while ((ch = getopt_long(argc, argv, "v::i:o:h?", long_options, NULL)) != -1) {
		switch (ch) {
		case 'v':
			if (optarg == NULL)
				TMLogGlobalLevel++;
			else {
				long v = strtol(optarg, NULL, 0);
				//TODO: FIXME
				//printf("%s=%d\n", optarg, v);
				TMLogSetLevel(v);
			}
			break;
		case 'i':
			inFiles[inFileIndex] = optarg;
			inFileIndex++;
			break;
		case 'o':
			outFiles[outFileIndex] = optarg;
			outFileIndex++;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return 0;
		case 'n': {
			int i = 0;
			while (tmAllContainers[i] != NULL) {
				printf("%s\n", tmAllContainers[i]->longname);
				i++;
			}
			return 0;
			}
		default:
			break;
		}
	}
	
	int i, j, k;
	for (i = 0; i < inFileIndex; i++) {
		FILE *inFile = fopen(inFiles[i], "rb");
		if (inFile == NULL) {
			TMLogError("Error opening file: %s", inFiles[i]);
			return 1;
		}
		TMTextureCollection *tc = tmDdsRead(inFile);
		if (tc == NULL)
			continue;
		TMLogDebug("%s sequence count: %d", inFiles[i], tc->sequenceCount);
		for (j = 0; j < tc->sequenceCount; j++) {
			TMLogDebug("  Frame count: %d", tc->sequences[j]->frameCount);
			for (k = 0; k < max(1, tc->sequences[j]->frameCount); k++) {
				TMTexture *tex = tc->sequences[j]->frames[k];
				TMLogDebug("    Compression: %08X\n    Pixel format: %d\n    Width: %d\n    Height: %d\n    Depth: %d\n    mipmapCount: %d", tex->compression, tex->pixfmt, tex->width, tex->height, tex->depth, tex->mipmapCount);
			}
		}
		fclose(inFile);
		
		if (outFileIndex == 1) {
			char *dxtData = tc->sequences[0]->frames[0]->mipmaps;
			char *expandData = DXT1AtoRGBA8888(dxtData, tc->sequences[0]->frames[0]->width, tc->sequences[0]->frames[0]->height);
			free(dxtData);
			tc->sequences[0]->frames[0]->mipmaps = expandData;
			tc->sequences[0]->frames[0]->compression = TMNoCompression;
			tc->sequences[0]->frames[0]->pixfmt = BGRA8888;
			FILE *outFile = fopen(outFiles[0], "wb");
			if (outFile == NULL) {
				TMLogError("Error opening file: %s", outFiles[0]);
				return 1;
			}
			tmTgaWrite(outFile, tc);
			fclose(outFile);
		}
	}
	return 0;
}
