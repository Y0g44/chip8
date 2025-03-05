/*
 * Copyright (c) 2025 Yoga. All rights reserved.
 *
 * This work is licensed under the terms of the MIT license.  
 * For a copy, see <https://opensource.org/licenses/MIT>.
*/

#include <stdio.h>
#include <string.h>
#include "chip8.h"

#define CHIP8_INSTRUCTION_START_ADDRESS 512
#define CHIP8_VRAM_SIZE 256
#define CHIP8_MEMORY_SIZE 4096

#define doesXORMakeCollision(a, b) (((a) & ~(b)) != 0)
#define max(a, b) a > b ? a : b
#define min(a, b) a < b ? a : b

// Sprite characters that will be stored in memory.
static const CHIP8_Word sprites[80] = {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
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

static unsigned short fillOneBit(unsigned short n) {
  switch (n) {
    case 7: return 0b1111111;
    case 6: return 0b111111;
    case 5: return 0b11111;
    case 4: return 0b1111;
    case 3: return 0b111;
    case 2: return 0b11;
    case 1: return 0b1;
  }
  return 0b11111111;
}

void CHIP8_init(CHIP8_Chip8* chip8) {
  chip8->wait = 0;
  chip8->pc = CHIP8_INSTRUCTION_START_ADDRESS;
  chip8->dt = 0;
  chip8->st = 0;
  chip8->i = 0;
  chip8->sp = 0;
  chip8->kp = 0;

  chip8->memory = malloc(sizeof(CHIP8_Word) * CHIP8_MEMORY_SIZE);
  chip8->vram = malloc(sizeof(CHIP8_Word) * CHIP8_VRAM_SIZE);
  memset(chip8->memory, 0, sizeof(CHIP8_Word) * CHIP8_MEMORY_SIZE);
  memset(chip8->vram, 0, sizeof(CHIP8_Word) * CHIP8_VRAM_SIZE);
  memset(chip8->r, 0, sizeof(CHIP8_Word) * 16);
  memset(chip8->stack, 0, sizeof(CHIP8_MemoryAddress) * CHIP8_STACK_SIZE);
  memcpy(chip8->memory, sprites, 80);
}

void CHIP8_deinit(CHIP8_Chip8* chip8) {
  chip8->wait = 0;
  chip8->pc = CHIP8_INSTRUCTION_START_ADDRESS;
  chip8->dt = 0;
  chip8->st = 0;
  chip8->i = 0;
  chip8->sp = 0;
  chip8->kp = 0;
  free(chip8->memory);
  free(chip8->vram);
  memset(chip8->r, 0, sizeof(CHIP8_Word) * 16);
  memset(chip8->stack, 0, sizeof(CHIP8_MemoryAddress) * CHIP8_STACK_SIZE);
}

void CHIP8_next(CHIP8_Chip8* chip8) {
  if (!chip8->wait) {
    chip8->pc += 2;
  }
}

CHIP8_Dword CHIP8_getop(CHIP8_Chip8* chip8, CHIP8_MemoryAddress addr) {
  return (chip8->memory[addr] << 8) | chip8->memory[addr + 1];
}

CHIP8_Key CHIP8_kpeek(CHIP8_Chip8* chip8) {
  return chip8->kp == 0 || chip8->kp > CHIP8_KEY_STACK_SIZE
    ? CHIP8_KeyNone
    : chip8->keys[chip8->kp];
}

CHIP8_Key CHIP8_kpop(CHIP8_Chip8* chip8) {
  CHIP8_Key head = CHIP8_kpeek(chip8);
  if (head != CHIP8_KeyNone) {
    chip8->kp--;
  }

  return head;
}

void CHIP8_kpush(CHIP8_Chip8* chip8, CHIP8_Key key) {
  if (key != CHIP8_KeyNone) {
    chip8->keys[chip8->kp > CHIP8_KEY_STACK_SIZE ? chip8->kp : ++chip8->kp] = key;
  }
}

void CHIP8_cls(CHIP8_Chip8* chip8) {
  memset(chip8->vram, 0, sizeof(CHIP8_Word) * CHIP8_VRAM_SIZE);
}

CHIP8_Error CHIP8_call(CHIP8_Chip8* chip8, CHIP8_MemoryAddress addr) {
  if (addr >= CHIP8_MEMORY_SIZE) {
    return CHIP8_MemoryAddressNotFound;
  }
  
  CHIP8_Error err = CHIP8_Ok;
  if (chip8->sp >= CHIP8_STACK_SIZE) {
    chip8->stack[chip8->sp - 1] = chip8->pc;
    err = CHIP8_StackOverflow;
  } else {
    chip8->stack[chip8->sp++] = chip8->pc;
  }

  chip8->pc = addr;
  return err;
}

CHIP8_Error CHIP8_ret(CHIP8_Chip8* chip8) {
  if (chip8->sp == 0) {
    return CHIP8_NoCall;
  }
  
  chip8->pc = chip8->stack[--chip8->sp];
  return CHIP8_Ok;
}

CHIP8_Error CHIP8_getspritechar(CHIP8_Word ch, CHIP8_MemoryAddress* addr) {
  if (ch > 15) {
    return CHIP8_InvalidArguments;
  }

  *addr = ch * 5;
  return CHIP8_Ok;
}

unsigned short CHIP8_getpixel(CHIP8_Chip8* chip8, CHIP8_Byte x, CHIP8_Byte y) {
  x = x % 64;
  y = y % 32;

  CHIP8_VramAddress i = ((x / 8) + (y * 8)) % CHIP8_VRAM_SIZE;
  CHIP8_VramAddress n = 7 - (x % 8);
  
  return (chip8->vram[i] & (1 << n)) >> n;
}

void CHIP8_draw(CHIP8_Chip8* chip8, CHIP8_Byte x, CHIP8_Byte y, CHIP8_Word sprite) {
  x = x % 64;
  y = y % 32;

  CHIP8_VramAddress i = ((x / 8) + (y * 8)) % CHIP8_VRAM_SIZE;
  CHIP8_VramAddress n1 = x % 8;
  CHIP8_VramAddress n2 = 8 - n1;

  CHIP8_Word b1 = (sprite & (fillOneBit(n2) << n1)) >> n1;
  CHIP8_Word b2 = (sprite & fillOneBit(n1)) << n2;
  
  CHIP8_VramAddress i1 = i;
  CHIP8_VramAddress i2 = (i % 8) == 7 ? i - 8 : i + 1;
  chip8->vram[i1] ^= b1;
  chip8->vram[i2] ^= b2;
  chip8->r[15] = doesXORMakeCollision(chip8->vram[i1] ^ b1, chip8->vram[i1]) ||
    doesXORMakeCollision(chip8->vram[i2] ^ b2, chip8->vram[i2]);
}

CHIP8_Error CHIP8_parse(CHIP8_Dword op, CHIP8_Opcode* opcode) {
  CHIP8_Dword nn = op & 0x00FF;
  CHIP8_Dword n = op & 0x000F;

  switch (op & 0xF000) {
    case 0x0000:
      if (op == 0x00E0) {
        *opcode = CHIP8_OpcodeCls;
      } else if (op == 0x00EE) {
        *opcode = CHIP8_OpcodeRet;
      } else {
        return Chip8_InvalidOpcode;
      }
    break;
    case 0x1000: *opcode = CHIP8_OpcodeJp1;
    break;
    case 0x2000: *opcode = CHIP8_OpcodeCall;
    break;
    case 0x3000: *opcode = CHIP8_OpcodeSe1;
    break;
    case 0x4000: *opcode = CHIP8_OpcodeSne1;
    break;
    case 0x5000: if (n == 0x0) *opcode = CHIP8_OpcodeSe2;
    break;
    case 0x6000: *opcode = CHIP8_OpcodeLd1;
    break;
    case 0x7000: *opcode = CHIP8_OpcodeAdd1;
    break;
    case 0x8000:
      switch (n) {
        case 0x0: *opcode = CHIP8_OpcodeLd2;
        break;
        case 0x1: *opcode = CHIP8_OpcodeOr;
        break;
        case 0x2: *opcode = CHIP8_OpcodeAnd;
        break;
        case 0x3: *opcode = CHIP8_OpcodeXor;
        break;
        case 0x4: *opcode = CHIP8_OpcodeAdd2;
        break;
        case 0x5: *opcode = CHIP8_OpcodeSub;
        break;
        case 0x6: *opcode = CHIP8_OpcodeShr;
        break;
        case 0x7: *opcode = CHIP8_OpcodeSubn;
        break;
        case 0xE: *opcode = CHIP8_OpcodeShl;
        break;
        default: return Chip8_InvalidOpcode;
      }
    break;
    case 0x9000: if (n == 0x0) *opcode = CHIP8_OpcodeSne2;
    break;
    case 0xA000: *opcode = CHIP8_OpcodeLd3;
    break;
    case 0xB000: *opcode = CHIP8_OpcodeJp2;
    break;
    case 0xC000: *opcode = CHIP8_OpcodeRnd;
    break;
    case 0xD000: *opcode = CHIP8_OpcodeDrw;
    break;
    case 0xE000:
      if (nn == 0x9E) *opcode = CHIP8_OpcodeSkp;
      else if (nn == 0xA1)  *opcode = CHIP8_OpcodeSknp;
      else return Chip8_InvalidOpcode;
    break;
    case 0xF000:
      switch (nn) {
        case 0x07: *opcode = CHIP8_OpcodeLd4;
        break;
        case 0x0A: *opcode = CHIP8_OpcodeLd5;
        break;
        case 0x15: *opcode = CHIP8_OpcodeLd6;
        break;
        case 0x18: *opcode = CHIP8_OpcodeLd7;
        break;
        case 0x1E: *opcode = CHIP8_OpcodeAdd3;
        break;
        case 0x29: *opcode = CHIP8_OpcodeLd8;
        break;
        case 0x33: *opcode = CHIP8_OpcodeLd9;
        break;
        case 0x55: *opcode = CHIP8_OpcodeLd10;
        break;
        case 0x65: *opcode = CHIP8_OpcodeLd11;
      }
    default: return Chip8_InvalidOpcode;
  }

  return CHIP8_Ok;
}

CHIP8_Error CHIP8_execute(CHIP8_Chip8* chip8, CHIP8_Dword op, CHIP8_Opcode* opcode) {
  CHIP8_Word vx = (op & 0x0F00) >> 8;
  CHIP8_Word vy = (op & 0x00F0) >> 4;
  CHIP8_Dword nnn = op & 0x0FFF;
  CHIP8_Dword nn = op & 0x00FF;
  CHIP8_Dword n = op & 0x000F;
  CHIP8_Error err = CHIP8_parse(op, opcode);

  switch (*opcode) {
    case CHIP8_OpcodeCls: CHIP8_cls(chip8);
    break;
    case CHIP8_OpcodeRet: err = CHIP8_ret(chip8);
    break;
    case CHIP8_OpcodeJp1: chip8->pc = nnn;
    break;
    case CHIP8_OpcodeCall: err = CHIP8_call(chip8, nnn);
    break;
    case CHIP8_OpcodeSe1: if (chip8->r[vx] == nn) CHIP8_next(chip8);
    break;
    case CHIP8_OpcodeSne1: if (chip8->r[vx] != nn) CHIP8_next(chip8);
    break;
    case CHIP8_OpcodeSe2: if (chip8->r[vx] == chip8->r[vy]) CHIP8_next(chip8);
    break;
    case CHIP8_OpcodeLd1: chip8->r[vx] = nn;
    break;
    case CHIP8_OpcodeAdd1: chip8->r[vx] += nn;
    break;
    case CHIP8_OpcodeLd2: chip8->r[vx] = chip8->r[vy];
    break;
    case CHIP8_OpcodeOr: chip8->r[vx] |= chip8->r[vy];
    break;
    case CHIP8_OpcodeAnd: chip8->r[vx] &= chip8->r[vy];
    break;
    case CHIP8_OpcodeXor:
      chip8->r[vx] ^= chip8->r[vy];
      chip8->r[15] = doesXORMakeCollision(chip8->r[vx] ^ chip8->r[vy], chip8->r[vx]);
    break;
    case CHIP8_OpcodeAdd2:
      chip8->r[15] = (chip8->r[vx] + chip8->r[vy]) > 255;
      chip8->r[vx] += chip8->r[vy];
    break;
    case CHIP8_OpcodeSub:
      chip8->r[15] = chip8->r[vy] > chip8->r[vx];
      chip8->r[vx] -= chip8->r[vy];
    break;
    case CHIP8_OpcodeShr:
      chip8->r[vx] >>= 1;
      chip8->r[15] = chip8->r[vx] & 1;
    break;
    case CHIP8_OpcodeSubn:
      chip8->r[15] = chip8->r[vx] > chip8->r[vy];
      chip8->r[vx] = chip8->r[vy] - chip8->r[vx];
    break;
    case CHIP8_OpcodeShl:
      chip8->r[vx] <<= 1;
      chip8->r[15] = chip8->r[vx] & 0x80;
    break;
    case CHIP8_OpcodeSne2: if (chip8->r[vx] != chip8->r[vy]) CHIP8_next(chip8);
    break;
    case CHIP8_OpcodeLd3: chip8->i = nnn;
    break;
    case CHIP8_OpcodeJp2: chip8->pc = min(chip8->r[0] + nnn, CHIP8_MEMORY_SIZE - 1);
    break;
    case CHIP8_OpcodeRnd: chip8->r[vx] = rand() & nn;
    break;
    case CHIP8_OpcodeDrw:
      CHIP8_VramAddress x = chip8->r[vx];
      CHIP8_VramAddress y = chip8->r[vy];

      for (CHIP8_Word i = 0; i < n; i++) {
        CHIP8_MemoryAddress addr = min(chip8->i + i, CHIP8_MEMORY_SIZE - 1);
        CHIP8_draw(chip8, x, y + i, chip8->memory[addr]);
      }
    break;
    case CHIP8_OpcodeSkp: if (CHIP8_kpop(chip8) == chip8->r[vx]) CHIP8_next(chip8);
    break;
    case CHIP8_OpcodeSknp: if (CHIP8_kpop(chip8) != chip8->r[vx]) CHIP8_next(chip8);
    break;
    case CHIP8_OpcodeLd4: chip8->r[vx] = chip8->dt;
    break;
    case CHIP8_OpcodeLd5:
      CHIP8_Key pressedKey = CHIP8_kpop(chip8);
      chip8->wait = pressedKey == CHIP8_KeyNone;
      if (pressedKey != CHIP8_KeyNone) {
        chip8->r[vx] = pressedKey;
      }
    break;
    case CHIP8_OpcodeLd6: chip8->dt = chip8->r[vx];
    break;
    case CHIP8_OpcodeLd7: chip8->st = chip8->r[vx];
    break;
    case CHIP8_OpcodeAdd3: chip8->i += chip8->i + chip8->r[vx];
    break;
    case CHIP8_OpcodeLd8: err = CHIP8_getspritechar(chip8->r[vx], &chip8->i);
    break;
    case CHIP8_OpcodeLd9:
      chip8->memory[chip8->i] = chip8->r[vx] / 100;
      chip8->memory[chip8->i + 1] = (chip8->r[vx] % 100) / 10;
      chip8->memory[chip8->i + 2] = chip8->r[vx] % 10;
    break;
    case CHIP8_OpcodeLd10: memcpy(chip8->memory + chip8->i, chip8->r, vx + 1);
    break;
    case CHIP8_OpcodeLd11: memcpy(chip8->r, chip8->memory + chip8->i, vx + 1);
  }

  return err;
}

CHIP8_Error CHIP8_cycle(CHIP8_Chip8* chip8) {
  if ((chip8->pc + 1) >= CHIP8_MEMORY_SIZE) {
    return CHIP8_MemoryAddressNotFound;
  }
  
  CHIP8_Dword op = CHIP8_getop(chip8, chip8->pc);
  CHIP8_Opcode opcode = CHIP8_OpcodeUnknown;
  CHIP8_Error err = CHIP8_execute(chip8, op, &opcode);

  if (chip8->dt) chip8->dt--;
  if (chip8->st) chip8->st--;
  if (
    opcode != CHIP8_OpcodeJp1 && opcode != CHIP8_OpcodeCall &&
    opcode != CHIP8_OpcodeJp2
  ) CHIP8_next(chip8);

  return err;
}

CHIP8_Error CHIP8_loadrom(CHIP8_Chip8* chip8, CHIP8_Word* rom, unsigned short n) {
  if(n > (CHIP8_MEMORY_SIZE - CHIP8_INSTRUCTION_START_ADDRESS)) {
    return CHIP8_ROMTooBig;
  }

  memcpy(chip8->memory + CHIP8_INSTRUCTION_START_ADDRESS, rom, n);
  return CHIP8_Ok;
}

CHIP8_Error CHIP8_loadfile(CHIP8_Chip8* chip8, const char* path) {
  FILE* fp = fopen(path, "r");
  if (!fp) {
    return CHIP8_FileNotFound;
  }

  fseek(fp, 0, SEEK_END);
  unsigned long romSize = ftell(fp);
  if(romSize > (CHIP8_MEMORY_SIZE - CHIP8_INSTRUCTION_START_ADDRESS)) {
    return CHIP8_ROMTooBig;
  }

  char* rom = malloc(sizeof(CHIP8_Word) * romSize);
  if (!rom) {
    return CHIP8_MemoryAllocationFailed;
  }

  fseek(fp, 0, SEEK_SET);
  fread(rom, sizeof(CHIP8_Word), romSize, fp);
  fclose(fp);
  memcpy(chip8->memory + CHIP8_INSTRUCTION_START_ADDRESS, rom, romSize);
  free(rom);

  return CHIP8_Ok;
}
