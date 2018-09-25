#ifndef _DEVICE_HPP_
#define _DEVICE_HPP_

#define BLOCK_SIZE  4096

extern int bitmap_size;
extern struct dir root;

int device_open(const char *path, int device_size);
int device_read_block(int block_no, unsigned char buffer[]);
int device_write_block(int blocn_no, unsigned char buffer[]);  
void device_new_device(const char *path, int device_size);
void device_close();
void device_flush();

#endif