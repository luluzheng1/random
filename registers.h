#include "uarray.h"
#include "mem.h"
#include "assert.h"
#include <stdint.h>

#ifndef REGISTERS_H
#define REGISTERS_H
typedef enum Um_register {r0 = 0, r1, r2, r3, r4, r5, r6, r7} Um_register;
typedef struct Register *Register;

Register initiate_registers(int size);
void register_free(Register *r);
uint32_t get_register_at(Register r, Um_register n);
void set_register_at(Register r, Um_register n, uint32_t value);
#endif