#ifndef INODE_H
#define INODE_H

#include <stdint.h>

enum class filesystem_states {
    CLEAN = 1,
    DIRTY = 2,
};

enum class error_behaviors {
    IGNORE = 1,
    REMOUNT = 2,
    PANIC = 3,
};

enum class operating_systems {
    LINUX = 0,
    HURD = 1,
    MASIX = 2,
    FREEBSD = 3,
    L4 = 4,
};

struct superblock_ext {
    uint32_t first_inode;               // First inode number
    uint16_t inode_size;                // Size of inode structure
    uint16_t superblock_id;             // superblock ID
    uint32_t optional_features;         // Compatible features
    uint32_t required_features;         // Incompatible features
    uint32_t readonly_features;         // Read-only compatible features
    uint64_t filesystem_id[2];          // Filesystem ID
    char volume_name[16];               // Volume name
    char last_mounted[64];              // Last mounted directory path
    uint32_t algorithm_usage_bitmap;    // Compression algorithm usage
    uint8_t prealloc_blocks;            // Preallocation blocks
    uint8_t prealloc_dir_blocks;        // Preallocation directory blocks
    uint16_t reserved_gdt_blocks;       // Reserved GDT blocks (unused)
    uint64_t journal_uuid[2];           // Journal UUID
    uint32_t journal_innode;            // Journal inode number
    uint32_t journal_device;            // Journal device number
    uint32_t last_orphan;               // Last orphan inode number
    uint8_t unused[788];                // Unused space
} __attribute__((packed));

struct superblock {
    uint32_t inodes_count;              // Total number of inodes
    uint32_t blocks_count;              // Total number of blocks
    uint32_t reserved_blocks_count;     // Reserved blocks count
    uint32_t free_blocks_count;         // Free blocks count
    uint32_t free_inodes_count;         // Free inodes count
    uint32_t first_data_block;          // First data block
    uint32_t log_block_size;            // log2 (Block size)
    uint32_t log_frag_size;             // log2 (Fragment size)
    uint32_t blocks_per_group;          // Blocks per group
    uint32_t frags_per_group;           // Fragments per group
    uint32_t inodes_per_group;          // Inodes per group
    uint32_t mtime;                     // Last mount time
    uint32_t wtime;                     // Last write time
    uint16_t mnt_count;                 // Mount count
    uint16_t max_mnt_count;             // Max mount count before doing fsck
    uint16_t magic;                     // Magic number (0xEF53)
    uint16_t state;                     // Filesystem state (clean, dirty)
    uint16_t errors;                    // Error behavior (ignore, remount, panic)
    uint16_t minor_rev_level;           // Minor revision level
    uint32_t lastcheck;                 // Last check time
    uint32_t checkinterval;             // Max time between checks (fsck)
    uint32_t creator_os;                // OS that created the filesystem
    uint32_t major_rev_level;           // Major revision level
    uint16_t uid;                       // User ID of the filesystem creator
    uint16_t gid;                       // Group ID of the filesystem creator
    superblock_ext extension;           // Ext2 specific flags
} __attribute__((packed));

struct block_group_descriptor {
    uint32_t block_bitmap;              // Block bitmap block number
    uint32_t inode_bitmap;              // Inode bitmap block number
    uint32_t inode_table;               // Inode table block number
    uint16_t free_blocks_count;         // Free blocks count in this group
    uint16_t free_inodes_count;         // Free inodes count in this group
    uint16_t used_dirs_count;           // Directories count in this group
    uint16_t pad;
    uint32_t reserved[3];               // Reserved for future use
};

enum class file_types {
    FIFO         = 0x1000,        // FIFO (named pipe)
    CHAR_DEVICE  = 0x2000,        // Character device
    DIRECTORY    = 0x4000,        // Directory
    REGULAR_FILE = 0x8000,        // Regular file
    SYMLINK      = 0xA000,        // Symbolic link
    BLOCK_DEVICE = 0x6000,        // Block device
    SOCKET       = 0xC000,        // Socket
};

enum class file_permissions {
    S_IXOTH = 0x0001, // Others have execute permission
    S_IWOTH = 0x0002, // Others have write permission
    S_IROTH = 0x0004, // Others have read permission
    S_IXGRP = 0x0008, // Group has execute permission
    S_IWGRP = 0x0010, // Group has write permission    
    S_IRGRP = 0x0020, // Group has read permission
    S_IXUSR = 0x0040, // Owner has execute permission
    S_IWUSR = 0x0080, // Owner has write permission
    S_IRUSR = 0x0100, // Owner has read permission
    S_ISVTX = 0x0200, // Sticky bit
    S_ISGID = 0x0400, // Set group ID on execution
    S_ISUID = 0x0800, // Set user ID on execution
};

enum class inode_flags {
    SECURE_DELETION = 0x00001, // Secure deletion (not used)
    KEEP_COPY       = 0x00002, // Keep a copy when deleted (not used)
    COMPRESSION     = 0x00004, // Compression (not used)
    SYNC_UPDATE     = 0x00008, // Synchronous update
    IMMUTABLE       = 0x00010, // Immutable file content
    APPEND_ONLY     = 0x00020, // Append-only file
    NO_DUMP         = 0x00040, // File not included in 'dump' command (What?)
    NO_ATIME        = 0x00080, // Do not update access time
    
    HASH_DIRECTORY  = 0x10000, // Hash indexed directory
    AFS_DIRECTORY   = 0x20000, // AFS directory
    JOURNAL_DATA    = 0x40000, // Journal data
};

enum class os_specific_I {
    LINUX       = 0, // Linux
    HURD        = 1, // GNU Hurd
    MASIX       = 2, // Masix
};

// This struct simply specifies how to interpret the os specific data
// in the inode. There are three operating systems: Linux, Hurd, and Masix.
// I have no intention to support Hurd or Masix, so I will only implement Linux.
struct os_specific_linux {
    uint8_t fragment_number; // Fragment number
    uint8_t fragment_size;   // Fragment size
    uint16_t reserved;       // Reserved
    uint16_t hi_uid;         // High 16 bits of 32-bit user ID
    uint16_t hi_gid;         // High 16 bits of 32-bit group ID
    uint32_t reserved2;      // Reserved
};

struct inode {                          //                (4 bits)       (botton 12 bits) 
    uint16_t mode;                      // File mode (see file_types and file_permissions)
    uint16_t uid;                       // Owner's user ID
    uint32_t size;                      // Size of the file in bytes
    uint32_t atime;                     // Last access time
    uint32_t ctime;                     // Last status change time
    uint32_t mtime;                     // Last modification time
    uint32_t dtime;                     // Deletion time
    uint16_t gid;                       // Group ID of the owner
    uint16_t links_count;               // Number of hard links to the file
    uint32_t blocks_count;              // Number of blocks allocated to the file
    uint32_t flags;                     // File flags (see inode_flags)
    uint32_t os_specific_I;             // OS-specific data (see os_specific_*)
    uint32_t block[12];                 // Pointers to data blocks
    uint32_t block_indirect;            // Pointer to the first indirect block
    uint32_t block_double_indirect;     // Pointer to the first double indirect block
    uint32_t block_triple_indirect;     // Pointer to the first triple indirect block
    uint32_t generation;                // File version (for NFS)
    uint32_t file_acl;                  // File ACL (Access Control List)
    union {
        uint32_t dir_acl;               // Directory ACL (Access Control List)
        uint32_t size_high;             // High 32 bits of file size
    };
    uint32_t faddr;                     // Fragment address
    uint8_t os_specific_II[12];         // OS-specific data (see os_specific_*)
};

enum class directory_entry_types {
    UNKNOWN  = 0, // Unknown file type
    REG_FILE = 1, // Regular file
    DIR      = 2, // Directory
    CHRDEV   = 3, // Character device
    BLKDEV   = 4, // Block device
    FIFO     = 5, // FIFO (named pipe)
    SOCK     = 6, // Socket
    SYMLINK  = 7, // Symbolic link
};

struct directory_entry {
    uint32_t inode;         // Inode number
    uint16_t rec_len;       // Length of this record
    uint8_t name_len;       // Length of the name
    uint8_t file_type;      // File type (see enum directory_entry_types)
    char name[255];         // File name (null-terminated)
};

#endif // INODE_H