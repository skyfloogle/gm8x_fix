#ifndef PATCH_TYPES_H
#define PATCH_TYPES_H

#include <stdbool.h>

typedef struct {
	int pos;
	int orig_byte;
	int new_byte;
} PatchByte;

typedef enum {
	UPX,
	JOY,
	MEM,
	DPLAY,
	SCHED,
	INPUTLAG,
	RESET,
	TYPE_COUNT,
} PatchType;

typedef enum {
	UNFOUND, // default aka 0
	ABLE,
	DONE,
} PatchState;

typedef struct {
	PatchByte *bytes;
	char *name;
	PatchType type;
	PatchState state;
} Patch;

#endif
