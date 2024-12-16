#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "md.h"
#include "cartridge.h"

/* A MASTER FUNCTION TO LOAD THE CARTRIDGE INFORMATION */
/* THIS IS DONE BY SEEKING INTO THE CONTENTS OF THE HEADER */
/* THEN EVALUATING SUCH */

/* ALLOCATING THE APPROPRIATE SIZE FOR EACH */

int MD_CART_LOAD(char* FILENAME, MD_CART* CART) 
{
    FILE* ROM;
    UNK SIZE;

    printf("Opening ROM file: %s\n", FILENAME);

    ROM = fopen(FILENAME, "rb");
    if (ROM == NULL) 
    {
        printf("Failed to open ROM file: %s\n", FILENAME);
        return -1;
    }

    fseek(ROM, 0, SEEK_END);
    SIZE = ftell(ROM);
    rewind(ROM);

    CART->ROM_SIZE = SIZE;
    CART->ROM_DATA = (unsigned char*)malloc(SIZE);

    if (CART->ROM_DATA == NULL) 
    {
        printf("Memory Allocation failed for ROM\n");
        fclose(ROM);
        return -1;
    }

    fread(CART->ROM_DATA, 1, SIZE, ROM);

    // EXTRA DEBUG INFORMATION PROVIDED ON COMMAND
    // SUCH AS THE CHEKCSUM IN HEX

    GET_CHECKSUM(CART->ROM_DATA, SIZE, FILENAME);

    RENDER_INIT();

    fclose(ROM);
    printf("ROM Loaded Successfully. Size: %lu bytes\n", SIZE);

    return 0;
}

void INIT_CHIPS(struct CPU_68K* CPU) 
{
    memset(CPU, 0, sizeof(struct CPU_68K));
    printf("Chipset Structs initialised.\n");
}

int main(int argc, char* argv[]) 
{
    if (argc < 2) 
    {
        fprintf(stderr, "Usage: %s <ROM_PATH>\n", argv[0]);
        return -1;
    }

    char* ROM_PATH = argv[1];

    int QUIT = 0;
    SDL_Window* WINDOW = SDL_CreateWindow("HARRY CLARK - MDEMU", 0, 0, 320, 240, SDL_WINDOW_SHOWN);
    SDL_Renderer* RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Event EV;

    if (WINDOW == NULL || RENDERER == NULL) 
    {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    /* THIS IS VERY CUT AND DRY, VERY DIRTY IMPLEMENTATION */
    /* THOUGH, THIS IS THE BEST I CAN DO RIGHT NOW */

    MD* CONSOLE = (MD*)malloc(sizeof(MD));
    memset(CONSOLE, 0, sizeof(MD));

    CPU_68K* CPU = (CPU_68K*)malloc(sizeof(CPU_68K));
    memset(CPU, 0, sizeof(CPU_68K));

    CONSOLE->MD_CART = (MD_CART*)malloc(sizeof(MD_CART));
    memset(CONSOLE->MD_CART, 0, sizeof(MD_CART));

    INIT_CHIPS(CPU);

    if (MD_CART_LOAD((char*)ROM_PATH, CONSOLE->MD_CART) != 0) 
    {
        printf("Failed to load ROM from: %s\n", ROM_PATH);
        free(CONSOLE->MD_CART);
        free(CPU);
        free(CONSOLE);
        return -1;
    }

    while (!QUIT)
    {
        while (SDL_PollEvent(&EV)) 
        {
            if (EV.type == SDL_QUIT) 
            {
                QUIT = 1;
            }
        }

        SDL_RenderClear(RENDERER);
        SDL_RenderPresent(RENDERER);
    }

    if (CONSOLE->MD_CART->ROM_DATA != NULL) 
    {
        free(CONSOLE->MD_CART->ROM_DATA);
    }
    
    free(CONSOLE->MD_CART);
    free(CPU);
    free(CONSOLE);

    SDL_DestroyRenderer(RENDERER);
    SDL_DestroyWindow(WINDOW);
    SDL_Quit();

    return 0;
}
