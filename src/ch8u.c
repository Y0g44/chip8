/*
 * Copyright (c) 2025 Yoga. All rights reserved.
 *
 * This work is licensed under the terms of the MIT license.  
 * For a copy, see <https://opensource.org/licenses/MIT>.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"

int main() {
  CHIP8_chip8 chip8;
  CHIP8_init(&chip8);
  chip8.r[0] = 12;
  CHIP8_draw(&chip8, 0, 1, 1);

  // printf("Stack: %d\n", chip8.stack[0]);
  // CHIP8_jmp(&chip8, 0x502);
  // printf("%d\n", chip8.pc);

  // srand(time(NULL));
  // CHIP8_loadrom(&chip8, rom, 3);
  // while (1) {
  //   printf("\e[1;1H\e[2J");
  //   printf("PC: %d      ", chip8.pc);
  //   printf("Program: %x\n", CHIP8_getop(&chip8, chip8.pc));
  //   printf("Register: \n");
  //   CHIP8_kpush(&chip8, CHIP8_key_2);
  //   for(unsigned short i = 0; i <= 0xF; i++) {
  //     printf("V%x = %d, ", i, chip8.r[i]);
  //   }
  //   printf("\nI = %d, ", chip8.i);
  //   printf("Dt = %d,  ", chip8.dt);
  //   printf("St = %d, ", chip8.st);
  //   printf("Wait = %d\n", chip8.wait);
  //   printf("Stack = ");
  //   for(unsigned short i = 0; i < chip8.sp; i++) {
  //     printf("%d -> ", chip8.stack[i]);
  //   }
  //   printf("\n");
  //   printf("Memory = ");
  //   for(unsigned short i = CHIP8_START_LOCATION; i <= (CHIP8_START_LOCATION + 32); i++) {
  //     printf("%d, ", chip8.memory[i]);
  //   }
  //   printf("\n");
    
  //   CHIP8_cycle(&chip8);
  //   for (unsigned short i = 0; i < CHIP8_VRAM_SIZE; i++) {
  //     if ((i % 8) == 0 && i != 0) {
  //       printf("\n");
  //     }

  //     for (unsigned short j = 8; j > 0; j--) {
  //       printf("%d", (chip8.vram[i] >> (j - 1)) & 0b1);
  //     }
  //     printf(" ");
  //   }
  //   usleep(500 * 1000);
  // }
  return 0;
}