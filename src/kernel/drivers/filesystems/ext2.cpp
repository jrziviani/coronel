#include "ext2.h"

constexpr uint32_t EXT2_INODES_PER_GROUP = 8192;

int get_block_group_by_inode(uint32_t inode)
{
    return (inode - 1) / EXT2_INODES_PER_GROUP;
}

int get_inode_index(uint32_t inode)
{
    auto index = (inode - 1) % EXT2_INODES_PER_GROUP;
    return (index * sizeof(inode)) / sizeof(uint32_t);
}

ext2::ext2()
{
}

ext2::~ext2()
{
}

void ext2::init()
{
    // Initialize the ext2 filesystem

}

void ext2::mount(const char* device)
{
    // Mount the ext2 filesystem on the specified device
}

void ext2::unmount()
{
    // Unmount the ext2 filesystem
}

void ext2::read(uint32_t block, uint8_t* buffer, uint32_t size)
{
    // Read data from the ext2 filesystem
}

void ext2::write(uint32_t block, const uint8_t* buffer, uint32_t size)
{
    // Write data to the ext2 filesystem
}

void ext2::format(const char* device)
{
    // Format the ext2 filesystem on the specified device
}

void ext2::get_file_info(const char* path, uint32_t* size, uint32_t* blocks)
{
    // Get information about a file in the ext2 filesystem
}

void ext2::get_free_space(uint32_t* free_blocks, uint32_t* free_inodes)
{
    // Get information about free space in the ext2 filesystem
}
