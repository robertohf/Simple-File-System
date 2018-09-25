#include "../hppsrc/device.hpp"
#include "../hppsrc/fs.hpp"

#include <stdio.h>

static const char *device_path;
static FILE *f;

int device_open(const char *path, int device_size)
{
  device_path = path;

  f = fopen(device_path, "r+");

  if(f == NULL)
    device_new_device(path, device_size);

  return ( f != NULL );
}

int device_read_block(int block_no, unsigned char buffer[])
{
  fseek(f, block_no*BLOCK_SIZE, SEEK_SET);

  return ( fread(buffer, 1, BLOCK_SIZE, f) == BLOCK_SIZE );
}

int device_write_block(int block_no, unsigned char buffer[])
{
  fseek(f, block_no*BLOCK_SIZE, SEEK_SET);

  return ( fwrite(buffer, 1, BLOCK_SIZE, f) == BLOCK_SIZE );
}

void device_new_device(const char *path, int device_size) 
{
  f = fopen(path, "w+");

  unsigned int empty_blocks[(BLOCK_SIZE * bitmap_size / 4)];
  int tmp_bitmap_size = bitmap_size;

  for(int i = 0; i < 160; i++) 
    empty_blocks[i] = 0x0;

  for(int i = 160; i < (BLOCK_SIZE * bitmap_size / 4); i++)
    empty_blocks[i] = 0xFFFFFFFF;
  int i = 160;

  if(tmp_bitmap_size != 0){
		empty_blocks[i] = 0xFFFFFFFF;
		empty_blocks[i] = empty_blocks[i] << (tmp_bitmap_size + 1);
		i++;
	}

  unsigned char *map = (unsigned char *)malloc(BLOCK_SIZE*bitmap_size);
  unsigned char *tmp_map = map;
  memcpy(map, empty_blocks, (BLOCK_SIZE * bitmap_size));

  for(int i = 0; i < (BLOCK_SIZE * bitmap_size / 4); i++){
    device_write_block(i, map);
    map += 4096;
  }

  map = tmp_map;
  free(map);
  
  for(int i = 0; i < MAX_DIR_ENTRIES; i++) {
    root.entries[i].index_block = 0;
  }
  
  unsigned char *root_map = (unsigned char*)malloc(sizeof(root));
  //memcpy(root_map, &device_size, sizeof(device_size));
  memcpy(root_map, &root, sizeof(root));
  device_write_block((bitmap_size + FILE_CONTROL_BLOCK), root_map);
  free(root_map);
}

void device_close()
{
  fflush(f);
  fclose(f);
}

void device_flush()
{   
  fflush(f);
}