#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <SDL.h>


#include "vm.h"

void RenderDisplay(VM* vm, SDL_Texture *texture, unsigned char* pixels)
{
	SDL_Rect rect = { 0, 0, 64, 32 };
	int pitch = rect.w * 4;


	SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
	SDL_LockTexture(texture, &rect, &pixels, &pitch);

	unsigned char r = 0;
	for (int i = 0; i < 64 * 32; i++)
	{
		if (vm->display[i])
			r = 255;
		else
			r = 0;
		pixels[i * 4] = 255;
		pixels[i * 4 + 1] = r;
		pixels[i * 4 + 2] = r;
		pixels[i * 4 + 3] = r;
	}
	SDL_UnlockTexture(texture);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Provide a relative file path\n");
		return -1;
	}

	int running = 1;
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window *sdlWin = SDL_CreateWindow("",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 256, 128, SDL_WINDOW_SHOWN);
	SDL_Renderer *render = SDL_CreateRenderer(sdlWin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	SDL_Texture *text = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

	VM *vm = VM_Create(render);

	unsigned char *pixels = malloc(4 * 32 * 64);


	errno = 0;

	FILE *fh;
	#ifdef _WIN32
	fopen_s(&fh, argv[1], "rb");
	#else
	fh = fopen(argv[1], "rb");
	#endif
	if (fh == NULL)
	{
		printf("ERROR: %d\n", errno);
	}
	//fopen_s(&fh, "p.c8", "r");


	unsigned char *data = calloc(3586, sizeof(unsigned char));
	unsigned int dataSize = sizeof(unsigned char) * 3586;

	if (data == NULL)
	{
		printf("ERROR");
		return -1;
	}

	//get file size
	fseek(fh, 0, SEEK_END);
	long fsize = ftell(fh);
	fseek(fh, 0, SEEK_SET);  //same as rewind(f);

	#ifdef _WIN32
	fread_s(data, dataSize, fsize, 1, fh);
	#else
	fread(data, fsize, 1, fh);
	#endif
	fclose(fh);

	VM_LoadRom(vm, data);

	free(data);

	vm->running = 1;

	while (running)
	{
		Uint32 ticks = SDL_GetTicks();

		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent) != 0)
		{
			if (sdlEvent.type == SDL_KEYUP) {
				switch (sdlEvent.key.keysym.sym)
				{
					case SDLK_ESCAPE:
					case SDL_QUIT:
						running = 0;
						break;

					case SDLK_1: vm->keys[0x1] = 0; break;
					case SDLK_2: vm->keys[0x2] = 0; break;
					case SDLK_3: vm->keys[0x3] = 0; break;
					case SDLK_4: vm->keys[0xC] = 0; break;

					case SDLK_q: vm->keys[0x4] = 0; break;
					case SDLK_w: vm->keys[0x5] = 0; break;
					case SDLK_e: vm->keys[0x6] = 0; break;
					case SDLK_r: vm->keys[0xD] = 0; break;

					case SDLK_a: vm->keys[0x7] = 0; break;
					case SDLK_s: vm->keys[0x8] = 0; break;
					case SDLK_d: vm->keys[0x9] = 0; break;
					case SDLK_f: vm->keys[0xE] = 0; break;

					case SDLK_z: vm->keys[0xA] = 0; break;
					case SDLK_x: vm->keys[0x0] = 0; break;
					case SDLK_c: vm->keys[0xB] = 0; break;
					case SDLK_v: vm->keys[0xF] = 0; break;

				}
			}

			if (sdlEvent.type == SDL_KEYDOWN) {
				switch (sdlEvent.key.keysym.sym)
				{
					case SDLK_1: vm->keys[0x1] = 1; break;
					case SDLK_2: vm->keys[0x2] = 1; break;
					case SDLK_3: vm->keys[0x3] = 1; break;
					case SDLK_4: vm->keys[0xC] = 1; break;

					case SDLK_q: vm->keys[0x4] = 1; break;
					case SDLK_w: vm->keys[0x5] = 1; break;
					case SDLK_e: vm->keys[0x6] = 1; break;
					case SDLK_r: vm->keys[0xD] = 1; break;

					case SDLK_a: vm->keys[0x7] = 1; break;
					case SDLK_s: vm->keys[0x8] = 1; break;
					case SDLK_d: vm->keys[0x9] = 1; break;
					case SDLK_f: vm->keys[0xE] = 1; break;

					case SDLK_z: vm->keys[0xA] = 1; break;
					case SDLK_x: vm->keys[0x0] = 1; break;
					case SDLK_c: vm->keys[0xB] = 1; break;
					case SDLK_v: vm->keys[0xF] = 1; break;

				}
			}

		}


		for (unsigned char r = 0; r < 15; r++)
			VM_Update(vm);

		RenderDisplay(vm, text, pixels);
		SDL_RenderCopy(render, text, NULL, NULL);
		SDL_RenderPresent(render);

		Uint32 deltaTicks = SDL_GetTicks();

		if ((deltaTicks - ticks) < 16) {
			SDL_Delay(16 - (deltaTicks - ticks ));
		}

	}


	VM_Destroy(vm);
	free(pixels);

	SDL_DestroyTexture(text);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(sdlWin);
	SDL_Quit();
	return 0;
}
