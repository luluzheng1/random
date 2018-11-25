#include "seq.h"
#include <stdint.h>
#include "uarray.h"
#ifndef MEMORY_H
#define MEMORY_H
typedef uint32_t Umsegment_Id;
typedef uint32_t Word;

typedef struct Memory *Memory;

Memory memory_new(int size);
void memory_free(Memory *m);
Umsegment_Id add_segment(Memory memory, int size);
void remove_segment(Memory memory, Umsegment_Id id);
void *get_segment(Memory memory, Umsegment_Id id);
uint32_t get_value_at(Memory memory, Umsegment_Id id, int offset);
void put_segment(Memory m, void *segment);
void set_value_at(Memory memory, Umsegment_Id id, int offset, uint32_t value);
//void set_counter(UArray_T segment, int offset, Word *counter);

#endif