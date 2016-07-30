#pragma once

#include "defines.h"
#ifdef _WIN32
#include <Windows.h>
#endif



#ifdef __cplusplus
extern "C" {
#endif


struct VM {
	BYTE running : 1;
	unsigned short PC;
	unsigned short I;

	unsigned short stack[16];
	unsigned char V[16];
	unsigned char SP : 4;
	unsigned char memory[4096];
	unsigned char display[2048];

	unsigned char delayTimer;
	unsigned char soundTimer;

	unsigned char keys[16];

};
typedef struct VM VM;

VM* VM_Create();
void VM_LoadFont(VM *vm, unsigned char *font);
void VM_LoadRom(VM *vm, unsigned char* rom);
int VM_Update(VM *vm);

int VM_Destroy(VM*);

#ifdef __cplusplus
}
#endif