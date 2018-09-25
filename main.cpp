#define FUSE_USE_VERSION 26

#include "hppsrc/fs.hpp"

#include <fuse.h>

struct fuse_operations fs_oper;

int main(int argc, char **argv)
{
  int i;
  
  device_size = 1*(1024*1024*1024);
  bitmap_size = device_size/BLOCK_SIZE/8/BLOCK_SIZE;

  fs_oper.init = fs_init;
  fs_oper.getattr = fs_getattr;
  fs_oper.readdir = fs_readdir;
  fs_oper.mknod = fs_mknod;
  fs_oper.mkdir = fs_mkdir;
  fs_oper.utime = fs_utime;
  fs_oper.open = fs_open;
  fs_oper.unlink = fs_unlink;
  fs_oper.write = fs_write;
  fs_oper.read = fs_read;
  fs_oper.rename = fs_rename;
  
  for(i = 1; i < argc && (argv[i][0] == '-'); i++) {
    if(i == argc) {
      return (-1);
    }
  }

  if(!device_open(argv[1], device_size)) {
    printf("Cannot open device file %s\n", argv[i]);
    return 1;
  }

  for(; i < argc; i++) {
    argv[i] = argv[i+1];
  }
  argc--;

  int fuse_stat = fuse_main(argc, argv, &fs_oper, NULL);

  device_close();

  return fuse_stat;
}