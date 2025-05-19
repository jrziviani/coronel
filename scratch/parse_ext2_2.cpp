#include "ext2_inode.h"
#include <cstdio>
#include <iostream>

using namespace std;

using file_t = FILE*;
constexpr size_t PARTITION_OFFSET = 2048 * 512;
constexpr size_t SUPERBLOCK_OFFSET = 1024;
constexpr size_t ROOT_INODE_INDEX = 1;  // inode 2 => index 1 (0-based)

file_t open_image()
{
    file_t file = fopen("hda.img", "rb");
    if (!file) {
        perror("Failed to open file");
        return nullptr;
    }
    return file;
}

template<typename T>
T read_from_file(file_t file, size_t offset)
{
    T data;

    fseek(file, offset, SEEK_SET);
    size_t bytesRead = fread(&data, 1, sizeof(data), file);
    if (bytesRead != sizeof(data)) {
        perror("Failed to read from file");
    }

    return data;
}

int main()
{
    static_assert(sizeof(superblock) == 1024, "Superblock size is not 1024 bytes");
    static_assert(sizeof(block_group_descriptor) == 32, "Block group descriptor size is not 32 bytes");
    static_assert(sizeof(inode) == 128, "Inode size is not 128 bytes");

    auto file = open_image();
    if (!file) {
        return 1;
    }

    auto sb = read_from_file<superblock>(file, PARTITION_OFFSET + SUPERBLOCK_OFFSET);
    cout << "EXT2 magic number: " << hex << sb.magic << endl;

    auto block_size = 1024 << sb.log_block_size;
    auto bgd = read_from_file<block_group_descriptor>(file, PARTITION_OFFSET + block_size);
    cout << "Block bitmap: " << bgd.block_bitmap << endl;

    uint32_t inode_table_block = bgd.inode_table;
    uint32_t inode_table_offset = PARTITION_OFFSET + (inode_table_block * block_size);
    auto root_inode = read_from_file<inode>(file, inode_table_offset + ROOT_INODE_INDEX * sizeof(inode));
    cout << "Root inode size: " << root_inode.size << endl;
    cout << "Root inode blocks count: " << root_inode.blocks_count << endl;
    cout << "Root inode block[0]: " << root_inode.block[0] << endl;

    uint32_t data_block = root_inode.block[0];
    uint32_t data_block_offset = PARTITION_OFFSET + (data_block * block_size);
    uint8_t *data_block_buffer = new uint8_t[block_size];
    auto data = read_from_file<uint8_t*>(file, data_block_offset);
    cout << "Data block content: " << data << "\n";


    return 0;
}