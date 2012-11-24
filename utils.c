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

void TMLog(TMLogLevel level, char *file, unsigned long line, char *message, ...) {
	if (level > TMLogGlobalLevel)
		return;
	switch (level) {
		case TMLogLevelError:
			fprintf(stderr, "[%s] %s:%d: ", "ERROR", file, line);
			break;
		case TMLogLevelWarning:
			fprintf(stderr, "[%s] %s:%d: ", "WARNING", file, line);
			break;
		case TMLogLevelInfo:
			fprintf(stderr, "[%s] %s:%d: ", "INFO", file, line);
			break;
		case TMLogLevelDebug:
			fprintf(stderr, "[%s] %s:%d: ", "DEBUG", file, line);
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

