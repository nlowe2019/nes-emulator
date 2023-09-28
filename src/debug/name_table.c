#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdlib.h>
#include <stdint.h>

#include "name_table.h"
#include "../memory/mem.h"
#include "../processing/palette.h"

uint32_t n_table_0[256*256];
uint32_t n_table_1[256*256];
uint32_t n_table_2[256*256];
uint32_t n_table_3[256*256];

SDL_Rect n_table_0_rect = { // top left
    214*2+256*2 + 128,
    0,
    256,
    256
};
SDL_Rect n_table_1_rect = { // top right
    214*2+256*2+ 128 +256 ,
    0,
    256,
    256
};
SDL_Rect n_table_2_rect = { // bottom left
    512 + 214 + 214 + 128,
    256,
    256,
    256
};
SDL_Rect n_table_3_rect = { // bottom right
    512 + 214 + 214 + 128 +256,
    256,
    256,
    256
};

SDL_Texture* n_texture_0;
SDL_Texture* n_texture_1;
SDL_Texture* n_texture_2;
SDL_Texture* n_texture_3;

void init_name_tables() {

    for(int i = 0; i < 256*256; i++) {
        n_table_0[i] = 0;
        n_table_1[i] = 0;
        n_table_2[i] = 0;
        n_table_3[i] = 0;
    }
}

void fetch_name_table(int table_id) {

    int table_addr = 0x2000 + (0x400*table_id);
    uint32_t* table_pointer;
    switch (table_id)
    {
        case 0:
            table_pointer = &n_table_0[0];
            break;
        case 1:
            table_pointer = &n_table_1[0];
            break;
        case 2:
            table_pointer = &n_table_2[0];
            break;
        case 3:
            table_pointer = &n_table_3[0];
            break;
    }

    /*
    i = sprite index, left to right, top to bottom
    j = byte number of sprite, byte contains pixel data for each row of sprite, top to bottom
    k = bit of byte, the column of the pixel
    */
    for(int i = 0; i < 32*30; i++) {
        int sprite_addr = 0x1000 + (peek_vram(table_addr + i)*0x10);

        for(int j = 0; j < 8; j++) {

            uint8_t pattern_byte = peek_vram(sprite_addr + j);
            uint8_t pattern_byte_2 = peek_vram(sprite_addr + 8 + j);

            for(int k = 0; k < 8; k++) {

                uint32_t* pixel = table_pointer + k + (j*256) + ((i%32)*8) + ((i/32) * 256 * 8); 
            
                *pixel = (pattern_byte >> (7-(k%8))) & 0b1;
                *pixel += ((pattern_byte_2 >> (7-(k%8))) & 0b1) << 1;
                if(*pixel == 0) 
                    *pixel = 0xff000000;
                if(*pixel == 1)
                    *pixel = 0xffffffff;
                if(*pixel == 2)
                    *pixel = 0xff00ff00;
                if(*pixel == 3)
                    *pixel = 0xffff0000;
                *pixel |= 0xff000000;
            }
        }
    }
}

void create_name_table_textures(SDL_Renderer* renderer) 
{
    SDL_Rect background_rect = (SDL_Rect) {
        n_table_0_rect.x, n_table_0_rect.y, n_table_0_rect.w*2, n_table_3_rect.y + n_table_3_rect.h
    };
    SDL_SetRenderDrawColor(renderer, 84,84,84, 255);
    SDL_RenderFillRect(renderer, &background_rect);

    SDL_Surface* surface_0 = SDL_CreateRGBSurfaceFrom(n_table_0, 256, 256, 32, 256*4/*pitch*/, 0, 0, 0, 0);
    SDL_Surface* surface_1 = SDL_CreateRGBSurfaceFrom(n_table_1, 256, 256, 32, 256*4/*pitch*/, 0, 0, 0, 0);
    SDL_Surface* surface_2 = SDL_CreateRGBSurfaceFrom(n_table_2, 256, 256, 32, 256*4/*pitch*/, 0, 0, 0, 0);
    SDL_Surface* surface_3 = SDL_CreateRGBSurfaceFrom(n_table_3, 256, 256, 32, 256*4/*pitch*/, 0, 0, 0, 0);

    n_texture_0 = SDL_CreateTextureFromSurface(renderer, surface_0);
    n_texture_1 = SDL_CreateTextureFromSurface(renderer, surface_1);
    n_texture_2 = SDL_CreateTextureFromSurface(renderer, surface_2);
    n_texture_3 = SDL_CreateTextureFromSurface(renderer, surface_3);

    SDL_FreeSurface(surface_0);
    SDL_FreeSurface(surface_1);
    SDL_FreeSurface(surface_2);
    SDL_FreeSurface(surface_3);

    render_name_tables(renderer);
}

void render_name_tables(SDL_Renderer* renderer) 
{
    SDL_RenderCopy(renderer, n_texture_0, NULL, &n_table_0_rect);
    SDL_RenderCopy(renderer, n_texture_1, NULL, &n_table_1_rect);
    SDL_RenderCopy(renderer, n_texture_2, NULL, &n_table_2_rect);
    SDL_RenderCopy(renderer, n_texture_3, NULL, &n_table_3_rect);

    SDL_DestroyTexture(n_texture_0);
    SDL_DestroyTexture(n_texture_1);
    SDL_DestroyTexture(n_texture_2);
    SDL_DestroyTexture(n_texture_3);
}

void Shut_Down_NT() 
{
    SDL_DestroyTexture(n_texture_0);
    SDL_DestroyTexture(n_texture_1);
    SDL_DestroyTexture(n_texture_2);
    SDL_DestroyTexture(n_texture_3);
}