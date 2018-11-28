#include "memory.h"
#include "mem.h"
#include "uarray.h"
#include "assert.h"
#include <stdio.h>

Seq_T memory;
Seq_T id_tracker;
int size;

/* Function: memory_new 
 * Allocates space for sequences in memory struct and adds 0 segment to memory
 * sequence
 * Parameters: Int 
 * Return: pointer to Memory struct
 */
void memory_new(int size)
{
        memory = Seq_new(0);
        UArray_T zero_seg = UArray_new(size, sizeof(uint32_t));
        Seq_addhi(memory,(void *)zero_seg);
        id_tracker = Seq_new(size);
        size = 1;
}

/* Function: memory_free 
 * Frees space allocated to memory sequence and uarray 
 * Parameters: pointer to Memory
 * Return: nothing
 */
void memory_free()
{
        Seq_free(&id_tracker);
        while (size > 0){
                UArray_T u_t = Seq_remlo(memory);
                if (u_t == NULL)
                        continue;
                UArray_free(&u_t);
                size--;
        }
        Seq_free(&memory);
}

/* Function: add_segment
 * Creates a new segment and adds to Memory
 * Parameters: Memory, Int 
 * Return: segment id 
 */
Umsegment_Id add_segment(int s)
{
        int id = 0;
        int length = Seq_length(id_tracker);
        UArray_T segment = UArray_new(s, sizeof(Word));
        
        if(length != 0){
                id = (int)(uintptr_t)Seq_remlo(id_tracker);
                Seq_put(memory, id, segment);
        }
        else {
                Seq_addhi(memory, segment);
                id = Seq_length(memory) - 1;
        }
        size += 1;
        return id;
}

/* Function: remove_segment
 * Removes a segment from Memory and updates id_tracker sequence
 * Parameters: Memory, Unsegment_Id  
 * Return: nothing
 */
void remove_segment(Umsegment_Id id)
{
        int null = 0;
        UArray_T segment = Seq_get(memory, id);
        UArray_free(&segment);
        Seq_put(memory, id, (void *)(uintptr_t)null);
        Seq_addhi(id_tracker, (void *)(uintptr_t)id); 
        size -= 1;
}

/* Function: get_segment
 * Gets segment of provided segment id
 * Parameters: Memory, Unsegment_Id
 * Return: void *
 */
void *get_segment(Umsegment_Id id)
{
        UArray_T segment = (UArray_T)Seq_get(memory, id);
        int length = UArray_length(segment);
        UArray_T copy = UArray_copy(segment, length);
        return (void *)copy;
}

/* Function: put_segment
 * Replaces zero segment with provided segment
 * Parameters: Memory, void *
 * Return: nothing
 */
void put_segment(void *segment)
{
        UArray_T zero_seg = Seq_get(memory, 0);
        UArray_free(&zero_seg);
        Seq_put(memory, 0, segment);
}

/* Function: get_value_at
 * Calls get_word in segments.c to access word at given segment id and offset 
 * Parameters: Memory, Unsegment_Id, int 
 * Return: Word (uint32_t)
 */

Word get_value_at(Umsegment_Id id, int offset)
{
        UArray_T segment = Seq_get(memory, id);
        /* COMBINE GET_WORD */
        Word *word = (Word *)(uintptr_t) UArray_at(segment, offset);
        return *word;
}

/* Function: set_value_at
 * Calls set_word in segments.c to access word at give segment id and offset to
 * Parameters: Memory, Unsegment_Id, int, Word
 * Return: nothing
 */
void set_value_at(Umsegment_Id id, int offset, Word value)
{
        UArray_T segment = Seq_get(memory, id);
        Word *word_p = (Word *)(uintptr_t)UArray_at(segment, offset);
        *word_p = value;
        (void) word_p;
}
