#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

char GlobalVerbosity = 0;

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
					fprintf(stderr, "Verbosity level %d out of range (0..127)\n", v);
					exit(1);
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
			fprintf(stderr, "Usage:\n"
					"\t%s: [vh] [-i infile]... [-o outfile]...\n",
					argv[0]);
			exit(0);
			break;
		default:
			break;
		}
	}
}
