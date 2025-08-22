#include "decode.h"

/*
CAMBIOS:
No veo donde tiene las funciones de MOVz y demas que al resto no le salen. 
voy leyendo el pdf devuelta para ver de donde sale
*/

/* Shifting function*/
uint32_t shift_bits(uint32_t instruction_code, int shift_amount, uint32_t mask) {
    return (instruction_code >> shift_amount) & mask;
}

/* Decoding function for R-format instructions. 
 */
void decode_r(data_t *data, uint32_t instruction_code) {
    data -> Rm = shift_bits(instruction_code, 16, 0x1f);
    data -> shamt = shift_bits(instruction_code, 10, 0x3f);
    data -> Rn = shift_bits(instruction_code, 5, 0x1f);
    data -> Rd = instruction_code & 0x1f;
}

/* Decoding function for I-format instructions. 
 */
void decode_i(data_t *data, uint32_t instruction_code) {
    data -> shift = shift_bits(instruction_code, 22, 0x3);
    data -> ALU_immediate = shift_bits(instruction_code, 10, 0xfff);
    data -> Rn = shift_bits(instruction_code, 5, 0x1f);
    data -> Rd = instruction_code & 0x1f;
}

/* Decoding function for D-format instructions. 
 */
void decode_d(data_t *data, uint32_t instruction_code) {
    data -> DT_address = shift_bits(instruction_code, 12, 0x1ff);
    data -> op = shift_bits(instruction_code, 10, 0x3);
    data -> Rn = shift_bits(instruction_code, 5, 0x1f);
    data -> Rt = instruction_code & 0x1f;
}

/* Decoding function for B-format instructions. 
 */
void decode_b(data_t *data, uint32_t instruction_code) {
    data -> BR_address = instruction_code & 0x3ffffff;
}

/* Decoding function for CB-format instructions. 
 */
void decode_cb(data_t *data, uint32_t instruction_code) {
    data -> COND_BR_address = shift_bits(instruction_code, 5, 0x7ffff);
    data -> Rt = instruction_code & 0x1f;
}

/* Decoding function for IW-format instructions. 
 */
void decode_iw(data_t *data, uint32_t instruction_code) {
    data -> MOV_immediate = shift_bits(instruction_code, 5, 0xffff);
    data -> Rd = instruction_code & 0x1f;
}