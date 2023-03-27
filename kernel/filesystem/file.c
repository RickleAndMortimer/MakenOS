#include "file.h"

fs_node_t* fs_root = NULL; // The root of the filesystem.

size_t read_fs(fs_node_t *node, size_t offset, uint32_t size, uint8_t* buffer)
{
	// Has the node got a read callback?
	if (node->read != 0)
		return node->read(node, offset, size, buffer);
	else
		return 0;
}

size_t write_fs(fs_node_t *node, size_t offset, size_t size, uint8_t* buffer)
{
	// Has the node got a read callback?
	if (node->write != 0)
		return node->write(node, offset, size, buffer);
	else
		return 0;
}

void open_fs(fs_node_t *node, uint8_t read, uint8_t write)
{
	// Has the node got a read callback?
	if (node->open != 0)
		node->open(node);
}

void close_fs(fs_node_t *node)
{
	// Has the node got a read callback?
	if (node->close != 0)
		node->close(node);
}

struct dirent *readdir_fs(fs_node_t *node, size_t index) 
{
	if (node->readdir != 0 && (node->flags & 0x7) == FS_DIRECTORY)
		return node->readdir(node, index);
	else
		return NULL;
}

fs_node_t* finddir_fs(fs_node_t *node, char *name)
{
	if (node->finddir != 0 && (node->flags & 0x7) == FS_DIRECTORY)
		return node->finddir(node, name);
	else
		return NULL;
}

