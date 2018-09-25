#ifndef _BITMAP_HPP_
#define _BITMAP_HPP_

#include "fs.hpp"

#define BITS_PER_WORD (int)32
#define WORD_OFFSET(n) (int)((n)/BITS_PER_WORD)
#define BIT_OFFSET(n) (int)((n) % BITS_PER_WORD)
#define CHECK_BIT(word, n) (int)(((word) >> (n)) & 1)

extern unsigned int *bitmap;
extern int device_size;

void set(int n, int value);
int get();
int count();

#endif