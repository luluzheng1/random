#include "seq.h"
#include <stdint.h>

#ifndef MEMORY_H
#define MEMORY_H
typedef uint32_t Umsegment_Id;
typedef uint32_t Word;

void memory_new(int size);
void memory_free();
Umsegment_Id add_segment(int size);
void remove_segment(Umsegment_Id id);
void *get_segment(Umsegment_Id id);
uint32_t get_value_at(Umsegment_Id id, int offset);
void put_segment(void *segment);
void set_value_at(Umsegment_Id id, int offset, uint32_t value);

#endif