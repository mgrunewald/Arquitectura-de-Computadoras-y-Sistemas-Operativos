#ifndef SIM_H
#define SIM_H

#include <stddef.h>
#include <stdint.h>

extern int BRANCH_FLAG;

/* Generic structure that stores the decoded
 * data of any ARMv8 core instruction format.
 */
typedef struct {
    uint32_t op:2;
    uint32_t shift:2;
    uint32_t Rm:5;
    uint32_t Rn:5;
    uint32_t Rd:5;
    uint32_t Rt:5;
    uint32_t shamt:6;
    uint32_t DT_address:9;
    uint32_t ALU_immediate:12;
    uint32_t MOV_immediate:16;
    uint32_t COND_BR_address:19;
    uint32_t BR_address:26;
} data_t;

/* Structure that models an ARMv8 instruction.
 * It contains its opcode, opcode length and
 * the functions that can decode and execute it.
 */
typedef struct {
    size_t opcode_length;
    uint32_t opcode;
    void (*decode)(data_t *, uint32_t);
    void (*execute)(data_t *);
} instruction_t;

/* Decodes and executes the ARMv8 instruction pointed by
 * the program counter and updates it to point to the
 * instruction to be processed in the next state.
 */
void process_instruction();

#endif
