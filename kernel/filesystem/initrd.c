#include "initrd.h"
#include "file.h"
#include "../lib/string.h"
#include "../memory/pmm.h"
#include "../limine.h"
#include "../kernel.h"

struct dirent* root_dirents;
fs_node_t* root_nodes;

initrd_header_t* initrd_header;	  
initrd_file_header_t* file_headers;
size_t nroot_nodes;

static size_t initrd_read(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer)
{
	initrd_file_header_t header = file_headers[node->inode - 2];
	if (offset > header.length)
		return 0;
	if (offset + size > header.length)
		size = header.length - offset;
	memcpy(buffer, (uint8_t*) (header.offset + offset), size);
	return size;
}

static struct dirent* initrd_readdir(fs_node_t* node, size_t index)
{
    if (index >= nroot_nodes) {
        return NULL;
    }

	return root_dirents + index;
}

static fs_node_t* initrd_finddir(fs_node_t* node, char* name)
{
	for (size_t i = 0; i < nroot_nodes; i++) {
	    if (!strcmp(name, root_dirents[i].name))
		    return root_nodes + i;
    }
	return 0;
}

fs_node_t* initialise_initrd(size_t location)
{
	// Initialise the main and file header pointers and populate the root directory.
	initrd_header = (initrd_header_t*) location;
	file_headers = (initrd_file_header_t*) (location + sizeof(initrd_header_t));

    // Initialize root nodes and root directory entries
	root_nodes = (fs_node_t*) k_malloc(sizeof(fs_node_t) * (initrd_header->nfiles + 2));
    root_dirents = (struct dirent*) k_malloc(sizeof(struct dirent) * (initrd_header->nfiles + 2));

    // Add root directory entry
	strcpy(root_dirents[0].name, "/");
    root_dirents[0].ino = 0;

	// Initialise the root directory.
	root_nodes[0].mask = 0;
    root_nodes[0].uid = 0; 
    root_nodes[0].gid = 0;
    root_nodes[0].inode = 0;
    root_nodes[0].length = 0;
	root_nodes[0].flags = FS_DIRECTORY;
	root_nodes[0].read = NULL;
	root_nodes[0].write = NULL;
	root_nodes[0].open = NULL;
	root_nodes[0].close = NULL;
	root_nodes[0].readdir = &initrd_readdir;
	root_nodes[0].finddir = &initrd_finddir;
	root_nodes[0].ptr = 0;
	root_nodes[0].impl = 0;

    // Add dev directory entry
	strcpy(root_dirents[1].name, "dev");
    root_dirents[1].ino = 1;

	// Initialise the /dev directory
	root_nodes[1].mask = 0;
    root_nodes[1].uid = 0;
    root_nodes[1].gid = 0;
    root_nodes[1].inode = 1;
    root_nodes[1].length = 0;
	root_nodes[1].flags = FS_DIRECTORY;
	root_nodes[1].read = NULL;
	root_nodes[1].write = NULL;
	root_nodes[1].open = NULL;
	root_nodes[1].close = NULL;
	root_nodes[1].readdir = &initrd_readdir;
	root_nodes[1].finddir = &initrd_finddir;
	root_nodes[1].ptr = NULL;
	root_nodes[1].impl = 0;
	nroot_nodes = initrd_header->nfiles + 2;
	// For every file...
	for (size_t i = 2; i < nroot_nodes; i++)
	{
	    // Edit the file's header - currently it holds the file offset
	    // relative to the start of the ramdisk. We want it relative to the start
	    // of memory.
	    file_headers[i - 2].offset += location;
        // Add file to the directory entries
	    strcpy(root_dirents[i].name, file_headers[i - 2].name);
        root_dirents[i].ino = i;

	    // Create a new file node.
	    root_nodes[i].inode = i;
	    root_nodes[i].mask = 0;
        root_nodes[i].uid = 0;
        root_nodes[i].gid = 0;
	    root_nodes[i].length = file_headers[i - 2].length;
	    root_nodes[i].inode = i;
	    root_nodes[i].flags = FS_FILE;
	    root_nodes[i].read = &initrd_read;
	    root_nodes[i].write = NULL;
	    root_nodes[i].readdir = NULL;
	    root_nodes[i].finddir = NULL;
	    root_nodes[i].open = NULL;
	    root_nodes[i].close = NULL;
	    root_nodes[i].impl = 0;
	}
    return root_nodes;
}
