/*
 * Copyright (c) 2025 Yoga. All rights reserved.
 *
 * This work is licensed under the terms of the MIT license.  
 * For a copy, see <https://opensource.org/licenses/MIT>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "chip8.h"

#define __CHIP8_chckReg1(r) if (r >= 16) { return CHIP8_RegisterNotFound; }
#define __CHIP8_chckReg2(r1, r2) if (r1 >= 16 && r2 >= 16) { return CHIP8_RegisterNotFound; }
#define __CHIP8_chckAddr(a)   if (a > CHIP8_MEMORY_SIZE) { return CHIP8_MemoryAddressNotFound; }

static int fillOneBit(unsigned short n) {  
  switch (n) {
    case 1: return 0b1;
    break;
    case 2: return 0b11;
    break;
    case 3: return 0b111;
    break;
    case 4: return 0b1111;
    break;
    case 5: return 0b11111;
    break;
    case 6: return 0b111111;
    break;
    case 7: return 0b1111111;
  }
  return 0b11111111;
}

// Sprite characters that will be stored in memory.
const CHIP8_word sprites[80] = {
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

// This function will do the necessary things before chip8 is used.
// This function is only executed once.
void CHIP8_init(CHIP8_chip8* chip8) {
  // Set the initial value of the member
  // Some members do not set the initial value,
  // because even if the value is not set, it will not have an effect.
  chip8->wait = 0;
  chip8->pc = CHIP8_START_LOCATION;
  chip8->dt = 0;
  chip8->st = 0;
  chip8->i = 0;
  chip8->sp = 0;
  chip8->kp = 0;

  chip8->memory = malloc(sizeof(CHIP8_word) * CHIP8_MEMORY_SIZE);
  chip8->vram = malloc(sizeof(CHIP8_word) * CHIP8_VRAM_SIZE);
  memset(chip8->memory, 0, sizeof(CHIP8_word) * CHIP8_MEMORY_SIZE);
  memset(chip8->vram, 0, sizeof(CHIP8_word) * CHIP8_VRAM_SIZE);
  memset(chip8->r, 0, sizeof(CHIP8_word) * CHIP8_NUMBER_REGISTERS);
  memset(chip8->stack, 0, sizeof(CHIP8_address) * CHIP8_STACK_SIZE);
  for (unsigned short int i = 0; i < 80; i++) {
    chip8->memory[i] = sprites[i];
  }
}

void CHIP8_next(CHIP8_chip8* chip8) {
  if (!chip8->wait) {
    chip8->pc += 2;
  }
}

// Get the key that was last pressed, without delete it.
CHIP8_key CHIP8_kpeek(CHIP8_chip8* chip8) {
  return chip8->kp == 0 || chip8->kp > CHIP8_KEY_STACK_SIZE
    ? CHIP8_key_none
    : chip8->keys[chip8->kp];
}

// Get the key that was last pressed, then pop it.
CHIP8_key CHIP8_kpop(CHIP8_chip8* chip8) {
  CHIP8_key head = CHIP8_kpeek(chip8);
  if (head != CHIP8_key_none) chip8->kp--;
  return head;
}

// Add newly pressed key to the stack.
void CHIP8_kpush(CHIP8_chip8* chip8, CHIP8_key key) {
  if (key == CHIP8_key_none) {
    return;
  }

  chip8->keys[chip8->kp > CHIP8_KEY_STACK_SIZE ? chip8->kp : ++chip8->kp] = key;
}

// Clear the screen of emulator / VRAM.
void CHIP8_cls(CHIP8_chip8* chip8) {
  for (unsigned short int i = 0; i < CHIP8_VRAM_SIZE; i++) {
    chip8->vram[i] = 0;
  }
}

// Set the program counter.
CHIP8_errors CHIP8_jmp(CHIP8_chip8* chip8, CHIP8_address addr) {
  __CHIP8_chckAddr(addr)
  chip8->pc = addr;
  return CHIP8_Ok;
}

// Call subroutine to the specified address.
CHIP8_errors CHIP8_call(CHIP8_chip8* chip8, CHIP8_address addr) {
  __CHIP8_chckAddr(addr)
  CHIP8_errors err = CHIP8_Ok;
  
  // If the stack has run out of space, the emulator will attempt to forcefully add to it
  // by overwriting the address at the very top of the stack.
  if (chip8->sp >= CHIP8_STACK_SIZE) {
    chip8->stack[chip8->sp - 1] = chip8->pc;
    err = CHIP8_StackOverflow;
  } else {
    chip8->stack[chip8->sp++] = chip8->pc;
  }

  chip8->pc = addr;
  return err;
}

// Return from a subroutine.
CHIP8_errors CHIP8_ret(CHIP8_chip8* chip8) {
  if (chip8->sp == 0) {
    return CHIP8_NoCall;
  }
  
  chip8->pc = chip8->stack[--chip8->sp];
  return CHIP8_Ok;
}

// If r == b is true, then skip to the next instruction.
CHIP8_errors CHIP8_eq1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word b) {
  __CHIP8_chckReg1(r)
  if (chip8->r[r] == b) {
    CHIP8_next(chip8);
  }
  return CHIP8_Ok;
}

// If r1 == r2 is true, then skip to the next instruction.
CHIP8_errors CHIP8_eq2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2) {
  __CHIP8_chckReg2(r1, r2)
  if (chip8->r[r1] == chip8->r[r2]) {
    CHIP8_next(chip8);
  }
  return CHIP8_Ok;
}

// If r != b is true, then skip to the next instruction.
CHIP8_errors CHIP8_neq1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word b) {
  __CHIP8_chckReg1(r)
  if (chip8->r[r] != b) {
    CHIP8_next(chip8);
  }
  return CHIP8_Ok;
}

// If r1 != r2 is true, then skip to the next instruction.
CHIP8_errors CHIP8_neq2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2) {
  __CHIP8_chckReg2(r1, r2)
  if (chip8->r[r1] != chip8->r[r2]) {
    CHIP8_next(chip8);
  }
  return CHIP8_Ok;
}

// Add r with b and then store to r (r = r + b), set 1 to Vf if the result > 255.
CHIP8_errors CHIP8_addr1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word b) {
  __CHIP8_chckReg1(r)
  chip8->r[CHIP8_FLAG_REGISTER] = (unsigned int)chip8->r[r] + b > 255 ? 1 : 0;
  chip8->r[r] += b;
  return CHIP8_Ok;
}

// Add r1 with r2 and then store to r1 (r1 = r1 + r2), set 1 to Vf if the result > 255.
CHIP8_errors CHIP8_addr2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2) {
  __CHIP8_chckReg2(r1, r2)
  chip8->r[CHIP8_FLAG_REGISTER] = (unsigned int)chip8->r[r1] + chip8->r[r2] > 255 ? 1 : 0;
  chip8->r[r1] += chip8->r[r2];
  return CHIP8_Ok;
}

// Store data to r (register).
CHIP8_errors CHIP8_setr1(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word byte) {
  __CHIP8_chckReg1(r)
  chip8->r[r] = byte;
  return CHIP8_Ok;
}

// Store data from r2 to r1.
CHIP8_errors CHIP8_setr2(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2) {
  __CHIP8_chckReg2(r1, r2)
  chip8->r[r1] = chip8->r[r2];
  return CHIP8_Ok;
}

// The or operation to the two registers (r1 & r2).
CHIP8_errors CHIP8_or(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2) {
  __CHIP8_chckReg2(r1, r2)
  chip8->r[r1] |= chip8->r[r2];
  return CHIP8_Ok;
}

// The and operation to the two registers (r1 & r2).
CHIP8_errors CHIP8_and(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2) {
  __CHIP8_chckReg2(r1, r2)
  chip8->r[r1] &= chip8->r[r2];
  return CHIP8_Ok;
}

// The xor operation to the two registers (r1 & r2).
CHIP8_errors CHIP8_xor(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2) {
  __CHIP8_chckReg2(r1, r2)
  chip8->r[r1] ^= chip8->r[r2];
  chip8->r[CHIP8_FLAG_REGISTER] = ((chip8->r[r1] ^ chip8->r[r2]) & ~chip8->r[r1]) != 0;
  return CHIP8_Ok;
}

// Subtract r1 from r2 and store to r1 (r1 = r1 - r2), set 1 to Vf if r2 > r1.
CHIP8_errors CHIP8_sub(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2) {
  __CHIP8_chckReg2(r1, r2)
  chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[r2] > chip8->r[r1] ? 1 : 0;
  chip8->r[r1] = chip8->r[r1] - chip8->r[r2];
  return CHIP8_Ok;
}

// Subtract r2 from r1 and store to r1 (r1 = r2 - r1), set 1 to Vf if r1 > r2.
CHIP8_errors CHIP8_subn(CHIP8_chip8* chip8, CHIP8_register_address r1, CHIP8_register_address r2) {
  __CHIP8_chckReg2(r1, r2)
  chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[r2] > chip8->r[r1] ? 1 : 0;
  chip8->r[r1] = chip8->r[r2] - chip8->r[r1];
  return CHIP8_Ok;
}

// Bitwise shift right from r then store to r (r = r >> 1).
CHIP8_errors CHIP8_shr(CHIP8_chip8* chip8, CHIP8_register_address r) {
  __CHIP8_chckReg1(r)
  chip8->r[r] >>= 1;
  chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[r] & 0b1;
  return CHIP8_Ok;
}

// Bitwise shift left from r then store to r (r = r << 1).
CHIP8_errors CHIP8_shl(CHIP8_chip8* chip8, CHIP8_register_address r) {
  __CHIP8_chckReg1(r)
  chip8->r[r] <<= 1;
  chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[r] & 0x80;
  return CHIP8_Ok;
}

// Store the address to I.
CHIP8_errors CHIP8_seti(CHIP8_chip8* chip8, CHIP8_address addr) {
  __CHIP8_chckAddr(addr)
  chip8->i = addr;
  return CHIP8_Ok;
}

// Store the address of the characters sprite to I.
CHIP8_errors CHIP8_chseti(CHIP8_chip8* chip8, CHIP8_word r) {
  __CHIP8_chckReg1(r);
  CHIP8_word ch = chip8->r[r];
  if (ch > 15) {
    return CHIP8_InvalidArguments;
  }

  // A naive way to get the address. Since the sprites are stored sequentially,
  // just multiply the number of bytes of the sprite (5) by its character (0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F).
  chip8->i = ch * 5;
  return CHIP8_Ok;
}

// Jump to address V0 + r.
CHIP8_errors CHIP8_rjmp(CHIP8_chip8* chip8, CHIP8_address addr) {
  __CHIP8_chckAddr(chip8->r[0] + addr)
  chip8->pc = chip8->r[0] + addr;
  return CHIP8_Ok;
}

// Get a random value (rand() & b) then store it to the r.
CHIP8_errors CHIP8_rand(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word b) {
  __CHIP8_chckReg1(r)
  chip8->r[r] = rand() & b;
  return CHIP8_Ok;
}

// Draw the pixel to position (rx, ry) to VRAM.
CHIP8_errors CHIP8_draw(CHIP8_chip8* chip8, CHIP8_register_address rx, CHIP8_register_address ry, CHIP8_word n) {
  __CHIP8_chckReg2(rx, ry);
  CHIP8_word x = chip8->r[rx] % 64;
  CHIP8_word y = chip8->r[ry] % 32;
  CHIP8_vram_address ivr = ((x / 8) + (y * 8)) % CHIP8_VRAM_SIZE;
  CHIP8_vram_address nvr = x % 8;

  for (CHIP8_word i = 0; i < n; i++) {
    CHIP8_vram_address x1 = ivr + i;
    CHIP8_vram_address x2 = ivr + i + 1;
    CHIP8_word b1 = (chip8->memory[chip8->i + i] & fillOneBit(8 - nvr)) << nvr;
    CHIP8_word b2 = chip8->memory[chip8->i + i] >> (8 - nvr);
    chip8->vram[x1] ^= b1;
    chip8->vram[x2] ^= b2;
    chip8->r[CHIP8_FLAG_REGISTER] = ((chip8->vram[x1] ^ b1) & ~chip8->vram[x1]) != 0
      || ((chip8->vram[x2] ^ b2) & ~chip8->vram[x2]) != 0;
  }
  
  return CHIP8_Ok;
}

// Wait for the key to be pressed, then store the newly pressed key to r.
CHIP8_errors CHIP8_waitkey(CHIP8_chip8* chip8, CHIP8_register_address r) {
  __CHIP8_chckReg1(r);
  CHIP8_key pressedKey = CHIP8_kpop(chip8);
  if (pressedKey == CHIP8_key_none) {
    chip8->wait = 1;
  } else {
    chip8->r[r] = pressedKey;
    chip8->wait = 0;
  }
  return CHIP8_Ok;
}

// If the key is pressed, then skip to the next instruction.
CHIP8_errors CHIP8_skp(CHIP8_chip8* chip8, CHIP8_register_address r) {
  __CHIP8_chckReg1(r)
  if (CHIP8_kpop(chip8) == chip8->r[r]) {
    CHIP8_next(chip8);
  }
  return CHIP8_Ok;
}

// If the key is not pressed, then skip to the next instruction.
CHIP8_errors CHIP8_sknp(CHIP8_chip8* chip8, CHIP8_register_address r) {
  __CHIP8_chckReg1(r)
  if (CHIP8_kpop(chip8) != chip8->r[r]) {
    CHIP8_next(chip8);
  }
  return CHIP8_Ok;
}

// Set r with DT (r = DT).
CHIP8_errors CHIP8_setrdt(CHIP8_chip8* chip8, CHIP8_register_address r) {
  __CHIP8_chckReg1(r);
  chip8->r[r] = chip8->dt;
  return CHIP8_Ok;
}

// Set the delay timer with r(DT = r).
CHIP8_errors CHIP8_setdt(CHIP8_chip8* chip8, CHIP8_register_address r) {
  __CHIP8_chckReg1(r);
  chip8->dt = chip8->r[r];
  return CHIP8_Ok;
}

// Set the sound timer with r(ST = r).
CHIP8_errors CHIP8_setst(CHIP8_chip8* chip8, CHIP8_register_address r) {
  __CHIP8_chckReg1(r);
  chip8->st = chip8->r[r];
  return CHIP8_Ok;
}

// Add the address I with r (I = I + r).
CHIP8_errors CHIP8_addi(CHIP8_chip8* chip8, CHIP8_register_address r) {
  __CHIP8_chckReg1(r);
  chip8->i += chip8->r[r];
  return CHIP8_Ok;
}

// Break the decimal value into unit, ten and hundred, then store it in I, I+1, I+2.
CHIP8_errors CHIP8_bcd(CHIP8_chip8* chip8, CHIP8_word regAddr) {
  __CHIP8_chckReg1(regAddr);
  chip8->memory[chip8->i] = (chip8->r[regAddr] / 100) % 10; ;
  chip8->memory[chip8->i + 1] = (chip8->r[regAddr] / 10) % 10;
  chip8->memory[chip8->i + 2] = chip8->r[regAddr] % 10;
  return CHIP8_Ok;
}

// Replace the memory data at address I ++ with values from V0 to Vr.
CHIP8_errors CHIP8_memtor(CHIP8_chip8* chip8, CHIP8_word r) {
  __CHIP8_chckReg1(r);
  for (unsigned short i = 0; i < r; i++) {
    chip8->memory[chip8->i + i] = chip8->r[i];
  }
  return CHIP8_Ok;
}

// Replace data V0 to Vr with address I ++.
CHIP8_errors CHIP8_rtomem(CHIP8_chip8* chip8, CHIP8_word r) {
  __CHIP8_chckReg1(r);
  for (unsigned short i = 0; i < r; i++) {
    chip8->r[i] = chip8->memory[chip8->i + i];
  }
  return CHIP8_Ok;
}

CHIP8_dword CHIP8_getop(CHIP8_chip8* chip8, CHIP8_address addr) {
  return (chip8->memory[addr] << 8) | chip8->memory[addr + 1];
}

// Execute the instruction in memory in 1 cycle.
CHIP8_errors CHIP8_cycle(CHIP8_chip8* chip8) {
  if (chip8->pc + 1 >= CHIP8_MEMORY_SIZE) {
    return CHIP8_MemoryAddressNotFound;
  }

  CHIP8_dword op = CHIP8_getop(chip8, chip8->pc);
  CHIP8_errors err = Chip8_InvalidOpcode;
  unsigned short jump = 0;

  CHIP8_dword op_nnn = op & 0x0FFF;
  CHIP8_dword op_nn = op & 0x00FF;
  CHIP8_dword op_n = op & 0x000F;
  CHIP8_dword op_r1 = (op & 0x0F00) >> 8;
  CHIP8_dword op_r2 = (op & 0x00F0) >> 4;

  switch (op & 0xF000) {
    case 0x0000:
      if (op == 0x00E0) {
        CHIP8_cls(chip8);
        err = CHIP8_Ok;
      } else if (op == 0x00EE) {
        err = CHIP8_ret(chip8);
        jump = 1;
      }
    break;
    case 0x1000:
      err = CHIP8_jmp(chip8, op_nnn);
      jump = 1;
    break;
    case 0x2000:
      err = CHIP8_call(chip8, op_nnn);
      jump = 1;
    break;
    case 0x3000: err = CHIP8_eq1(chip8, op_r1, op_nn);
    break;
    case 0x4000: err = CHIP8_neq1(chip8, op_r1, op_nn);
    break;
    case 0x5000:
      if (op_n == 0x0) {
        err = CHIP8_eq2(chip8, op_r1, op_r2);
      }
    break;
    case 0x6000: err = CHIP8_setr1(chip8, op_r1, op_nn);
    break;
    case 0x7000: err = CHIP8_addr1(chip8, op_r1, op_nn);
    break;
    case 0x8000:
      switch (op_n) {
        case 0x0: err = CHIP8_setr2(chip8, op_r1, op_r2);
        break;
        case 0x1: err = CHIP8_or(chip8, op_r1, op_r2);
        break;
        case 0x2: err = CHIP8_and(chip8, op_r1, op_r2);
        break;
        case 0x3: err = CHIP8_xor(chip8, op_r1, op_r2);
        break;
        case 0x4: err = CHIP8_addr2(chip8, op_r1, op_r2);
        break;
        case 0x5: err = CHIP8_sub(chip8, op_r1, op_r2);
        break;
        case 0x6: err = CHIP8_shr(chip8, op_r1);
        break;
        case 0x7: err = CHIP8_subn(chip8, op_r1, op_r2);
        break;
        case 0xE: err = CHIP8_shl(chip8, op_r1);
      }
    break;
    case 0x9000: 
      if (op_n == 0x0) {
        err = CHIP8_neq2(chip8, op_r1, op_r2);
      }
    break;
    case 0xA000: err = CHIP8_seti(chip8, op_nnn);
    break;
    case 0xB000:
      err = CHIP8_rjmp(chip8, op_nnn);
      jump = 1;
    break;
    case 0xC000: err = CHIP8_rand(chip8, op_r1, op_nn);
    break;
    case 0xD000:
    printf("%d %d %d %d\n", op_r1, op_r2, op_n, op);
      err = CHIP8_draw(chip8, op_r1, op_r2, op_n);
    break;
    case 0xE000:
      if (op_nn == 0x9E) {
        err = CHIP8_skp(chip8, op_r1);
      } else if (op_nn == 0xA1) {
        err = CHIP8_sknp(chip8, op_r1);
      }
    break;
    case 0xF000:
      switch (op_nn) {
        case 0x07: err = CHIP8_setrdt(chip8, op_r1);
        break;
        case 0x0A: err = CHIP8_waitkey(chip8, op_r1);
        break;
        case 0x15: err = CHIP8_setdt(chip8, op_r1);
        break;
        case 0x18: err = CHIP8_setst(chip8, op_r1);
        break;
        case 0x1E: err = CHIP8_addi(chip8, op_r1);
        break;
        case 0x29: err = CHIP8_chseti(chip8, op_r1);
        break;
        case 0x33: err = CHIP8_bcd(chip8, op_r1);
        break;
        case 0x55: err = CHIP8_memtor(chip8, op_r1);
        break;
        case 0x65: err = CHIP8_rtomem(chip8, op_r1);
      }
  }

  if (chip8->dt) {
    chip8->dt--;
  }

  if (chip8->st) {
    chip8->st--;
  }

  if (!jump) {
    CHIP8_next(chip8);
  }
  return err;
}

// Load the ROM into memory.
CHIP8_errors CHIP8_loadrom(CHIP8_chip8* chip8, CHIP8_word* rom, CHIP8_size n) {
  if(n > CHIP8_MAX_ROM_SIZE) {
    return CHIP8_ROMTooBig;
  }

  printf("ROM = %d\n", rom[1]);
  for (unsigned short i = 0; i < n; i++) {
    chip8->memory[CHIP8_START_LOCATION + i] = rom[i];
  }

  return CHIP8_Ok;
}

// Load the ROM file into memory.
CHIP8_errors CHIP8_loadfile(CHIP8_chip8* chip8, const char* path) {
  FILE* fp = fopen(path, "r");
  if (!fp) {
    return CHIP8_FileNotFound;
  }

  fseek(fp, 0, SEEK_END);
  unsigned long ROMSize = ftell(fp);
  if(ROMSize > CHIP8_MAX_ROM_SIZE) {
    return CHIP8_ROMTooBig;
  }

  char* rom = malloc(sizeof(CHIP8_word) * ROMSize);
  if (!rom) {
    return CHIP8_MemoryAllocationFailed;
  }

  fseek(fp, 0, SEEK_SET);
  fread(rom, sizeof(CHIP8_word), ROMSize, fp);
  fclose(fp);

  for (unsigned short i = 0; i < ROMSize; i++) {
    chip8->memory[CHIP8_START_LOCATION + i] = rom[i];
  }
  free(rom);

  return CHIP8_Ok;
}
