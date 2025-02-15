/*
Copyright (c) 2025 Yoga

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdlib.h>
#include "chip8.h"

#define __CHIP8_chckReg1(r) if (r >= 16) { return CHIP8_RegisterNotFound; }
#define __CHIP8_chckReg2(r1, r2) if (r1 >= 16 && r2 >= 16) { return CHIP8_RegisterNotFound; }

int binaryFillOne(unsigned short n) {
  switch (n) {
    case 1: return 0b1;
    case 2: return 0b11;
    case 3: return 0b111;
    case 4: return 0b1111;
    case 5: return 0b11111;
    case 6: return 0b111111;
    case 7: return 0b1111111;
  }

  return 0;
}

void CHIP8_init(CHIP8_chip8* chip8) {
  CHIP8_byte sprites[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xD0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
  };

  chip8->dt = 0;
  chip8->st = 0;
  chip8->pc = 0x200;
  chip8->i = 0;
  chip8->sp = 0;
  chip8->kp = 0;

  for (unsigned short int i = 0; i < CHIP8_MEMORY_SIZE; i++) {
    chip8->memory[i] = i < 80 ? sprites[i] : 0;
  }

  for (unsigned short int i = 0; i < CHIP8_NUMBER_REGISTERS; i++) {
    chip8->r[i] = 0;
  }

  for (unsigned short int i = 0; i < CHIP8_VRAM_SIZE; i++) {
    chip8->vram[i] = 0;
  }
}

void CHIP8_cls(CHIP8_chip8* chip8) {
  for (unsigned short int i = 0; i < CHIP8_VRAM_SIZE; i++) {
    chip8->vram[i] = 0;
  }
}

void CHIP8_jmp(CHIP8_chip8* chip8, CHIP8_address addr) {
  chip8->pc = addr;
}

int CHIP8_call(CHIP8_chip8* chip8, CHIP8_address addr) {
  if (chip8->sp >= CHIP8_STACK_SIZE) {
    chip8->stack[chip8->sp] = addr;
    return 1;
  }

  chip8->stack[chip8->sp++] = addr;
  return 0;
}

void CHIP8_ret(CHIP8_chip8* chip8) {
  chip8->pc = chip8->stack[--chip8->sp];
}

CHIP8_errors CHIP8_eq1(CHIP8_chip8* chip8, CHIP8_register_address regAddr, CHIP8_byte byte) {
  __CHIP8_chckReg1(regAddr)
  if (chip8->r[regAddr] == byte) chip8->pc++;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_eq2(CHIP8_chip8* chip8, CHIP8_register_address regAddr1, CHIP8_register_address regAddr2) {
  __CHIP8_chckReg2(regAddr1, regAddr2)
  if (chip8->r[regAddr1] == chip8->r[regAddr2]) chip8->pc++;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_neq1(CHIP8_chip8* chip8, CHIP8_register_address regAddr, CHIP8_byte byte) {
  __CHIP8_chckReg1(regAddr)
  if (chip8->r[regAddr] != byte) chip8->pc++;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_neq2(CHIP8_chip8* chip8, CHIP8_register_address regAddr1, CHIP8_register_address regAddr2) {
  __CHIP8_chckReg2(regAddr1, regAddr2)
  if (chip8->r[regAddr1] != chip8->r[regAddr2]) chip8->pc++;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_addr1(CHIP8_chip8* chip8, CHIP8_register_address regAddr, CHIP8_byte byte) {
  __CHIP8_chckReg1(regAddr)
  chip8->r[regAddr] += byte;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_addr2(CHIP8_chip8* chip8, CHIP8_register_address regAddr1, CHIP8_register_address regAddr2) {
  __CHIP8_chckReg2(regAddr1, regAddr2)
  chip8->r[CHIP8_FLAG_REGISTER] = (unsigned int)chip8->r[regAddr1] + chip8->r[regAddr2] >= 256 ? 1 : 0;
  chip8->r[regAddr1] = chip8->r[regAddr1] + chip8->r[regAddr2];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_setr1(CHIP8_chip8* chip8, CHIP8_register_address regAddr, CHIP8_byte byte) {
  __CHIP8_chckReg1(regAddr)
  chip8->r[regAddr] = byte;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_setr2(CHIP8_chip8* chip8, CHIP8_register_address regAddr1, CHIP8_register_address regAddr2) {
  __CHIP8_chckReg2(regAddr1, regAddr2)
  chip8->r[regAddr1] = chip8->r[regAddr2];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_or(CHIP8_chip8* chip8, CHIP8_register_address regAddr1, CHIP8_register_address regAddr2) {
  __CHIP8_chckReg2(regAddr1, regAddr2)
  chip8->r[regAddr1] |= chip8->r[regAddr2];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_and(CHIP8_chip8* chip8, CHIP8_register_address regAddr1, CHIP8_register_address regAddr2) {
  __CHIP8_chckReg2(regAddr1, regAddr2)
  chip8->r[regAddr1] &= chip8->r[regAddr2];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_xor(CHIP8_chip8* chip8, CHIP8_register_address regAddr1, CHIP8_register_address regAddr2) {
  __CHIP8_chckReg2(regAddr1, regAddr2)
  chip8->r[regAddr1] ^= chip8->r[regAddr2];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_sub(CHIP8_chip8* chip8, CHIP8_register_address regAddr1, CHIP8_register_address regAddr2) {
  __CHIP8_chckReg2(regAddr1, regAddr2)
  chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[regAddr1] > chip8->r[regAddr2] ? 1 : 0;
  chip8->r[regAddr1] = chip8->r[regAddr1] - chip8->r[regAddr2];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_subn(CHIP8_chip8* chip8, CHIP8_register_address regAddr1, CHIP8_register_address regAddr2) {
  __CHIP8_chckReg2(regAddr1, regAddr2)
  chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[regAddr2] > chip8->r[regAddr1] ? 1 : 0;
  chip8->r[regAddr1] = chip8->r[regAddr2] - chip8->r[regAddr1];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_shr(CHIP8_chip8* chip8, CHIP8_register_address regAddr) {
  __CHIP8_chckReg1(regAddr)
  chip8->r[regAddr] >>= 1;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_shl(CHIP8_chip8* chip8, CHIP8_register_address regAddr) {
  __CHIP8_chckReg1(regAddr)
  chip8->r[regAddr] <<= 1;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_seti1(CHIP8_chip8* chip8, CHIP8_address addr) {
  chip8->i = addr;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_rjmp(CHIP8_chip8* chip8, CHIP8_address addr) {
  chip8->pc = chip8->r[0] + addr;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_rand(CHIP8_chip8* chip8, CHIP8_register_address regAddr, CHIP8_byte byte) {
  __CHIP8_chckReg1(regAddr)
  chip8->r[regAddr] = rand() & byte;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_draw(CHIP8_chip8* chip8, CHIP8_register_address regAddrX, CHIP8_register_address regAddrY, CHIP8_byte n) {
  __CHIP8_chckReg2(regAddrX, regAddrY);
  CHIP8_vram_address x = chip8->r[regAddrX];
  CHIP8_vram_address y = chip8->r[regAddrY];
  CHIP8_vram_address vi = (y * 8) + (x / 8);
  CHIP8_vram_address vn = x % 8;

  for (unsigned short bi = chip8->i; bi <= n; bi++) {
    CHIP8_byte b1 = chip8->memory[bi] >> vn;
    CHIP8_byte b2 = chip8->memory[bi] & binaryFillOne(vn);

    CHIP8_vram_address nextVi = vi + 1 >= CHIP8_VRAM_SIZE ? CHIP8_VRAM_SIZE - 8 : vi + 1;
    CHIP8_byte xoredByte1 = chip8->vram[vi] ^ b1;
    CHIP8_byte xoredByte2 = chip8->vram[nextVi] ^ b2;

    if (xoredByte1 != (chip8->vram[vi] | b1) || xoredByte2 != (chip8->vram[vi] | b2)) {
      chip8->vram[CHIP8_FLAG_REGISTER] = 1;
    }

    chip8->vram[vi] = xoredByte1;
    chip8->vram[nextVi] = xoredByte2;
    vi = nextVi;
  }
}

CHIP8_key CHIP8_kpeek(CHIP8_chip8* chip8) {
  return chip8->kp == 0 || chip8->kp > CHIP8_KEY_STACK_SIZE
    ? CHIP8_key_none
    : chip8->keys[chip8->kp];
}

CHIP8_key CHIP8_kpop(CHIP8_chip8* chip8) {
  CHIP8_key head = CHIP8_kpeek(chip8);
  if (head != CHIP8_key_none) chip8->kp--;
  return head;
}

void CHIP8_kpush(CHIP8_chip8* chip8, CHIP8_key key) {
  chip8->keys[chip8->kp > CHIP8_KEY_STACK_SIZE ? chip8->kp : ++chip8->kp] = key;
}

void CHIP8_skp(CHIP8_chip8* chip8, CHIP8_key key) {
  if (CHIP8_kpop(chip8) == key) chip8->pc++;
}

void CHIP8_sknp(CHIP8_chip8* chip8, CHIP8_key key) {
  if (CHIP8_kpop(chip8) != key) chip8->pc++;
}

CHIP8_errors CHIP8_setdt(CHIP8_chip8* chip8, CHIP8_register_address regAddr) {
  __CHIP8_chckReg1(regAddr);
  chip8->dt = chip8->r[regAddr];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_setst(CHIP8_chip8* chip8, CHIP8_register_address regAddr) {
  __CHIP8_chckReg1(regAddr);
  chip8->st = chip8->r[regAddr];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_addi(CHIP8_chip8* chip8, CHIP8_register_address regAddr) {
  __CHIP8_chckReg1(regAddr);
  chip8->i += chip8->r[regAddr];
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_seti2(CHIP8_chip8* chip8, CHIP8_byte ch) {
  if (ch > 15) {
    return CHIP8_InvalidArguments;
  }
  chip8->i = ch * 5;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_bcd(CHIP8_chip8* chip8, CHIP8_byte regAddr) {
  __CHIP8_chckReg1(regAddr);
  chip8->memory[chip8->i] = chip8->r[regAddr] % 10;
  chip8->memory[chip8->i + 1] = (chip8->r[regAddr] / 10) % 10;
  chip8->memory[chip8->i + 2] = (chip8->r[regAddr] / 100) % 10;
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_cprtomem(CHIP8_chip8* chip8, CHIP8_byte regAddrEnd) {
  __CHIP8_chckReg1(regAddrEnd);
  for (unsigned short i = 0; i < regAddrEnd; i++) {
    chip8->memory[chip8->i + i] = chip8->r[i];
  }
  return CHIP8_Ok;
}

CHIP8_errors CHIP8_cpmemtor(CHIP8_chip8* chip8, CHIP8_byte regAddrEnd) {
  __CHIP8_chckReg1(regAddrEnd);
  for (unsigned short i = 0; i < regAddrEnd; i++) {
    chip8->r[i] = chip8->memory[chip8->i + i];
  }
  return CHIP8_Ok;
}
