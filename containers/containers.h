#ifndef TM_CONTAINER_H
#define TM_CONTAINER_H

typedef struct TMContainer {
    const char *name;
    const char *longname;
    const char *mime_types; // comma-separated
    const char *extensions; // comma-separated
} TMContainer;

#endif // TM_CONTAINER_H