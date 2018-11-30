#include "mem.h"
#include "seq.h"
#include "assert.h"
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define BYTESIZE 8
#define MEGABYTE 1000000000
#define REG_WIDTH 3
#define OP_WIDTH 4
#define LSB_OP 28
#define LSB_A 6
#define LSB_B 3
#define LSB_C 0

//uint32_t read_next_instruction(Memory m, Register r, uint32_t *counter);
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;
typedef uint32_t Umsegment_Id, Word, Um_instruction;
typedef enum Um_register {r0 = 0, r1, r2, r3, r4, r5, r6, r7} Um_register;

typedef struct Segment {
	uint32_t *arr;
	int size;
} *Segment;

uint32_t r[8];
Um_instruction r_c, r_a, r_b;
uint32_t val_b, val_c, val_a;
Segment *memory;
Seq_T id_tracker;
int size, mem_length = 1;


/*instructions.c*/
static inline uint64_t Bitpack_getu(uint64_t word, unsigned width, 
				    unsigned lsb);
static inline Um_instruction read_instruction(Um_instruction word, 
					      uint32_t *counter);
static inline void cmov(Um_instruction word);
static inline void sload(Um_instruction word, uint32_t *counter);
static inline void sstore(Um_instruction word);
static inline void add(Um_instruction word);
static inline void mult(Um_instruction word);
static inline void divide(Um_instruction word);
static inline void nand(Um_instruction word);
static inline void map(Um_instruction word);
static inline void unmap(Um_instruction word);
static inline void out(Um_instruction word);
static inline void in(Um_instruction word);
static inline void loadp(Um_instruction word, uint32_t *counter);
static inline void lv(Um_instruction word);
/*memory.c*/
static inline void memory_new(int size);
static inline void memory_free();
static inline Umsegment_Id add_segment(int size);
static inline void remove_segment(Umsegment_Id id);
static inline void *get_segment(Umsegment_Id id);
static inline uint32_t get_value_at(Umsegment_Id id, int offset);
static inline void put_segment(void *Segment);
static inline void set_value_at(Umsegment_Id id, int offset, uint32_t value);

int main(int argc, char *argv[])
{
        if (argc != 2){
                fprintf(stderr, "Invalid usage: %s program_name.um\n", argv[0]);
                exit(EXIT_FAILURE);
        }
        FILE *um_file = fopen(argv[1], "r");
        
        struct stat s;
        stat(argv[1], &s);
        int file_size = s.st_size;
        assert(file_size < MEGABYTE);

        memory_new(file_size / 4);
        /* Fill 0 segment with all instructions */
        int num_inst = 0;
        uint32_t instruction = 0;
        for (int i = 0; i < file_size; i++){
                int c = getc(um_file);
                instruction = instruction << BYTESIZE;
                instruction += c;
                /* After reading in 4 chars, add the instruction to 0 segment */
                if ((i % 4 == 3)){
                        set_value_at(0, num_inst, instruction);
                        instruction = 0;
                        num_inst++;
                }
        }
        fclose(um_file);
        int op_code = 0;
	uint32_t counter = 0;
	instruction = get_value_at(0, counter);
	
        while (op_code != HALT){
                uint32_t instruction = get_value_at(0, counter);
                op_code = read_instruction(instruction, &counter);
		if(op_code != LOADP){
			counter++;
		}
        }
        memory_free();
}

static inline void cmov(Um_instruction word)
{
	
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	uint32_t val_c = r[r_c]; 
	if(val_c != 0)
	{
		uint32_t val_b = r[r_b];
		uint32_t val_a = r[r_a];
		r[r_a] = val_b;
	}
}

static inline void sload(Um_instruction word, uint32_t *counter)
{
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	
	val_b = r[r_b];
	val_c = r[r_c];
	val_a = get_value_at(val_b, val_c);
	
	r[r_a] = val_a;
}

static inline void sstore(Um_instruction word)
{
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
        
        val_a = r[r_a];
        val_b = r[r_b];
        val_c = r[r_c];

        set_value_at(val_a, val_b, val_c);        
}

static inline void add(Um_instruction word)
{
        uint32_t sum;
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        val_b = r[r_b];
        val_c = r[r_c];

        sum = val_b + val_c;           

	r[r_a] = sum;
}

static inline void mult(Um_instruction word)
{
        uint32_t product;
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
        
        val_b = r[r_b];
        val_c = r[r_c];

        product = val_b * val_c;            

	r[r_a] = product;
}

static inline void divide(Um_instruction word)
{
        uint32_t quotient;
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
        
        val_b = r[r_b];
        val_c = r[r_c];

        quotient = val_b / val_c;          

	r[r_a] = quotient;
}

static inline void nand(Um_instruction word)
{
        uint32_t nand;
        
        r_a = Bitpack_getu(word, REG_WIDTH, LSB_A);
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        val_b = r[r_b];
        val_c = r[r_c];

        nand = ~(val_b & val_c);

	r[r_a] = nand;
}

static inline void map(Um_instruction word)
{
        uint32_t id, length;
        
        r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        length = r[r_c];
        
        id = add_segment(length);
	r[r_b] = id;
}

static inline void unmap(Um_instruction word)
{
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);

        val_c = r[r_c];
        remove_segment(val_c);
}

static inline void out(Um_instruction word)
{
	r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	val_c = r[r_c];
	fprintf(stdout, "%c", val_c);
}

static inline void in(Um_instruction word)
{
	uint32_t value = fgetc(stdin);
        r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	if((int32_t)value != EOF) {
		r[r_c] = value;
	}
	else {
		uint32_t marker = ~0;
		r[r_c] = marker;
	}
}

static inline void loadp(Um_instruction word, uint32_t *counter)
{
	r_b = Bitpack_getu(word, REG_WIDTH, LSB_B);
	r_c = Bitpack_getu(word, REG_WIDTH, LSB_C);
	val_b = r[r_b];
	val_c = r[r_c];
	if(val_b != 0) {
		Segment array = memory[val_b];
		Segment temp = memory[0];
		free(temp->arr);
		free(temp);
		memory[0] = array;
	}
	get_value_at(0, val_c);
	*counter = val_c;
}

static inline void lv(Um_instruction word)
{
	uint32_t value;
	r_a = Bitpack_getu(word, REG_WIDTH, 25);
	value = Bitpack_getu(word, 25, 0);
	r[r_a] = value;
}

static inline uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        unsigned bitsl = 64 - hi, bitsr = 64 - width;
        /* left shift */
        if (bitsl != 64)
                word = word << bitsl;
        /* right shift */
        if (bitsr != 64)
                word = word >> bitsr;
        /* ----- */
        return word;
}

static inline Um_instruction read_instruction(Um_instruction word, 
					      uint32_t *counter)
{
	Um_instruction op = Bitpack_getu(word, OP_WIDTH, LSB_OP);
        
	switch(op) {
		case CMOV :
                        cmov(word);
			break;
		case SLOAD :
                        sload(word, counter);
			break;
		case SSTORE :
                        sstore(word);
			break;
		case ADD :
                        add(word);
			break;
		case MUL :
                        mult(word);
			break;
		case DIV :
                        divide(word);
			break;
		case NAND :
                        nand(word);
			break;
		case HALT :
			break;
		case ACTIVATE :
                        map(word);
			break;
		case INACTIVATE :
                        unmap(word);
			break;
		case OUT :
                        out(word);
			break;
		case IN :
                        in(word);
			break;
		case LOADP :
                        loadp(word, counter);
			break;
		case LV :
                        lv(word);
			break;
		default :
			break;
	}
	
	return op;
}



/* Function: memory_new 
 * Allocates space for sequences in memory struct and adds 0 segment to memory
 * sequence
 * Parameters: Int 
 * Return: pointer to Memory struct
 */
static inline void memory_new(int size)
{
        memory = calloc(1, sizeof(Segment));
	Segment zero_seg = malloc(sizeof(struct Segment));
	zero_seg->size = size;
        zero_seg->arr = calloc(zero_seg->size, sizeof(uint32_t));
        memory[0] = zero_seg;
        id_tracker = Seq_new(size);
        size = 1;
}

/* Function: memory_free 
 * Frees space allocated to memory sequence and array 
 * Parameters: pointer to Memory
 * Return: nothing
 */
static inline void memory_free()
{
        Seq_free(&id_tracker);
        while (mem_length > 1){
		Segment array = memory[mem_length -1];
                if (array != NULL) {
			free(array->arr);
                        free(array);
		}
		mem_length--;
        }
        free(memory);
}

/* Function: add_segment
 * Creates a new segment and adds to Memory
 * Parameters: Memory, Int 
 * Return: segment id 
 */
static inline Umsegment_Id add_segment(int s)
{
        int id = 0;
        int length = Seq_length(id_tracker);
	Segment array = malloc(sizeof(struct Segment));
	array->size = s;
	array->arr = calloc(s, sizeof(Word));
        
        if(length != 0){
                id = (int)(uintptr_t)Seq_remlo(id_tracker);
                memory[id] = array;
		
        }
        else {
		Segment *temp = realloc(memory, sizeof(Segment) * (mem_length + 1));
                if(temp != NULL) {
			memory = temp;
			memory[mem_length++] = array;
			id = mem_length - 1;
  		}
        }
        size += 1;
        return id;
}

/* Function: remove_segment
 * Removes a segment from Memory and updates id_tracker sequence
 * Parameters: Memory, Unsegment_Id  memory[0] = zero_seg
 * Return: nothing
 */
static inline void remove_segment(Umsegment_Id id)
{
	Segment array = memory[id];
	free(array->arr);
	free(array);
	size -= 1;
        if(id < (uint32_t)mem_length) {
		memory[id] = NULL;
	}
	else {
		mem_length -= 1;
	}
	Seq_addhi(id_tracker, (void *)(uintptr_t)id); 
}

/* Function: get_segment
 * Gets segment of provided segment id
 * Parameters: Memory, Unsegment_Id
 * Return: void *
 */
static inline void *get_segment(Umsegment_Id id)
{
        Segment array = memory[id];
        int length = array->size;
        Segment cpy = malloc(sizeof(struct Segment));
	memcpy(cpy->arr, array->arr, length * sizeof(uint32_t));
        return (void *)cpy;
}

/* Function: put_segment
 * Replaces zero segment with provided segment
 * Parameters: Memory, void *
 * Return: nothing
 */
static inline void put_segment(void *array)
{
        Segment zero_seg = memory[0];
	free(zero_seg->arr);
	free(zero_seg);
        memory[0] = array;
}

/* Function: get_value_at
 * Calls get_word in segments.c to access word at given segment id and offset 
 * Parameters: Memory, Unsegment_Id, int 
 * Return: Word (uint32_t)
 */
static inline Word get_value_at(Umsegment_Id id, int offset)
{
        Segment array = memory[id];
        /* COMBINE GET_WORD */
        Word word = array->arr[offset];
        return word;
}

/* Function: set_value_at
 * Calls set_word in segments.c to access word at give segment id and offset to
 * Parameters: Memory, Unsegment_Id, int, Word
 * Return: nothing
 */
static inline void set_value_at(Umsegment_Id id, int offset, Word value)
{
        Segment array = memory[id];
        array->arr[offset] = value;
}
