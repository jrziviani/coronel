#ifndef EXT2_H
#define EXT2_H

#include "ext2_inode.h"
#include <libs/stdint.h>

class ext2 {
private:
    inode* root_inode;

public:
    ext2();
    ~ext2();

    void init();
    void mount(const char* device);
    void unmount();

    void read(uint32_t block, uint8_t* buffer, uint32_t size);
    void write(uint32_t block, const uint8_t* buffer, uint32_t size);
    void format(const char* device);

    void get_file_info(const char* path, uint32_t* size, uint32_t* blocks);
    void get_free_space(uint32_t* free_blocks, uint32_t* free_inodes);   
};


#endif // EXT2_H