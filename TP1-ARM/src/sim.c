#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "shell.h"
#include "sim.h"
#include "decode.h"
#include "execute.h"

// Variable global para la bandera de salto de instrucción
int BRANCH_FLAG = FALSE;

instruction_t instructions[] = {
    {6, 0x5, decode_b, execute_b},              // B
    {8, 0xea, decode_r, execute_ands_sr},       // ANDS (shifted register)
    {8, 0xca, decode_r, execute_eor_sr},        // EORS (shifted register)
    {8, 0xaa, decode_r, execute_orr_sr},        // ORRS (shifted register)
    {8, 0x91, decode_i, execute_add_imm},       // ADD (immediate)
    {8, 0xb1, decode_i, execute_adds_imm},      // ADDS (immediate)
    {8, 0xf1, decode_i, execute_subs_cmp_imm},  // SUBS and CMP (immediate)
    {8, 0x54, decode_cb, execute_b_cond},       // B.cond
    {8, 0xb4, decode_cb, execute_cbz},          // CBZ
    {8, 0xb5, decode_cb, execute_cbnz},         // CBNZ
    {10, 0x34d, decode_i, execute_lsl_lsr_imm}, // LSL and LSR (immediate)
    {11, 0x558, decode_r, execute_adds_er},     // ADDS (extended register)
    {11, 0x758, decode_r, execute_subs_cmp_er}, // SUBS and CMP (extended register)
    {11, 0x6b0, decode_r, execute_br},          // BR
    {11, 0x458, decode_r, execute_add_er},      // ADD (extended register)
    {11, 0x4d8, decode_r, execute_mul},         // MUL
    {11, 0x7c0, decode_d, execute_stur},        // STUR
    {11, 0x1c0, decode_d, execute_sturb},       // STURB
    {11, 0x3c0, decode_d, execute_sturh},       // STURH
    {11, 0x7c2, decode_d, execute_ldur},        // LDUR
    {11, 0x1c2, decode_d, execute_ldurb},       // LDURB
    {11, 0x3c2, decode_d, execute_ldurh},       // LDURH
    {11, 0x6a2, decode_iw, execute_hlt},        // HLT
    {11, 0x694, decode_iw, execute_movz},       // MOVZ
};

void process_instruction() {
    uint32_t instruction_code = mem_read_32(CURRENT_STATE.PC);

    for (size_t i = 0; i < sizeof(instructions) / sizeof(instruction_t); i++) {
        uint32_t opcode = instruction_code >> (32 - instructions[i].opcode_length);
        
        if (instructions[i].opcode == opcode) {
            data_t instruction_data;
            instructions[i].decode(&instruction_data, instruction_code);  // Decodificación
            instructions[i].execute(&instruction_data);  // Ejecución

            if (!BRANCH_FLAG) {
                NEXT_STATE.PC = CURRENT_STATE.PC + 4;
            } else {
                BRANCH_FLAG = FALSE;
            }

            return;
        }
    }

    // printf("Error: Instrucción desconocida en 0x%08x\n", CURRENT_STATE.PC);
    // NEXT_STATE.PC = CURRENT_STATE.PC + 4;
}
