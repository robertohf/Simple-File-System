#ifndef _DEVICE_UPDATE_HPP_
#define _DEVICE_UPDATE_HPP_

#include "fs.hpp"

extern struct file_control_block fcb;

void device_update_bitmap();
void device_update_root();
void device_update_file_control_block();

#endif