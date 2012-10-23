#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "libtex.h"

char GlobalVerbosity = 0;

TMTextureCollection *tmTgaRead(FILE *inStream);
void tmTgaWrite(FILE *outStream, TMTextureCollection *collection);
TMTextureCollection *tmDdsRead(FILE *inStream);

void usage(char *execname) {
	fprintf(stderr, "Usage:\n"
			"\t%s: [vh] [-i infile]... [-o outfile]...\n",
			execname);
}

int main (int argc, char **argv) {
	static struct option long_options[] = {
		{"verbose", optional_argument, NULL, 'v'},
		{"in", required_argument, NULL, 'i'},
		{"out", required_argument, NULL, 'o'},
		{"help", required_argument, NULL, 'h'},
		{NULL, 0, NULL, 0}
	};
	int ch;
	char *inFiles[255];
	char *outFiles[255];
	unsigned char inFileIndex = 0;
	unsigned char outFileIndex = 0;
	if (argc < 2) {
		usage(argv[0]);
	}
	while ((ch = getopt_long(argc, argv, "v::i:o:h?", long_options, NULL)) != -1) {
		switch (ch) {
		case 'v':
			if (optarg == NULL)
				GlobalVerbosity++;
			else {
				long v = strtol(optarg, NULL, 0);
				//TODO: FIXME
				//printf("%s=%d\n", optarg, v);
				if (v < 0 || v > 127) {
					fprintf(stderr, "Verbosity level %ld out of range (0..127)\n", v);
					return 1;
				}
				GlobalVerbosity = (char)v;
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
		default:
			break;
		}
	}
	
	int i, j, k;
	for (i = 0; i < inFileIndex; i++) {
		FILE *inFile = fopen(inFiles[i], "rb");
		if (inFile == NULL) {
			fprintf(stderr, "Error opening file: %s\n", inFiles[i]);
			return 1;
		}
		TMTextureCollection *tc = tmDdsRead(inFile);
		if (tc == NULL)
			continue;
		printf("%s sequence count: %d\n", inFiles[i], tc->sequenceCount);
		for (j = 0; j < tc->sequenceCount; j++) {
			printf("  Frame count: %d\n", tc->sequences[j]->frameCount);
			for (k = 0; k < max(1, tc->sequences[j]->frameCount); k++) {
				TMTexture *tex = tc->sequences[j]->frames[k];
				printf("    Compression: %08X\n    Pixel format: %d\n    Width: %d\n    Height: %d\n    Depth: %d\n    mipmapCount: %d\n", tex->compression, tex->pixfmt, tex->width, tex->height, tex->depth, tex->mipmapCount);
			}
		}
		fclose(inFile);
		
		if (outFileIndex == 1) {
			FILE *outFile = fopen(outFiles[0], "wb");
			if (outFile == NULL) {
				fprintf(stderr, "Error opening file: %s\n", outFiles[0]);
				return 1;
			}
			tmTgaWrite(outFile, tc);
			fclose(outFile);
		}
	}
	return 0;
}
