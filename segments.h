#include "uarray.h"
#include <stdint.h>
#ifndef SEGMENTS_H
#define SEGMENTS_H
uint32_t *get_word(int offset, UArray_T segment); 
/* returns word and updates program counter */
void set_word(int offset, UArray_T segment, uint32_t word);
#endif