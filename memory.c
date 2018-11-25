#include "memory.h"
#include "mem.h"
#include "uarray.h"
#include "segments.h"
#include "assert.h"
#include <stdio.h>
typedef struct Memory {
        Seq_T memory;
        Seq_T id_tracker;
        int size;
} *Memory;
/* Function: memory_new 
 * Allocates space for sequences in memory struct and adds 0 segment to memory
 * sequence
 * Parameters: Int 
 * Return: pointer to Memory struct
 */
Memory memory_new(int size)
{
	Memory UM_memory;
	NEW(UM_memory);
	UM_memory->memory = Seq_new(0);
        UArray_T zero_seg = UArray_new(size, sizeof(uint32_t));
        Seq_addhi(UM_memory->memory,(void *)zero_seg);
	UM_memory->id_tracker = Seq_new(size);
	UM_memory->size = 1;
	return UM_memory;
}

/* Function: memory_free 
 * Frees space allocated to memory sequence and uarray 
 * Parameters: pointer to Memory
 * Return: nothing
 */
void memory_free(Memory *m)
{
	Seq_free(&((*m)->id_tracker));
	int length = (*m)->size;
	while (length > 0){
		UArray_T u_t = Seq_remlo((*m)->memory);
                if (u_t == NULL)
                        continue;
		UArray_free(&u_t);
		length--;
	}
	Seq_free(&(*m)->memory);
}

/* Function: add_segment
 * Creates a new segment and adds to Memory
 * Parameters: Memory, Int 
 * Return: segment id 
 */
Umsegment_Id add_segment(Memory memory, int size)
{
	int id = 0;
	int length = Seq_length(memory->id_tracker);
	UArray_T segment = UArray_new(size, sizeof(Word));
        
	if(length != 0){
		id = (int)(uintptr_t)Seq_remlo(memory->id_tracker);
		Seq_put(memory->memory, id, segment);
	}
	else {
		Seq_addhi(memory->memory, segment);
		id = Seq_length(memory->memory) - 1;
	}
        memory->size += 1;
        return id;
}

/* Function: remove_segment
 * Removes a segment from Memory and updates id_tracker sequence
 * Parameters: Memory, Unsegment_Id  
 * Return: nothing
 */
void remove_segment(Memory memory, Umsegment_Id id)
{
	int null = 0;
	UArray_T segment = Seq_get(memory->memory, id);
	UArray_free(&segment);
	Seq_put(memory->memory, id, (void *)(uintptr_t)null);
	Seq_addhi(memory->id_tracker, (void *)(uintptr_t)id); 
        memory->size -= 1;
}

/* Function: get_segment
 * Gets segment of provided segment id
 * Parameters: Memory, Unsegment_Id
 * Return: void *
 */
void *get_segment(Memory memory, Umsegment_Id id)
{
	UArray_T segment = (UArray_T)Seq_get(memory->memory, id);
	int length = UArray_length(segment);
	UArray_T copy = UArray_copy(segment, length);
	return (void *)copy;
}

/* Function: put_segment
 * Replaces zero segment with provided segment
 * Parameters: Memory, void *
 * Return: nothing
 */
void put_segment(Memory m, void *segment)
{
        UArray_T zero_seg = Seq_get(m->memory, 0);
        UArray_free(&zero_seg);
	Seq_put(m->memory, 0, segment);
}

/* Function: get_value_at
 * Calls get_word in segments.c to access word at given segment id and offset 
 * Parameters: Memory, Unsegment_Id, int 
 * Return: Word (uint32_t)
 */
Word get_value_at(Memory memory, Umsegment_Id id, int offset)
{
	int length = Seq_length(memory->memory);
	UArray_T segment = Seq_get(memory->memory, id);
	Word *word = get_word(offset, segment);
        return *word;
}

/* Function: set_value_at
 * Calls set_word in segments.c to access word at give segment id and offset to
 * Parameters: Memory, Unsegment_Id, int, Word
 * Return: nothing
 */
void set_value_at(Memory memory, Umsegment_Id id, int offset, Word value)
{
	int length = Seq_length(memory->memory);
        UArray_T segment = Seq_get(memory->memory, id);
	set_word(offset, segment, value);
}
