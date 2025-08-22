#ifndef DECODE_H
#define DECODE_H

#include "sim.h"

/* Decoding functions for ARMv8 instructions.
 * They take the instruction code as input and
 * store the decoded data in a data_t structure
 * according to the instruction format.
 */
uint32_t shift_bits(uint32_t instruction_code, int shift_amount, uint32_t mask);
void decode_r(data_t *instruction_data, uint32_t instruction_code);
void decode_i(data_t *instruction_data, uint32_t instruction_code);
void decode_d(data_t *instruction_data, uint32_t instruction_code);
void decode_b(data_t *instruction_data, uint32_t instruction_code);
void decode_cb(data_t *instruction_data, uint32_t instruction_code);
void decode_iw(data_t *instruction_data, uint32_t instruction_code);

#endif
