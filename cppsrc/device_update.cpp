#include "../hppsrc/device_update.hpp"

void device_update_bitmap()
{
	unsigned char *map = (unsigned char *)malloc(bitmap_size * BLOCK_SIZE);
	unsigned char *tmp_map = map;
	memcpy(map, bitmap, (bitmap_size * BLOCK_SIZE));

	for(int i = 0; i < bitmap_size; i++) {
		device_write_block(i, map);
		map += 4096;
	}
	
	map = tmp_map;
	free(map);
}

void device_update_root()
{
	unsigned char * root_map = (unsigned char*)malloc(sizeof(root));
	memcpy(root_map, &root, sizeof(root));
	device_write_block((bitmap_size + BLOCK_SIZE), root_map);
	free(root_map);
}

void device_update_file_control_block()
{
	unsigned char * fcb_map = (unsigned char *)malloc(FILE_CONTROL_BLOCK * BLOCK_SIZE);
	unsigned char * tmp_fcb = fcb_map;
	memcpy(fcb_map, &fcb, (FILE_CONTROL_BLOCK * BLOCK_SIZE));

	for(int i = bitmap_size; i < (bitmap_size + FILE_CONTROL_BLOCK); i++){
		device_write_block(i, fcb_map);
		fcb_map += BLOCK_SIZE;
	}

	fcb_map = tmp_fcb;
	free(fcb_map);
}