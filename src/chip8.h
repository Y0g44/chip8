/*
 * Copyright (c) 2025 Yoga. All rights reserved.
 *
 * This work is licensed under the terms of the MIT license.  
 * For a copy, see <https://opensource.org/licenses/MIT>.
*/

#define CHIP8_STACK_SIZE 16
#define CHIP8_KEY_QUEUE_SIZE 32

typedef unsigned char CHIP8_RegisterAddress;
typedef unsigned int CHIP8_MemoryAddress;
typedef unsigned char CHIP8_VramAddress;
typedef unsigned char CHIP8_Byte;
typedef CHIP8_Byte CHIP8_Word;
typedef unsigned int CHIP8_Dword;

typedef enum CHIP8_Key {
  CHIP8_KeyNone = 0xFF,
  CHIP8_Key0    = 0x0,
  CHIP8_Key1    = 0x1,
  CHIP8_Key2    = 0x2,
  CHIP8_Key3    = 0x3,
  CHIP8_Key4    = 0x4,
  CHIP8_Key5    = 0x5,
  CHIP8_Key6    = 0x6,
  CHIP8_Key7    = 0x7,
  CHIP8_Key8    = 0x8,
  CHIP8_Key9    = 0x9,
  CHIP8_KeyA    = 0xA,
  CHIP8_KeyB    = 0xB,
  CHIP8_KeyC    = 0xC,
  CHIP8_KeyD    = 0xD,
  CHIP8_KeyE    = 0xE,
  CHIP8_KeyF    = 0xF,
} CHIP8_Key;

typedef enum CHIP8_Error {
  CHIP8_Ok,
  CHIP8_FileNotFound,
  CHIP8_ROMTooBig,
  CHIP8_MemoryAllocationFailed,
  CHIP8_MemoryAddressNotFound,
  CHIP8_InvalidArguments,
  CHIP8_StackOverflow,
  CHIP8_NoCall,
  Chip8_InvalidOpcode
} CHIP8_Error;

typedef enum CHIP8_Opcode {
  CHIP8_OpcodeUnknown,
  CHIP8_OpcodeCls,  // 00E0
  CHIP8_OpcodeRet,  // 00EE
  CHIP8_OpcodeJp1,  // 1nnn
  CHIP8_OpcodeCall, // 2nnn
  CHIP8_OpcodeSe1,  // 3xkk
  CHIP8_OpcodeSne1, // 4xkk
  CHIP8_OpcodeSe2,  // 5xy0
  CHIP8_OpcodeLd1,  // 6xkk
  CHIP8_OpcodeAdd1, // 7xkk
  CHIP8_OpcodeLd2,  // 8xy0
  CHIP8_OpcodeOr,   // 8xy1
  CHIP8_OpcodeAnd,  // 8xy2
  CHIP8_OpcodeXor,  // 8xy3
  CHIP8_OpcodeAdd2, // 8xy4
  CHIP8_OpcodeSub,  // 8xy5
  CHIP8_OpcodeShr,  // 8xy6
  CHIP8_OpcodeSubn, // 8xy7
  CHIP8_OpcodeShl,  // 8xyE
  CHIP8_OpcodeSne2, // 9xy0
  CHIP8_OpcodeLd3,  // Annn
  CHIP8_OpcodeJp2,  // Bnnn
  CHIP8_OpcodeRnd,  // Cxkk
  CHIP8_OpcodeDrw,  // Dxyn
  CHIP8_OpcodeSkp,  // Ex9E
  CHIP8_OpcodeSknp, // ExA1
  CHIP8_OpcodeLd4,  // Fx07
  CHIP8_OpcodeLd5,  // Fx0A
  CHIP8_OpcodeLd6,  // Fx15
  CHIP8_OpcodeLd7,  // Fx18
  CHIP8_OpcodeAdd3, // Fx1E
  CHIP8_OpcodeLd8,  // Fx29
  CHIP8_OpcodeLd9,  // Fx33
  CHIP8_OpcodeLd10, // Fx55
  CHIP8_OpcodeLd11, // Fx65
} CHIP8_Opcode;

typedef struct CHIP8_Keyboard {
  CHIP8_Key keys[CHIP8_KEY_QUEUE_SIZE];
  unsigned short head;
  unsigned short tail;
} CHIP8_Keyboard;

typedef struct CHIP8_Chip8 {
  CHIP8_Word* memory;
  CHIP8_Word* vram;
  CHIP8_Word dt, st;
  CHIP8_MemoryAddress pc;
  CHIP8_MemoryAddress i;
  CHIP8_Dword sp, kp;
  CHIP8_Word r[16];
  
  CHIP8_MemoryAddress stack[CHIP8_STACK_SIZE];
  CHIP8_Word wait;
  CHIP8_Keyboard* keyboard;
} CHIP8_Chip8;

void CHIP8_init(CHIP8_Chip8* chip8);
void CHIP8_deinit(CHIP8_Chip8* chip8);
void CHIP8_next(CHIP8_Chip8* chip8);
CHIP8_Dword CHIP8_getop(CHIP8_Chip8* chip8, CHIP8_MemoryAddress addr);
CHIP8_Key CHIP8_kpeek(CHIP8_Keyboard* keyboard);
CHIP8_Key CHIP8_kpop(CHIP8_Keyboard* keyboard);
void CHIP8_kpush(CHIP8_Keyboard* keyboard, CHIP8_Key key);
void CHIP8_cls(CHIP8_Chip8* chip8);
CHIP8_Error CHIP8_call(CHIP8_Chip8* chip8, CHIP8_MemoryAddress addr);
CHIP8_Error CHIP8_ret(CHIP8_Chip8* chip8);
CHIP8_Error CHIP8_getspritechar(CHIP8_Word ch, CHIP8_MemoryAddress* addr);
unsigned short CHIP8_getpixel(CHIP8_Chip8* chip8, CHIP8_Byte x, CHIP8_Byte y);
void CHIP8_draw(CHIP8_Chip8* chip8, CHIP8_Byte x, CHIP8_Byte y, CHIP8_Word sprite);
CHIP8_Error CHIP8_parse(CHIP8_Dword op, CHIP8_Opcode* opcode);
CHIP8_Error CHIP8_execute(CHIP8_Chip8* chip8, CHIP8_Dword op, CHIP8_Opcode* opcode);
CHIP8_Error CHIP8_cycle(CHIP8_Chip8* chip8);
CHIP8_Error CHIP8_loadrom(CHIP8_Chip8* chip8, CHIP8_Word* rom, unsigned short n);
CHIP8_Error CHIP8_loadfile(CHIP8_Chip8* chip8, const char* path);