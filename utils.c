#include "libtex.h"
#include <stdarg.h>
#include <stdio.h>

TMLogLevel TMLogGlobalLevel;

void TMLogSetLevel(TMLogLevel i) {
	if ((i < 0) || (i > TMLogLevelAll)) {
		TMLogError("Invalid log level %d", i);
	}
	else {
		TMLogGlobalLevel = i;
	}
}

void TMLog(TMLogLevel level, char *message, ...) {
	if (level > TMLogGlobalLevel)
		return;
	switch (level) {
		case TMLogLevelError:
			fprintf(stderr, "%s", "[ERROR] ");
			break;
		case TMLogLevelWarning:
			fprintf(stderr, "%s", "[WARNING] ");
			break;
		case TMLogLevelInfo:
			fprintf(stderr, "%s", "[INFO] ");
			break;
		case TMLogLevelDebug:
			fprintf(stderr, "%s", "[DEBUG] ");
			break;
		default:
			return;
	}
	va_list argptr;
	va_start(argptr, message);
	vfprintf(stderr, message, argptr);
	va_end(argptr);
	fprintf(stderr, "\n");
}

