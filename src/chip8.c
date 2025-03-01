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

CHIP8_errors CHIP8_getr(CHIP8_chip8* chip8, CHIP8_register_address r, CHIP8_word* b) {
 if (r >= CHIP8_NUMBER_REGISTERS) {
  return CHIP8_RegisterNotFound;
 }

  *b = chip8->r[r];
  return CHIP8_Ok;
}

CHIP8_address CHIP8_geti(CHIP8_chip8* chip8) {
  return chip8->i;
}

// Set the program counter.
CHIP8_errors CHIP8_jmp(CHIP8_chip8* chip8, CHIP8_address addr) {
  if (addr >= CHIP8_MEMORY_SIZE) {
    return CHIP8_MemoryAddressNotFound;
  }

  chip8->pc = addr;
  return CHIP8_Ok;
}

// Call subroutine to the specified address.
CHIP8_errors CHIP8_call(CHIP8_chip8* chip8, CHIP8_address addr) {
  if (addr >= CHIP8_MEMORY_SIZE) {
    return CHIP8_MemoryAddressNotFound;
  }
  
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

CHIP8_address CHIP8_getspraddr(CHIP8_word ch, CHIP8_address* addr) {
  if (ch > 15) {
    return CHIP8_InvalidArguments;
  }

  // A naive way to get the address. Since the sprites are stored sequentially,
  // just multiply the number of bytes of the sprite (5) by its character (0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F).
  *addr = ch * 5;
  return CHIP8_Ok;
}


// Draw the pixel to position (rx, ry) to VRAM.
CHIP8_errors CHIP8_draw(CHIP8_chip8* chip8, CHIP8_register_address rx, CHIP8_register_address ry, CHIP8_word n) {
  CHIP8_word x = chip8->r[rx] % 64;
  CHIP8_word y = chip8->r[ry] % 32;
  CHIP8_vram_address ivr = ((x / 8) + (y * 8)) % CHIP8_VRAM_SIZE;
  CHIP8_vram_address nvr = x % 8;

  for (CHIP8_word i = 0; i < n; i++) {
    CHIP8_word b2 = chip8->memory[chip8->i + i] >> (8 - nvr);
    CHIP8_word b1 = chip8->memory[chip8->i + i];
    switch (nvr) {
      case 7: b1 &= 0b1;
      break;
      case 6: b1 &= 0b11;
      break;
      case 5: b1 &= 0b111;
      break;
      case 4: b1 &= 0b1111;
      break;
      case 3: b1 &= 0b11111;
      break;
      case 2: b1 &= 0b111111;
      break;
      case 1: b1 &= 0b1111111;
    }
    b1 <<= nvr;

    CHIP8_vram_address x1 = ivr + i;
    CHIP8_vram_address x2 = ivr + i + 1;
    chip8->vram[x1] ^= b1;
    chip8->vram[x2] ^= b2;
    chip8->r[CHIP8_FLAG_REGISTER] = ((chip8->vram[x1] ^ b1) & ~chip8->vram[x1]) != 0 ||
      ((chip8->vram[x2] ^ b2) & ~chip8->vram[x2]) != 0;
  }
  
  return CHIP8_Ok;
}

// Wait for the key to be pressed, then store the newly pressed key to r.
CHIP8_errors CHIP8_waitkey(CHIP8_chip8* chip8, CHIP8_register_address r) {
 if (r >= CHIP8_NUMBER_REGISTERS) {
  return CHIP8_RegisterNotFound;
 }

  CHIP8_key pressedKey = CHIP8_kpop(chip8);
  if (pressedKey == CHIP8_key_none) {
    chip8->wait = 1;
  } else {
    chip8->r[r] = pressedKey;
    chip8->wait = 0;
  }
  return CHIP8_Ok;
}

CHIP8_dword CHIP8_getop(CHIP8_chip8* chip8, CHIP8_address addr) {
  return (chip8->memory[addr] << 8) | chip8->memory[addr + 1];
}

CHIP8_errors CHIP8_parse(CHIP8_dword op, CHIP8_opcode* opcode) {
  CHIP8_dword nn = op & 0x00FF;
  CHIP8_dword n = op & 0x000F;

  switch (op & 0xF000) {
    case 0x0000:
      if (op == 0x00E0) {
        *opcode = CHIP8_opcode_cls;
      } else if (op == 0x00EE) {
        *opcode = CHIP8_opcode_ret;
      } else {
        return Chip8_InvalidOpcode;
      }
    break;
    case 0x1000: *opcode = CHIP8_opcode_jp1;
    break;
    case 0x2000: *opcode = CHIP8_opcode_call;
    break;
    case 0x3000: *opcode = CHIP8_opcode_se1;
    break;
    case 0x4000: *opcode = CHIP8_opcode_sne1;
    break;
    case 0x5000: if (n == 0x0) *opcode = CHIP8_opcode_se2;
    break;
    case 0x6000: *opcode = CHIP8_opcode_ld1;
    break;
    case 0x7000: *opcode = CHIP8_opcode_add1;
    break;
    case 0x8000:
      switch (n) {
        case 0x0: *opcode = CHIP8_opcode_ld2;
        break;
        case 0x1: *opcode = CHIP8_opcode_or;
        break;
        case 0x2: *opcode = CHIP8_opcode_and;
        break;
        case 0x3: *opcode = CHIP8_opcode_xor;
        break;
        case 0x4: *opcode = CHIP8_opcode_add2;
        break;
        case 0x5: *opcode = CHIP8_opcode_sub;
        break;
        case 0x6: *opcode = CHIP8_opcode_shr;
        break;
        case 0x7: *opcode = CHIP8_opcode_subn;
        break;
        case 0xE: *opcode = CHIP8_opcode_shl;
        break;
        default: return Chip8_InvalidOpcode;
      }
    break;
    case 0x9000: if (n == 0x0) *opcode = CHIP8_opcode_sne2;
    break;
    case 0xA000: *opcode = CHIP8_opcode_ld3;
    break;
    case 0xB000: *opcode = CHIP8_opcode_jp2;
    break;
    case 0xC000: *opcode = CHIP8_opcode_rnd;
    break;
    case 0xD000: *opcode = CHIP8_opcode_drw;
    break;
    case 0xE000:
      if (nn == 0x9E) *opcode = CHIP8_opcode_skp;
      else if (nn == 0xA1)  *opcode = CHIP8_opcode_sknp;
      else return Chip8_InvalidOpcode;
    break;
    case 0xF000:
      switch (nn) {
        case 0x07: *opcode = CHIP8_opcode_ld4;
        break;
        case 0x0A: *opcode = CHIP8_opcode_ld5;
        break;
        case 0x15: *opcode = CHIP8_opcode_ld6;
        break;
        case 0x18: *opcode = CHIP8_opcode_ld7;
        break;
        case 0x1E: *opcode = CHIP8_opcode_add3;
        break;
        case 0x29: *opcode = CHIP8_opcode_ld8;
        break;
        case 0x33: *opcode = CHIP8_opcode_ld9;
        break;
        case 0x55: *opcode = CHIP8_opcode_ld10;
        break;
        case 0x65: *opcode = CHIP8_opcode_ld11;
      }
    default: return Chip8_InvalidOpcode;
  }

  return CHIP8_Ok;
}

CHIP8_errors CHIP8_execute(CHIP8_chip8* chip8, CHIP8_dword op, CHIP8_opcode* opcode) {
  CHIP8_word vx = op & 0x0F00 >> 8;
  CHIP8_word vy = op & 0x00F0 >> 4;
  CHIP8_dword nnn = op & 0x0FFF;
  CHIP8_dword nn = op & 0x00FF;
  CHIP8_dword n = op & 0x000F;
  CHIP8_errors err = CHIP8_parse(op, opcode);

  switch (*opcode) {
    case CHIP8_opcode_cls: CHIP8_cls(chip8);
    break;
    case CHIP8_opcode_ret: err = CHIP8_ret(chip8);
    break;
    case CHIP8_opcode_jp1: err = CHIP8_jmp(chip8, nnn);
    break;
    case CHIP8_opcode_call:  err = CHIP8_call(chip8, nnn);
    break;
    case CHIP8_opcode_se1: if (chip8->r[vx] == nn) CHIP8_next(chip8);
    break;
    case CHIP8_opcode_sne1: if (chip8->r[vx] != nn) CHIP8_next(chip8);
    break;
    case CHIP8_opcode_se2: if (chip8->r[vx] == chip8->r[vy]) CHIP8_next(chip8);
    break;
    case CHIP8_opcode_ld1: chip8->r[vx] = nn;
    break;
    case CHIP8_opcode_add1:
      chip8->r[vx] += nn;
      chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[vx] + nn > 255;
    break;
    case CHIP8_opcode_ld2: chip8->r[vx] = chip8->r[vy];
    break;
    case CHIP8_opcode_or: chip8->r[vx] |= chip8->r[vy];
    break;
    case CHIP8_opcode_and: chip8->r[vx] &= chip8->r[vy];
    break;
    case CHIP8_opcode_xor:
      chip8->r[vx] ^= chip8->r[vy];
      chip8->r[CHIP8_FLAG_REGISTER] = ((chip8->r[vx] ^ chip8->r[vy]) & ~chip8->r[vx]) != 0;
    break;
    case CHIP8_opcode_add2:
      chip8->r[vx] += chip8->r[vy];
      chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[vx] + chip8->r[vy] > 255;
    break;
    case CHIP8_opcode_sub:
      chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[vy] > chip8->r[vx];
      chip8->r[vx] -= chip8->r[vy];
    break;
    case CHIP8_opcode_shr:
      chip8->r[vx] >>= 1;
      chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[vx] & 0b1;
    break;
    case CHIP8_opcode_subn:
      chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[vy] > chip8->r[vx] ? 1 : 0;
      chip8->r[vx] = chip8->r[vy] - chip8->r[vx];
    break;
    case CHIP8_opcode_shl:
      chip8->r[vx] <<= 1;
      chip8->r[CHIP8_FLAG_REGISTER] = chip8->r[vx] & 0x80;
    break;
    case CHIP8_opcode_sne2: if (chip8->r[vx] != chip8->r[vy]) CHIP8_next(chip8);
    break;
    case CHIP8_opcode_ld3: chip8->i = nnn;
    break;
    case CHIP8_opcode_jp2: chip8->pc = chip8->i + nnn > CHIP8_MEMORY_SIZE ? CHIP8_MEMORY_SIZE - 1 : chip8->i + nnn;
    break;
    case CHIP8_opcode_rnd: chip8->r[vx] = rand() & nn;
    break;
    case CHIP8_opcode_drw: err = CHIP8_draw(chip8, vx, vy, n);
    break;
    case CHIP8_opcode_skp: if (CHIP8_kpop(chip8) == chip8->r[vx]) CHIP8_next(chip8);
    break;
    case CHIP8_opcode_sknp: if (CHIP8_kpop(chip8) != chip8->r[vx]) CHIP8_next(chip8);
    break;
    case CHIP8_opcode_ld4: chip8->r[vx] = chip8->dt;
    break;
    case CHIP8_opcode_ld5: chip8->r[vx] = CHIP8_waitkey(chip8, vx);
    break;
    case CHIP8_opcode_ld6: chip8->dt = chip8->r[vx];
    break;
    case CHIP8_opcode_ld7: chip8->st = chip8->r[vx];
    break;
    case CHIP8_opcode_add3: chip8->i += chip8->i + chip8->r[vx];
    break;
    case CHIP8_opcode_ld8: err = CHIP8_getspraddr(chip8->r[vx], &chip8->i);
    break;
    case CHIP8_opcode_ld9: err = CHIP8_getspraddr(chip8->r[vx], &chip8->i);
      chip8->memory[chip8->i] = (chip8->r[vx] / 100) % 10; ;
      chip8->memory[chip8->i + 1] = (chip8->r[vx] / 10) % 10;
      chip8->memory[chip8->i + 2] = chip8->r[vx] % 10;
    break;
    case CHIP8_opcode_ld10:
      for (unsigned short i = 0; i < vx; i++) {
        chip8->memory[chip8->i + i] = chip8->r[i];
      }
    break;
    case CHIP8_opcode_ld11:
      for (unsigned short i = 0; i < vx; i++) {
        chip8->r[i] = chip8->memory[chip8->i + i];
      }
  }

  return err;
}


// Execute the instruction in memory in 1 cycle.
CHIP8_errors CHIP8_cycle(CHIP8_chip8* chip8) {
  if ((chip8->pc + 1) >= CHIP8_MEMORY_SIZE) {
    return CHIP8_MemoryAddressNotFound;
  }
  
  CHIP8_dword op = CHIP8_getop(chip8, chip8->pc);
  CHIP8_opcode opcode;
  CHIP8_errors err = CHIP8_execute(chip8, op, &opcode);

  if (chip8->dt) chip8->dt--;
  if (chip8->st) chip8->st--;
  if (
    opcode != CHIP8_opcode_ret && opcode != CHIP8_opcode_jp1 &&
    opcode != CHIP8_opcode_call && opcode != CHIP8_opcode_jp2
  ) CHIP8_next(chip8);

  return err;
}

// Load the ROM into memory.
CHIP8_errors CHIP8_loadrom(CHIP8_chip8* chip8, CHIP8_word* rom, CHIP8_size n) {
  if(n > CHIP8_MAX_ROM_SIZE) {
    return CHIP8_ROMTooBig;
  }

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
