#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdlib.h>
#include <stdint.h>

#include "pattern_table.h"
#include "../memory/mem.h"
#include "../processing/palette.h"

uint32_t p_table_0[128*128];
uint32_t p_table_1[128*128];

SDL_Rect table_0_rect = {
    214*2+256*2,
    0,
    128,
    128
};
SDL_Rect table_1_rect = {
    512 + 214 + 214,
    480-128,
    128,
    128
};

SDL_Texture* texture_0;
SDL_Texture* texture_1;

void init_tables() {

    for(int i = 0; i < 128*128; i++) {
        p_table_0[i] = 0;
    }
    for(int i = 0; i < 128*128; i++) {
        p_table_1[i] = 0;
    }
}

void fetch_table(int table) {

    int table_addr = (table == 1 ? 0x1000 : 0x0000);
    uint32_t* table_pointer;
    table_pointer = (table_addr == 0x1000 ? &p_table_1[0] : &p_table_0[0]);

    /*
    i = sprite index, left to right, top to bottom
    j = byte number of sprite, byte contains pixel data for each row of sprite, top to bottom
    k = bit of byte, the column of the pixel
    */
    for(int i = 0; i < 0x100; i++) {
        int sprite_addr = table_addr + (i*0x10);

        for(int j = 0; j < 8; j++) {

            uint8_t pattern_byte = peek_vram(sprite_addr + j);
            uint8_t pattern_byte_2 = peek_vram(sprite_addr + 8 + j);

            for(int k = 0; k < 8; k++) {

                uint32_t* pixel = table_pointer + k + (j*128) + ((i%16)*8) + ((i/16) * 128 * 8); 
            
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

void create_table_texture(SDL_Renderer* renderer) {
    
    SDL_Surface* surface_0 = SDL_CreateRGBSurfaceFrom(p_table_0, 128, 128, 32, 128*4/*pitch*/, 0, 0, 0, 0);
    SDL_Surface* surface_1 = SDL_CreateRGBSurfaceFrom(p_table_1, 128, 128, 32, 128*4/*pitch*/, 0, 0, 0, 0);

    texture_0 = SDL_CreateTextureFromSurface(renderer, surface_0);
    texture_1 = SDL_CreateTextureFromSurface(renderer, surface_1);

    SDL_FreeSurface(surface_0);
    SDL_FreeSurface(surface_1);

}

void render_pattern_tables(SDL_Renderer* renderer) {

    SDL_Rect background_rect = (SDL_Rect) {
        table_0_rect.x, table_0_rect.y, table_0_rect.w, table_1_rect.y + table_1_rect.h
    };
    SDL_SetRenderDrawColor(renderer, 84,84,84, 255);
    SDL_RenderFillRect(renderer, &background_rect);

    create_table_texture(renderer);
    render_sprites(renderer);
    render_palettes(renderer);
}

void render_sprites(SDL_Renderer* renderer) {

    SDL_RenderCopy(renderer, texture_0, NULL, &table_0_rect);
    SDL_RenderCopy(renderer, texture_1, NULL, &table_1_rect);

    SDL_DestroyTexture(texture_0);
    SDL_DestroyTexture(texture_1);
}

void render_palettes(SDL_Renderer* renderer) {

    SDL_Color color;
    SDL_Rect color_rect = (SDL_Rect) {
        table_0_rect.x, table_0_rect.y + table_0_rect.h, 16, 16
    };

    for(int i = 0; i < 16; i++) {
        
        int color_id = read_vram(0x3f00 + i);
        color = palette[color_id];
        //if(i % 4 == 0)
        //    color = palette[read_vram(0x3f00)];

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_RenderFillRect(renderer, &color_rect);
        color_rect.x += 16;
        if(color_rect.x > table_0_rect.x + (16 * 7)) {
            color_rect.x = table_0_rect.x;
            color_rect.y += 16;
        }
    }

    color_rect.y = table_1_rect.y - 32;
    for(int i = 16; i < 32; i++) {
        
        int color_id = read_vram(0x3f00 + i);
        color = palette[color_id];
        //if(i % 4 == 0)
        //    color = palette[read_vram(0x3f00)];

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_RenderFillRect(renderer, &color_rect);
        color_rect.x += 16;
        if(color_rect.x > table_0_rect.x + (16 * 7)) {
            color_rect.x = table_0_rect.x;
            color_rect.y += 16;
        }
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

void Shut_Down_PT() {

    SDL_DestroyTexture(texture_0);
    SDL_DestroyTexture(texture_1);
}