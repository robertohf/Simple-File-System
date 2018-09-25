#include "../hppsrc/bitmap.hpp"

#include <math.h>

void set(int n, int value)
{
  if(value)
    bitmap[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
  else
    bitmap[WORD_OFFSET(n)] &=~ (1 << BIT_OFFSET(n));
}

int get()
{
  int i, j;
  for(i = 0; bitmap[i] == 0 && i < device_size/BLOCK_SIZE/8; i++);
  for(j = 0; !CHECK_BIT(bitmap[i], j) && j < BITS_PER_WORD; j++);

  if(!CHECK_BIT(bitmap[i], j) || (BITS_PER_WORD * i + j) >= (device_size/BLOCK_SIZE))
    return -1;

  return (BITS_PER_WORD * i + j);
}

int count()
{  
  int count=0;
  for(int x=0; x<BLOCK_SIZE; x++) {
    for(int y=0; y<BITS_PER_WORD; y++) {
      if((BITS_PER_WORD*x+y)>=floor(device_size/BLOCK_SIZE)) {
        x=BLOCK_SIZE;
        break;
      }
      if(CHECK_BIT(bitmap[x], y))
        count++;
    }
  }
  return count;
}