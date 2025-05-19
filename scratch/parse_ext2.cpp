#include <string>
#include <iostream>

#include "ext2_inode.h"

using namespace std;

constexpr size_t PARTITION_OFFSET = 2048 * 512;
constexpr size_t SUPERBLOCK_OFFSET = 1024;
constexpr size_t ROOT_INODE_INDEX = 2;  // inode 2 => index 1 (0-based)

string read_filesystem(const string& filename) {
    FILE* file = fopen(filename.c_str(), "rb");
    fseek(file, PARTITION_OFFSET + SUPERBLOCK_OFFSET, SEEK_SET);

    superblock sb;
    size_t bytesRead = fread(&sb, 1, sizeof(sb), file);
    cout << "Inodes count: " << sb.inodes_count << endl;
    cout << "EXT2 magic number: " << hex << sb.magic << endl;

    uint32_t block_size = 1024 << sb.log_block_size;
    cout << "Block size: " << block_size << endl;

    block_group_descriptor bgd;
    fseek(file, PARTITION_OFFSET + block_size, SEEK_SET);
    bytesRead = fread(&bgd, 1, sizeof(bgd), file);
    cout << "Block bitmap: " << bgd.block_bitmap << endl;
    cout << "Inode bitmap: " << bgd.inode_bitmap << endl;
    cout << "Inode table: " << bgd.inode_table << endl;

    uint32_t inode_table_block = bgd.inode_table;
    uint32_t inode_table_offset = PARTITION_OFFSET + (inode_table_block * block_size);
    inode root_inode;
    fseek(file, inode_table_offset + ROOT_INODE_INDEX * sizeof(root_inode), SEEK_SET);
    bytesRead = fread(&root_inode, 1, sizeof(root_inode), file);

    cout << "Root inode size: " << root_inode.size << endl;
    cout << "Root inode blocks count: " << root_inode.blocks_count << endl;
    cout << "Root inode block[0]: " << root_inode.block[0] << endl;

    uint32_t data_block = root_inode.block[0];
    uint32_t data_block_offset = PARTITION_OFFSET + (data_block * block_size);
    uint8_t *data_block_buffer = new uint8_t[block_size];
    fseek(file, data_block_offset, SEEK_SET);
    bytesRead = fread(data_block_buffer, 1, block_size, file);

    cout << "Data block size: " << bytesRead << endl;
    cout << "Data block content: " << data_block_buffer << "\n";

    for (int i = 0; i < 15; i++) {
    printf("i_block[%d] = %u\n", i, root_inode.block[i]);
}
    
    int pos = 0;
    while (pos < block_size) {
        //directory_entry *entry = reinterpret_cast<directory_entry*>(data_block_buffer + pos);
        directory_entry *entry = (directory_entry*)(data_block_buffer + pos);
        if (entry->inode == 0 || entry->rec_len == 0) {
            break; // End of entries
        }

        cout << "\tInode: " << entry->inode << endl;
        cout << "\tName: "  << entry->name << endl;
        cout << "\tType: " << static_cast<int>(entry->file_type) << endl;
        pos += entry->rec_len;
    }

    delete[] data_block_buffer;
    fclose(file);
    return ""; 
}


int main() {
    string fs = read_filesystem("hda.img");

    return 0;
}


