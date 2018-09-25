#ifndef _FS_HPP_
#define _FS_HPP_

#include "device.hpp"
#include "bitmap.hpp"
#include "device_update.hpp"

#include <ctype.h>
#include <cstddef>
#include <cmath>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <iostream> 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <experimental/filesystem>


#define MAX_NAME_LEN		        (int)256
#define MAX_BLOCKS_PER_FILE	    (int)(BLOCK_SIZE/sizeof(int) * (BLOCK_SIZE/sizeof(int)))
#define MAX_FILE_SIZE           (int)(MAX_BLOCKS_PER_FILE * BLOCK_SIZE)
#define MAX_DIR_ENTRIES         (int)(20480 * 1024)/(int)sizeof(struct dir_entry)
#define FILE_CONTROL_BLOCK      (int)(20480 * 1024)/BLOCK_SIZE
#define MAX_INDEX_BLOCKS        (int)MAX_BLOCKS_PER_FILE/sizeof(int)

#define DEVICE_SIZE             (int)1*(1024*1024*1024)
#define BITMAP_SIZE             (int)DEVICE_SIZE/BLOCK_SIZE/8/BLOCK_SIZE

struct dir_entry
{
  char name[MAX_NAME_LEN];
  char type;
  int size;
  long creation_date;
  long modified_date;
  int index_block;
};

struct file_control_block 
{
  struct dir_entry entries[MAX_DIR_ENTRIES];
};

struct dir
{
  struct dir_entry entries[MAX_DIR_ENTRIES/BLOCK_SIZE];
};

struct index_block
{
  int iblocks[BLOCK_SIZE/4];
};

void fs_update_bitmap();
void fs_update_root();
void fs_update_file_control_block();

int fs_getattr(const char *path, struct stat *statbuf);
int fs_readlink(const char *path, char *link, size_t size);
int fs_mknod(const char *path, mode_t mode, dev_t dev);
int fs_mkdir(const char *path, mode_t mode);
int fs_unlink(const char *path);
int fs_rmdir(const char *path);
int fs_symlink(const char *path, const char *link);
int fs_rename(const char *path, const char *newpath);
int fs_link(const char *path, const char *newpath);
int fs_chmod(const char *path, mode_t mode);
int fs_chown(const char *path, uid_t uid, gid_t gid);
int fs_truncate(const char *path, off_t newSize);
int fs_utime(const char *path, struct utimbuf *ubuf);
int fs_open(const char *path, struct fuse_file_info *fileInfo);
int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
int fs_statfs(const char *path, struct statvfs *statInfo);
int fs_flush(const char *path, struct fuse_file_info *fileInfo);
int fs_release(const char *path, struct fuse_file_info *fileInfo);
int fs_fsync(const char *path, int datasync, struct fuse_file_info *fi);
int fs_opendir(const char *path, struct fuse_file_info *fileInfo);
int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);
int fs_releasedir(const char *path, struct fuse_file_info *fileInfo);
int fs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo);
void *fs_init(struct fuse_conn_info *conn);

#endif