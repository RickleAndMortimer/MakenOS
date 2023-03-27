#ifndef INITRD_H
#define INITRD_H
#include "./file.h"

typedef struct {
	size_t nfiles; // The number of files in the ramdisk.
} initrd_header_t;

typedef struct {
	uint8_t magic;	    // Magic number, for error checking.
	char name[64];      // Filename.
	size_t offset;	// Offset in the initrd that the file starts.
	size_t length;	// Length of the file.
} initrd_file_header_t;

// Initialises the initial ramdisk. It gets passed the address of the multiboot module,
// and returns a completed filesystem node.
fs_node_t* initialise_initrd(size_t location);

#endif
