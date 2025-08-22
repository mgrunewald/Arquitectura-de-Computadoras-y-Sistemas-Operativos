#ifndef EXECUTE_H
#define EXECUTE_H

#include "sim.h"

/* Execution functions for ARMv8 instructions.
 * They take the decoded data in a data_t structure
 * as input and update the next CPU state accordingly.
 */
void execute_b(data_t *instruction_data);
void execute_ands_sr(data_t *instruction_data);
void execute_eor_sr(data_t *instruction_data);
void execute_orr_sr(data_t *instruction_data);
void execute_add_imm(data_t *instruction_data);
void execute_adds_imm(data_t *instruction_data);
void execute_subs_cmp_imm(data_t *instruction_data);
void execute_b_cond(data_t *instruction_data);
void execute_cbz(data_t *instruction_data);
void execute_cbnz(data_t *instruction_data);
void execute_lsl_lsr_imm(data_t *instruction_data);
void execute_adds_er(data_t *instruction_data);
void execute_subs_cmp_er(data_t *instruction_data);
void execute_br(data_t *instruction_data);
void execute_add_er(data_t *instruction_data);
void execute_mul(data_t *instruction_data);
void execute_stur(data_t *instruction_data);
void execute_sturb(data_t *instruction_data);
void execute_sturh(data_t *instruction_data);
void execute_ldur(data_t *instruction_data);
void execute_ldurb(data_t *instruction_data);
void execute_ldurh(data_t *instruction_data);
void execute_hlt(data_t *instruction_data);
void execute_movz(data_t *instruction_data);

#endif
