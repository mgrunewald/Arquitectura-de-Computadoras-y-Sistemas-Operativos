#include "shell.h"
#include "execute.h"
#include <stdint.h>
# include <stdio.h> 
#include <inttypes.h> // Para los especificadores de formato de 64 bits

#define XZR 0x1f

/* Utility function that sign extends a
 * <bits>-bit unsigned integer to 64-bit.
 */
uint64_t sign_extend(uint64_t n, size_t bits) {
    uint64_t m = 1U << (bits - 1);
    return (n ^ m) - m;
}

/* Executes the B instruction.
*/
void execute_b(data_t *instruction_data) {
    uint64_t offset = sign_extend(instruction_data->BR_address << 2, 28);
    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    BRANCH_FLAG = TRUE;
}

/* Executes the ANDS (shifted register) instruction.
 */
void execute_ands_sr(data_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t operand2 = CURRENT_STATE.REGS[instruction_data->Rm];
    uint64_t result = operand1 & operand2;
    NEXT_STATE.FLAG_N = result >> 63;
    NEXT_STATE.FLAG_Z = result == 0;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}

/* Executes the EOR (shifted register) instruction.
 */
void execute_eor_sr(data_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t operand2 = CURRENT_STATE.REGS[instruction_data->Rm];
    uint64_t result = operand1 ^ operand2;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}

/* Executes the ORR (shifted register) instruction.
 */
void execute_orr_sr(data_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t operand2 = CURRENT_STATE.REGS[instruction_data->Rm];
    uint64_t result = operand1 | operand2;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}

/* Executes the ADD (immediate) instruction.
 */
void execute_add_imm(data_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t imm = instruction_data->ALU_immediate;
    if (instruction_data->shift == 0x1) {
        imm <<= 12;
    }
    uint64_t result = operand1 + imm;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}

/* Executes the ADDS (immediate) instruction.
 */
void execute_adds_imm(data_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t imm = instruction_data->ALU_immediate;
    if (instruction_data->shift == 0x1) {
        imm <<= 12;
    }
    uint64_t result = operand1 + imm;
    NEXT_STATE.FLAG_N = result >> 63;
    NEXT_STATE.FLAG_Z = result == 0;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}

/* Executes the SUBS (immediate) and CMP (immediate) instructions.
 */
void execute_subs_cmp_imm(data_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t imm = instruction_data->ALU_immediate;
    if (instruction_data->shift == 0x1) {
        imm <<= 12;
    }
    uint64_t result = operand1 + ~imm + 1;
    NEXT_STATE.FLAG_N = result >> 63;
    NEXT_STATE.FLAG_Z = result == 0;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}

/* Executes the B.cond instruction that implements the standard
 * conditions BEQ, BNE, BLT, BGT and BLE assuming C=0 and V=0.
 */
void execute_b_cond(data_t *instruction_data) {
    uint64_t offset = sign_extend(instruction_data->COND_BR_address << 2, 21);
    switch (instruction_data->Rt) {
        case 0x0: // BEQ
            if (CURRENT_STATE.FLAG_Z) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                BRANCH_FLAG = TRUE;
            }
            break;
        case 0x1: // BNE
            if (!CURRENT_STATE.FLAG_Z) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                BRANCH_FLAG = TRUE;
            }
            break;
        case 0xb: // BLT
            if (CURRENT_STATE.FLAG_N) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                BRANCH_FLAG = TRUE;
            }
            break;
        case 0xc: // BGT
            if (!CURRENT_STATE.FLAG_N && !CURRENT_STATE.FLAG_Z) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                BRANCH_FLAG = TRUE;
            }
            break;
        case 0xd: // BLE
            if (CURRENT_STATE.FLAG_N || CURRENT_STATE.FLAG_Z) {
                NEXT_STATE.PC = CURRENT_STATE.PC + offset;
                BRANCH_FLAG = TRUE;
            }
            break;
    }
}

/*  Executes the CBZ instruction.
 */
void execute_cbz(data_t *instruction_data) {
    uint64_t offset = sign_extend(instruction_data->COND_BR_address << 2, 21);
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rt];
    if (operand1 == 0x0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
        BRANCH_FLAG = TRUE;
    }
}

/* Executes the CBNZ instruction.
 */
void execute_cbnz(data_t *instruction_data) {
    uint64_t offset = sign_extend(instruction_data->COND_BR_address << 2, 21);
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rt];
    if (operand1 != 0x0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
        BRANCH_FLAG = TRUE;
    }
}

/* Executes the LSL (immediate) and LSR (immediate) instructions.
 */
void execute_lsl_lsr_imm(data_t *instruction_data) {
    uint64_t src = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t immr = instruction_data->ALU_immediate >> 6;
    uint64_t imms = instruction_data->ALU_immediate & 0x3f;
    uint64_t result;
    if (imms != 0x3f) { // LSL
        result = src << (64 - immr);
    } else { // LSR
        result = src >> immr;
    }
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}

/* Executes the ADDS (extended register) instruction.
 */
void execute_adds_er(data_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t operand2 = CURRENT_STATE.REGS[instruction_data->Rm];
    uint64_t result = operand1 + operand2;
    NEXT_STATE.FLAG_N = result >> 63;
    NEXT_STATE.FLAG_Z = result == 0;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}

/* Executes the SUBS (extended register) and
 * CMP (extended register) instructions.
 */
void execute_subs_cmp_er(data_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t operand2 = CURRENT_STATE.REGS[instruction_data->Rm];
    uint64_t result = operand1 + ~operand2 + 1;
    NEXT_STATE.FLAG_N = result >> 63;
    NEXT_STATE.FLAG_Z = result == 0;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}

/* Executes the BR instruction.
 */
void execute_br(data_t *instruction_data) {
    if (instruction_data->Rm == 0x1f &&
        instruction_data->shamt == 0x0 &&
        instruction_data->Rd == 0x0) {
        NEXT_STATE.PC = CURRENT_STATE.REGS[instruction_data->Rn];
        BRANCH_FLAG = TRUE;
    }
}

/* Executes the ADD (extended register) instruction.
 */
void execute_add_er(data_t *instruction_data) {
    uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
    uint64_t operand2 = CURRENT_STATE.REGS[instruction_data->Rm];
    uint64_t result = operand1 + operand2;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
        BRANCH_FLAG = TRUE;
    }
}

/* Executes the MUL instruction.
 */
void execute_mul(data_t *instruction_data) {
    if (instruction_data->shamt == XZR) {
        uint64_t operand1 = CURRENT_STATE.REGS[instruction_data->Rn];
        uint64_t operand2 = CURRENT_STATE.REGS[instruction_data->Rm];
        uint64_t result = operand1 * operand2;
        if (instruction_data->Rd != XZR) {
            NEXT_STATE.REGS[instruction_data->Rd] = result;
        }
    }
}

/* Executes the STUR instruction.
 */
void execute_stur(data_t *instruction_data) {
    if (instruction_data->op == 0x0) {
        uint64_t offset = sign_extend(instruction_data->DT_address, 9);
        uint64_t address = CURRENT_STATE.REGS[instruction_data->Rn] + offset;
        uint64_t data = CURRENT_STATE.REGS[instruction_data->Rt];
        mem_write_32(address, data & 0xffffffff);
        mem_write_32(address + 4, data >> 32);
    }
}

/* Executes the STURB instruction.
 */
void execute_sturb(data_t *instruction_data) {
    if (instruction_data->op == 0x0) {
        uint64_t offset = sign_extend(instruction_data->DT_address, 9);
        uint64_t address = CURRENT_STATE.REGS[instruction_data->Rn] + offset;
        uint32_t data = CURRENT_STATE.REGS[instruction_data->Rt] & 0xff;
        data |= mem_read_32(address) & 0xffffff00;
        mem_write_32(address, data);
    }
}

/* Executes the STURH instruction.
 */
void execute_sturh(data_t *instruction_data) {
    if (instruction_data->op == 0x0) {
        uint64_t offset = sign_extend(instruction_data->DT_address, 9);
        uint64_t address = CURRENT_STATE.REGS[instruction_data->Rn] + offset;
        uint64_t data = CURRENT_STATE.REGS[instruction_data->Rt] & 0xffff;
        data |= mem_read_32(address) & 0xffff0000;
        mem_write_32(address, data);
    }
}

/*
// Función para invertir el orden de los bytes en un valor de 32 bits
// al pedo?
uint32_t reverse_bytes_32(uint32_t value) {
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0xFF000000) >> 24);
}
*/

/* Executes the LDUR instruction
*/
void execute_ldur(data_t *instruction_data) {
    if (instruction_data->op == 0x0) {
        uint64_t offset = sign_extend(instruction_data->DT_address, 9);
        uint64_t address = CURRENT_STATE.REGS[instruction_data->Rn] + offset;
        uint32_t lower_half = mem_read_32(address);      
        uint32_t upper_half = mem_read_32(address + 4);   
        /*
        printf("Address: 0x%lx\n", address);
        printf("Lower Half: 0x%x\n", lower_half);
        printf("Upper Half: 0x%x\n", upper_half);
        */
        uint64_t data = ((uint64_t)lower_half) | ((uint64_t)upper_half << 32);
        //printf("Data: 0x%lx\n", data);
        NEXT_STATE.REGS[instruction_data->Rt] = data;
    }
}

/* Executes the LDURB instruction
*/
void execute_ldurb(data_t *instruction_data) {
    if (instruction_data->op == 0x0) { 
        uint64_t offset = sign_extend(instruction_data->DT_address, 9);
        uint64_t address = CURRENT_STATE.REGS[instruction_data->Rn] + offset;
        uint32_t word = mem_read_32(address);  
        uint8_t data = (uint8_t)(word & 0xFF);
        /*
        printf("Address: 0x%lx\n", address);
        printf("Data: 0x%x\n", data);
        */
        NEXT_STATE.REGS[instruction_data->Rt] = (uint64_t)data;
    }
}

/* Executes LDURH instruction
*/
void execute_ldurh(data_t *instruction_data) {
    if (instruction_data->op == 0x0) { 
        uint64_t offset = sign_extend(instruction_data->DT_address, 9); 
        uint64_t address = CURRENT_STATE.REGS[instruction_data->Rn] + offset; 
        uint32_t word = mem_read_32(address);  
        uint16_t halfword = (uint16_t)(word & 0xFFFF); 
        NEXT_STATE.REGS[instruction_data->Rt] = (uint64_t)(int16_t)halfword;
    }
}

/* Executes the HLT instruction.
 */
void execute_hlt(data_t *instruction_data) {
    if (instruction_data->Rd == 0x0) {
        RUN_BIT = FALSE;
    }
}

/* Executes the MOVZ instruction.
 */
void execute_movz(data_t *instruction_data) {
    uint64_t result = instruction_data->MOV_immediate;
    if (instruction_data->Rd != XZR) {
        NEXT_STATE.REGS[instruction_data->Rd] = result;
    }
}
