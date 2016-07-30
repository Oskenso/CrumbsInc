#include "vm.h"

const char fontdata[(5 * 16)] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0,
	0x20, 0x60, 0x20, 0x20, 0x70,
	0xF0, 0x10, 0xF0, 0x80, 0xF0,
	0xF0, 0x10, 0xF0, 0x10, 0xF0,
	0x90, 0x90, 0xF0, 0x10, 0x10,
	0xF0, 0x80, 0xF0, 0x10, 0xF0,
	0xF0, 0x80, 0xF0, 0x90, 0xF0,
	0xF0, 0x10, 0x20, 0x40, 0x40,
	0xF0, 0x90, 0xF0, 0x90, 0xF0,
	0xF0, 0x90, 0xF0, 0x10, 0xF0,
	0xF0, 0x90, 0xF0, 0x90, 0x90,
	0xE0, 0x90, 0xE0, 0x90, 0xE0,
	0xF0, 0x80, 0x80, 0x80, 0xF0,
	0xE0, 0x90, 0x90, 0x90, 0xE0,
	0xF0, 0x80, 0xF0, 0x80, 0xF0,
	0xF0, 0x80, 0xF0, 0x80, 0x80
};

VM* VM_Create()
{
	VM *vm = malloc(sizeof(VM));
	vm->PC = 0x200;
	vm->running = 0;
	vm->delayTimer = 0;
	vm->soundTimer = 0;
	vm->SP = 0;
	vm->I = 0;

	int i = 0;
	for (i = 0; i < 16; i++) {
		vm->V[i] = 0;
		vm->stack[i] = 0;
	}

	//vm->display = SDL_CreateTexture(rend, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 64, 32);

	for (i = 0; i < 2048; i++) {
		vm->display[i] = 0;
	}

	memcpy(vm->memory, fontdata, sizeof(fontdata));

	return vm;
}

void VM_LoadFont(VM *vm, unsigned char *font)
{
	memcpy(vm->memory, font, (16 * 5));
}
int VM_Destroy(VM *vm)
{
	if (vm)
		free(vm);

	return 0;
}

void VM_LoadRom(VM *vm, unsigned char* rom)
{
	memcpy(&vm->memory[0x200], rom, 3586);
}

unsigned char VM_SetPixel(VM *vm, int x, int y)
{
	//wrap around
	if (x > 64 - 1) while (x > 64 - 1) x -= 64;
	if (x < 0) while (x < 0) x += 64;
	if (y > 32 - 1) while (y > 32 - 1) y -= 32;
	if (y < 0) while (y < 0) y += 32;
	unsigned short pos = (unsigned short) x + (y * 64);

	vm->display[pos] = vm->display[pos] ^ 1;

	return !vm->display[pos];

}
int VM_Update(VM *vm)
{
	if (!vm->running) return 1;

	unsigned short opcode = vm->memory[vm->PC] << 8 | vm->memory[vm->PC + 1];
	//printf("Opcode: %04x\n", opcode);
	
	unsigned char x = (opcode & 0x0F00) >> 8;
	unsigned char y = (opcode & 0x00F0) >> 4;
	unsigned short nnn = (opcode & 0x0FFF);
	vm->PC += 2;

	switch (opcode & 0xF000) {
		case 0x0000:
			switch (opcode) {
				case 0x00E0:
					for (int i = 0; i < 2048; i++) {
						vm->display[i] = 0;
					}

					break;
				case 0x00EE:
					//->stac
					//printf("Stack[0x%04x] = %02x\n", vm->SP - 1, vm->stack[vm->SP - 1]);
					vm->PC = vm->stack[--vm->SP];
					break;
			}
			break;

		case 0x1000:
			vm->PC = nnn;
			break;

		case 0x2000:

			vm->stack[vm->SP++] = vm->PC;
			//printf("Stack[0x%04x] = %02x\n", vm->SP - 1, vm->stack[vm->SP - 1]);

			vm->PC = nnn;
			break;

		case 0x3000:
			if (vm->V[x] == (opcode & 0xFF))
				vm->PC += 2;
			break;

		case 0x4000:
			if (vm->V[x] != (opcode & 0xFF))
				vm->PC += 2;
			break;
		case 0x5000:
			if (vm->V[x] == vm->V[y])
				vm->PC += 2;
			break;

		case 0x6000:
			vm->V[x] = (opcode & 0xFF);
			break;

		case 0x7000:
			vm->V[x] += (opcode & 0xFF);
			break;

		case 0x8000:
			switch (opcode & 0x000F) {
				case 0x0000:
					vm->V[x] = vm->V[y];
					break;
				case 0x0001:
					vm->V[x] |= vm->V[y];
					break;

				case 0x0002:
					vm->V[x] &= vm->V[y];
					break;

				case 0x0003:
					vm->V[x] ^= vm->V[y];
					break;

				case 0x0004:
					vm->V[0xF] = ((vm->V[x] + vm->V[y]) > 255);
					vm->V[x] += vm->V[y];
					
					break;

				case 0x0005:
					vm->V[0xF] = +(vm->V[x] > vm->V[y]);
					vm->V[x] -= vm->V[y];
					break;

				case 0x0006:
					vm->V[0xF] = vm->V[x] & 1;
					vm->V[x] >>= 1;
					break;

				case 0x0007:
					vm->V[0x0F] = +(vm->V[y] > vm->V[x]);
					vm->V[x] = vm->V[y] - vm->V[x];
					break;

				case 0x000E:
					vm->V[0xF] = +(vm->V[x] & 0x80);
					vm->V[x] = vm->V[x] << 1;
					break;

			}
			break;

		case 0x9000:
			if (vm->V[x] != vm->V[y])
				vm->PC += 2;
			break;

		case 0xA000:
			vm->I = nnn;
			break;

		case 0xB000:
			vm->PC = nnn + vm->V[0];
			break;

		case 0xC000:
			vm->V[x] = (rand() % 256) & (opcode & 0xFF);
			break;

			//WARNING: a nightmare :[
		case 0xD000:
			vm->V[0xF] = 0;
			unsigned char height = (opcode & 0x000F);

			for (int yy = 0; yy < height; yy++) {
				unsigned char spr = vm->memory[vm->I + yy];
				for (int xx = 0; xx < 8; xx++) {
					if ((spr & 0x80) > 0) {
						if (VM_SetPixel(vm, vm->V[x] + xx, vm->V[y] + yy))
							vm->V[0xF] = 1;

					}
					spr <<= 1;
				}
			}


			break;
		case 0xE000:
			switch (opcode & 0x00FF) {
				case 0x009E:
					if (vm->keys[vm->V[x]])
						vm->PC += 2;
					break;

				case 0x00A1:
					if (vm->keys[vm->V[x]] == 0)
						vm->PC += 2;
					break;
			}
			break;

		case 0xF000:
			switch (opcode & 0x00FF) {
				case 0x0007:
					vm->V[x] = vm->delayTimer;
					break;

				case 0x000A:
					vm->PC -= 2;
					for (unsigned char i = 0; i < sizeof(vm->keys); i++) {
						if (vm->keys[i]) {
							vm->V[x] = i;
							vm->PC += 2;
						}
					}
					break;

				case 0x0015:
					vm->delayTimer = vm->V[x];
					break;

				case 0x0018:
					vm->soundTimer = vm->V[x];
					break;

				case 0x001E:
					vm->I = (vm->I + vm->V[x]) & 0xFFFF;
					break;
				case 0x0029:
					vm->I = vm->V[x] * 5;
					break;

				case 0x0033:
					vm->memory[vm->I] = vm->V[x] / 100;
					vm->memory[vm->I + 1] = vm->V[x] % 100 / 10;
					vm->memory[vm->I + 2] = vm->V[x] % 10;
					break;

				case 0x0055:
					for (int i = 0; i < vm->V[x]; i++)
						vm->memory[vm->I + i] = vm->V[i];
					break;

				case 0x0065:
					for (int i = 0; i < vm->V[x]; i++)
						vm->V[x] = vm->memory[vm->I + i];

					break;
			}

			break;

			
	}
	if (vm->delayTimer > 0)
		vm->delayTimer--;
	if (vm->soundTimer > 0)
		vm->soundTimer--;
	//if (vm->soundTimer == 1)
		//printf("beep");
	return 0;
}