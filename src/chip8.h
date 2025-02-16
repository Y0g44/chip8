/*
 * Copyright (c) 2025 Yoga. All rights reserved.
 *
 * This work is licensed under the terms of the MIT license.  
 * For a copy, see <https://opensource.org/licenses/MIT>.
*/

#define CHIP8_MEMORY_SIZE 4096
#define CHIP8_STACK_SIZE 16
#define CHIP8_KEY_STACK_SIZE 32
#define CHIP8_VRAM_SIZE 256
#define CHIP8_FLAG_REGISTER 15
#define CHIP8_NUMBER_REGISTERS 16
#define CHIP8_START_LOCATION 512
#define CHIP8_MAX_ROM_SIZE CHIP8_MEMORY_SIZE - CHIP8_START_LOCATION

typedef unsigned char CHIP8_register_address;
typedef unsigned int CHIP8_address;
typedef unsigned char CHIP8_vram_address;
typedef unsigned char CHIP8_byte;
typedef int CHIP8_word;

typedef enum CHIP8_key {
  CHIP8_key_none,
  CHIP8_key_1,
  CHIP8_key_2,
  CHIP8_key_3,
  CHIP8_key_4,
  CHIP8_key_5,
  CHIP8_key_6,
  CHIP8_key_7,
  CHIP8_key_8,
  CHIP8_key_9,
  CHIP8_key_0,
  CHIP8_key_A,
  CHIP8_key_B,
  CHIP8_key_C,
  CHIP8_key_D,
  CHIP8_key_E,
  CHIP8_key_F
} CHIP8_key;

typedef enum CHIP8_errors {
  CHIP8_Ok,
  CHIP8_FileNotFound,
  CHIP8_ROMTooBig,
  CHIP8_MemoryAllocationFailed,
  CHIP8_MemoryAddressNotFound,
  CHIP8_RegisterNotFound,
  CHIP8_InvalidArguments,
  CHIP8_StackOverflow,
  CHIP8_NoCall
} CHIP8_errors;

typedef struct CHIP8_chip8 {
  CHIP8_byte r[CHIP8_NUMBER_REGISTERS];
  CHIP8_byte dt, st;
  CHIP8_address pc;
  CHIP8_word i, sp, kp;
  
  CHIP8_byte wait;
  CHIP8_address stack[CHIP8_STACK_SIZE];
  CHIP8_key keys[CHIP8_KEY_STACK_SIZE];
  CHIP8_byte vram[CHIP8_VRAM_SIZE];
  CHIP8_byte memory[CHIP8_MEMORY_SIZE];
} CHIP8_chip8;

void CHIP8_init(CHIP8_chip8* chip8);
CHIP8_key CHIP8_kpeek(CHIP8_chip8* chip8);
CHIP8_key CHIP8_kpop(CHIP8_chip8* chip8);
void CHIP8_kpush(CHIP8_chip8* chip8, CHIP8_key key);
void CHIP8_cls(CHIP8_chip8* chip8);
CHIP8_errors CHIP8_jmp(CHIP8_chip8* chip8, CHIP8_address addr);
CHIP8_errors CHIP8_call(CHIP8_chip8* chip8, CHIP8_address addr);
CHIP8_errors CHIP8_ret(CHIP8_chip8* chip8);
CHIP8_errors CHIP8_eq1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_byte b);
CHIP8_errors CHIP8_eq2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_neq1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_byte b);
CHIP8_errors CHIP8_neq2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_addr1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_byte b);
CHIP8_errors CHIP8_addr2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_setr1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_byte byte);
CHIP8_errors CHIP8_setr2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_or(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_and(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_xor(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_sub(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_subn(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2);
CHIP8_errors CHIP8_shr(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_shl(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_seti(CHIP8_chip8* chip8, CHIP8_address addr);
CHIP8_errors CHIP8_chseti(CHIP8_chip8* chip8, CHIP8_byte ch);
CHIP8_errors CHIP8_rjmp(CHIP8_chip8* chip8, CHIP8_address addr);
CHIP8_errors CHIP8_rand(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_byte b);
CHIP8_errors CHIP8_draw(CHIP8_chip8* chip8, CHIP8_register_address rx, CHIP8_register_address ry, CHIP8_byte n);
CHIP8_errors CHIP8_waitkey(CHIP8_chip8* chip8, CHIP8_register_address r);
void CHIP8_skp(CHIP8_chip8* chip8, CHIP8_key key);
void CHIP8_sknp(CHIP8_chip8* chip8, CHIP8_key key);
CHIP8_errors CHIP8_setdt(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_setst(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_addi(CHIP8_chip8* chip8, CHIP8_register_address r);
CHIP8_errors CHIP8_bcd(CHIP8_chip8* chip8, CHIP8_byte regAddr);
CHIP8_errors CHIP8_rprtomem(CHIP8_chip8* chip8, CHIP8_byte r);
CHIP8_errors CHIP8_rpmemtor(CHIP8_chip8* chip8, CHIP8_byte r);
CHIP8_errors CHIP8_cycle(CHIP8_chip8* chip8);
CHIP8_errors CHIP8_loadrom(CHIP8_chip8* chip8, const char* rom);
CHIP8_errors CHIP8_loadfile(CHIP8_chip8* chip8, const char* path);
