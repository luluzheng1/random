#include "segments.h"
#include "assert.h"
#include <stdio.h>
typedef uint32_t Word;

/* Function: get_word
 * Gets value at provided segment and offset
 * Parameters: int, UArray_T 
 * Return: pointer to a 32-bit word(uint32_t)
 */
uint32_t *get_word(int offset, UArray_T segment){
	int length = UArray_length(segment);
	Word *word = (Word *)(uintptr_t) UArray_at(segment, offset);
        return word;
}

/* Function: set_word
 * Sets value at provided segment and offset to word
 * Parameters: int, UArray_T, Word 
 * Return: nothing
 */
void set_word(int offset, UArray_T segment, Word word){
	Word *word_p = (Word *)(uintptr_t)UArray_at(segment, offset);
	*word_p = word;
	(void) word_p;
}
