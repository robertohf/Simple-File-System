#include "../hppsrc/fs.hpp"

namespace fs = std::experimental::filesystem;

unsigned int *bitmap;
int device_size, bitmap_size;
struct dir root, dir;
struct file_control_block fcb;

int check_in_root(const char *path)
{
	int in_root = 1;
	for(int i = 1; i < (MAX_NAME_LEN + 1) && path[i] != 0; i++) {
		if(path[i] == '/') {
			in_root = false;
			break;
		}
	}
	return in_root;
}

std::string get_dir_name(const char *path) {
  return &fs::path(path).parent_path().generic_string()[1];
}

std::string get_file_name(const char *path) {
  return fs::path(path).filename().generic_string();
}

struct dir *fs_load_dir(int index_block)
{
	struct dir *directory = (struct dir *)malloc(sizeof(struct dir));
	unsigned char *dir_map = (unsigned char *)malloc(sizeof(*directory));
	device_read_block(index_block, dir_map);
	memcpy(directory, dir_map, sizeof(*directory));
	free(dir_map);

	return directory;
}

struct dir_entry* fs_lookup_root_dir(const char *name)
{
	struct dir_entry *entry;
	for(int i = 0; i < MAX_DIR_ENTRIES; i++) {
		entry = &(fcb.entries[i]);
		if(strcmp(entry->name, name) == 0)
			return entry;
	}
	return NULL;
}

void fs_get_file_stat(struct dir_entry* dentry, int *size, int *block_count)
{
	struct index_block *iblock = (struct index_block *)malloc(sizeof(struct index_block));
	unsigned char *index_map = (unsigned char*)malloc(sizeof(*iblock));
	device_read_block(dentry->index_block, index_map);
	memcpy(iblock, index_map, sizeof(*iblock));
	free(index_map);

	int i;
	for(i = 0; i < (BLOCK_SIZE / 4); i++) {
		if(iblock->iblocks[i] != 0)
		(*block_count)++;
	}

	for(i = 0; i <= *block_count; i++)
		*size += 4096;

	free(iblock);
}	

int fs_getattr(const char *path, struct stat *statbuf)
{
	printf("%s\n", path);
	int path_len = strlen(path);

	if((path_len == 1) && path[0] == '/') {
		statbuf->st_mode =  S_IFDIR;
		statbuf->st_uid = 0;
		statbuf->st_gid = 0;
		statbuf->st_nlink = 1;
		statbuf->st_ino = 0;
		statbuf->st_size = BLOCK_SIZE;
		statbuf->st_blksize = BLOCK_SIZE;
		statbuf->st_blocks = 1;
	}
	else {
		struct dir_entry *entry = fs_lookup_root_dir(&path[1]);

		if(entry == NULL)
			return -ENOENT;

		if(entry->type) {
			statbuf->st_mode = S_IFDIR | 0777;
			statbuf->st_uid = 0;
			statbuf->st_gid = 0;
			statbuf->st_nlink = 1;
			statbuf->st_ino = 0;
			statbuf->st_size = BLOCK_SIZE;
			statbuf->st_blksize = BLOCK_SIZE;
			statbuf->st_blocks = 1;
		}
		else {
			int size, block_count;
			fs_get_file_stat(entry, &size, &block_count);

			statbuf->st_mode = S_IFREG | 0777;
			statbuf->st_nlink = 1;
			statbuf->st_ino = 0;
			statbuf->st_uid = 0;
			statbuf->st_gid = 0;
			statbuf->st_size = size; 
			statbuf->st_blksize = BLOCK_SIZE;
			statbuf->st_blocks = block_count;
		}
	}

	return 0;
}

int fs_readlink(const char *path, char *link, size_t size)
{
	printf("readlink\n");
	return 0;
}

int fs_mknod(const char *path, mode_t mode, dev_t dev)
{
	printf("mknod(path=%s, mode=%d)\n", path, mode);

	if(S_ISREG(mode)) {
		struct dir_entry *entry = fs_lookup_root_dir(&path[1]);

		if(entry != NULL) 
			return -EEXIST;
		
		int block, i;
		struct dir *directory_entry;
		switch(check_in_root(path)) {
			case 0:
			{
				entry = fs_lookup_root_dir(get_dir_name(path).c_str());

				if(entry == NULL){
					printf("Entry Null.\n");
					return -ENOENT;
				}

				block = entry->index_block;
				printf("block no: %d\n", block);
				directory_entry = fs_load_dir(block);
				printf("name: %s\n", directory_entry->entries[0].name);
			}
			break;
			case 1:
			{
				directory_entry = &root;
				block = 5128;
				i = 0;
			}
			break;
		}

		int j;
		for(j = 0; j < MAX_DIR_ENTRIES/BLOCK_SIZE; j++) {
			if(directory_entry->entries[j].index_block == 0)
				break;
		}

		int empty_block = get();
		
		strcpy(directory_entry->entries[j].name, &path[1]);
		directory_entry->entries[j].type = 0;
		directory_entry->entries[j].size = 0;
		directory_entry->entries[j].index_block = empty_block;
		set(directory_entry->entries[j].index_block, 0);
		device_update_bitmap();

		for(i = 0 ; i < MAX_DIR_ENTRIES; i++) {
			if(strcmp(fcb.entries[i].name, "\0") == 0) {
				fcb.entries[i] = directory_entry->entries[j];
				device_update_file_control_block();
				break;
			}
		}

		struct index_block iblock;
		for(i = 0; i < BLOCK_SIZE/(int)sizeof(int); i++)
			iblock.iblocks[i] = 0;

		switch(check_in_root(path)) {
			case 0:
			{
				unsigned char *dir_map = (unsigned char *)malloc(sizeof(*directory_entry));
				memcpy(dir_map, directory_entry, sizeof(*directory_entry));
				device_write_block(block, dir_map);
				free(dir_map);
			}
			break;
			case 1:
			{
				device_update_root();
			}
			break;
		}

		unsigned char *index_map = (unsigned char *)malloc(sizeof(iblock));
		memcpy(index_map, &iblock, sizeof(iblock));
		device_write_block(directory_entry->entries[j].index_block, index_map);
		free(index_map);

		return 0;
	}

	return -EPERM;
}

int fs_mkdir(const char *path, mode_t mode)
{
	printf("mkdir\n");
	
	struct dir_entry *entry = fs_lookup_root_dir(&path[1]);

	int block, i;
	struct dir_entry *directory_entry;

	switch(check_in_root(path)) {
		case 0:
		{
			entry = fs_lookup_root_dir(get_dir_name(path).c_str());
			if(entry == NULL)
				return -ENOENT;
			block = entry->index_block;
			directory_entry = fs_load_dir(block)->entries;
		}
		break;
		case 1:
		{
			directory_entry = root.entries;
			i = 0;
			block = 5128;
		}
		break;
	}

	int j;
	for(j = 0; j < MAX_DIR_ENTRIES; j++) {
		if(directory_entry[j].index_block == 0)
			break;
	}

	int empty_block = get();
	
	strcpy(directory_entry[j].name, &path[1]);
	directory_entry[j].type = 1;
	directory_entry[j].size = 0;
	directory_entry[j].index_block = empty_block;
	set(directory_entry[j].index_block, 0);
	device_update_bitmap();

	for(i = 0 ; i < MAX_DIR_ENTRIES; i++) {
		if(strcmp(fcb.entries[i].name, "\0") == 0) {
			fcb.entries[i] = directory_entry[j];
			device_update_file_control_block();
			break;
		}
	}

	struct index_block iblock;
	for(i = 0; i < BLOCK_SIZE/(int)sizeof(int); i++)
		iblock.iblocks[i] = 0;

	switch(check_in_root(path)) {
		case 0:
		{
			unsigned char *dir_map = (unsigned char *)malloc(sizeof(*directory_entry));
			memcpy(dir_map, directory_entry, sizeof(*directory_entry));
			device_write_block(block, dir_map);
			free(dir_map);
		}
		break;
		case 1:
		{
			device_update_root();
		}
		break;
	}

	unsigned char *index_map = (unsigned char *)malloc(sizeof(iblock));
	memcpy(index_map, &iblock, sizeof(iblock));
	device_write_block(directory_entry[j].index_block, index_map);
	free(index_map);
	
	return 0;
}

int fs_unlink(const char *path)
{
	printf("unlink\n");
	return 0;
}

int fs_rmdir(const char *path)
{
	printf("rmdir\n");
	return 0;
}

int fs_symlink(const char *path, const char *link)
{
	printf("symlink\n");
	return 0;
}

int fs_rename(const char *path, const char *newpath)
{
	printf("rename\n");
	switch(check_in_root(path)) {
		case 1:
		{
			strcpy(fs_lookup_root_dir(&path[1])->name, &newpath[1]);
			device_update_root();
		return 0;
		}
		break;
	}
	return -ENOENT;
}

int fs_link(const char *path, const char *newpath)
{
	printf("link\n");
	return 0;
}

int fs_chmod(const char *path, mode_t mode)
{
	printf("chmod\n");
	return 0;
}

int fs_chown(const char *path, uid_t uid, gid_t gid)
{
	printf("chown\n");
	return 0;
}

int fs_truncate(const char *path, off_t newSize)
{
	printf("truncate\n");
	return 0;
}

int fs_utime(const char *path, struct utimbuf *ubuf)
{
	printf("utime\n");
	return 0;
}

int fs_open(const char *path, struct fuse_file_info *fileInfo)
{
	printf("open\n");
	return 0;
}

int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
	printf("read\n");

	struct dir_entry *entry = fs_lookup_root_dir(&path[1]);

	if(entry == NULL)
		return -ENOENT;

	unsigned char *data = (unsigned char *)calloc(1, int(size));
	struct index_block *iblock = (struct index_block *)malloc(sizeof(struct index_block));
	
	unsigned char *index_map = (unsigned char *)malloc(sizeof(*iblock));
	device_read_block(entry->index_block, index_map);
	memcpy(iblock, index_map, sizeof(*iblock));
	free(index_map);

  int my_offset = 0;
	for(int i = 0; i < 1; i++) {
		if(iblock->iblocks[i] != 0) {
			unsigned char *block_data = (unsigned char *)malloc(BLOCK_SIZE);
			device_read_block(iblock->iblocks[i], block_data);
			memcpy(&data[my_offset], block_data, BLOCK_SIZE);
			my_offset += BLOCK_SIZE;
			free(block_data);
		}
	}

	memcpy(buf, data, size);

	free(iblock);
	free(data);

	return size; 
}

int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
	printf("write\n");

	struct dir_entry *entry = fs_lookup_root_dir(&path[1]);

	if(entry == NULL)
		return -ENOENT;

	struct index_block *iblock = (struct index_block *)malloc(sizeof(struct index_block));
	unsigned char *index_map = (unsigned char *)malloc(sizeof(*iblock));
	device_read_block(entry->index_block, index_map);
	memcpy(iblock, index_map, sizeof(*iblock));
	free(index_map);

	for(int i = 0; i < 1; i++) {
		int empty_block = get();
		set(empty_block, 0);
		device_update_bitmap();
		iblock->iblocks[i + int(floor(((double)offset)/BLOCK_SIZE))] = empty_block;

		unsigned char *index_map = (unsigned char *)malloc(BLOCK_SIZE);
		memcpy(index_map, iblock, BLOCK_SIZE);
		device_write_block(entry->index_block, index_map);
		free(index_map);

		unsigned char *block_data = (unsigned char *)malloc(BLOCK_SIZE);
		memset(block_data, '\0', BLOCK_SIZE);
		memcpy(block_data, buf, size);
		device_write_block(iblock->iblocks[i + int(floor(((double)offset)/BLOCK_SIZE))], block_data);
		free(block_data);
	}
	return size;
}

int fs_statfs(const char *path, struct statvfs *statInfo)
{
	printf("statfs");
	return 0;
}

int fs_flush(const char *path, struct fuse_file_info *fileInfo)
{
	printf("flush");
	return 0;
}

int fs_release(const char *path, struct fuse_file_info *fileInfo)
{
	printf("release");
	return 0;
}

int fs_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
	printf("fsync");
	return 0;
}

int fs_opendir(const char *path, struct fuse_file_info *fileInfo)
{
	printf("opendir");
	return 0;
}

int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo)
{
	printf("readdir(path=%s, offset=%d)\n", path, (int)offset);

	struct dir_entry *entry;
	if(strcmp(path, "/") == 0)
		dir = root;
	else {
		entry = fs_lookup_root_dir(&path[1]);

		if(entry == NULL) 
			return -ENOENT;

		dir = *(fs_load_dir(entry->index_block));
	}

	for(int i = 0; i < MAX_DIR_ENTRIES; i++) {
		if(dir.entries[i].index_block != 0) {
			if(filler(buf, get_file_name(dir.entries[i].name).c_str(), NULL, 0) != 0) {
				return -ENOMEM;
			}
		}
	}
	return 0;
}

int fs_releasedir(const char *path, struct fuse_file_info *fileInfo)
{
	printf("releasedir");
	return 0;
}

int fs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo)
{
	printf("fsyncdir");
	return 0;
}

void *fs_init(struct fuse_conn_info *conn)
{
	printf("....Initializing....\n");

	unsigned char * root_map = (unsigned char*)malloc(sizeof(root));
	bitmap = (unsigned int *)malloc(bitmap_size * BLOCK_SIZE);
	unsigned char *map = (unsigned char *)malloc(bitmap_size * BLOCK_SIZE);
	unsigned char *tmp_map = map;
	unsigned char *fcb_map = (unsigned char *)malloc(FILE_CONTROL_BLOCK * BLOCK_SIZE);
	unsigned char *tmp_fcb = fcb_map;

	device_read_block((bitmap_size + FILE_CONTROL_BLOCK), root_map);
	for(int i = 0; i < bitmap_size; i++) {
		device_read_block(i, map);
		map += 4096;
	}
	for(int i = bitmap_size; i < (bitmap_size + FILE_CONTROL_BLOCK); i++){
		device_read_block(i, fcb_map);
		fcb_map += 4096;
	}

	memcpy(&root, root_map, sizeof(root));
	map = tmp_map;
	memcpy(bitmap, map, (bitmap_size * BLOCK_SIZE));
	fcb_map = tmp_fcb;
	memcpy(&fcb, fcb_map, (FILE_CONTROL_BLOCK * BLOCK_SIZE));
	free(map);
	free(root_map);
	free(fcb_map);

	return NULL;
}

