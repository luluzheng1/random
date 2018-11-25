#include "registers.h"

typedef struct Register {
        uint32_t regs[8];
} *Register;

/* Function: initiate_registers
 * Allocates space for register Uarray
 * Parameters: int 
 * Return: Register struct
 */
Register initiate_registers(int size) 
{
	Register r;
	NEW(r);
	return r;
}

/* Function: register_free
 * Deallocates register UArray in Register struct
 * Parameters: pointer to Register struct 
 * Return: nothing
 */
void register_free(Register *r)
{
	//UArray_free(&(*r)->regs);
}
	
/* Function: get_register_at
 * Gets value at provided register
 * Checks if a valid register number is provided
 * Parameters: Reister, Um_register 
 * Return: uint32_t 
 */
uint32_t get_register_at(Register r, Um_register n) 
{
	assert(n >= 0);
	assert(n <= 7);
	uint32_t val = r->regs[n];
	return val;
}

/* Function: set_register_at
 * Set register n to value
 * Checks if a valid register number is provided
 * Parameters: Reister, Um_register, uint32_t
 * Return: nothing
 */
void set_register_at(Register r, Um_register n, uint32_t value)
{
	assert(n >= 0);
	assert(n <= 7);
	r->regs[n] = value;
}
