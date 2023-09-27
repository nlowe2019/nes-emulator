#include <SDL2/SDL_blendmode.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>

#include "../debug/debug.h"

SDL_Texture* texture;

SDL_Rect screen = (SDL_Rect) {
    0,
    0,
    256*2,
    240*2
};

uint32_t frame_data[240][256];

uint32_t grid_overlay_data[240][256];
SDL_Texture* grid_overlay_texture;

void create_grid() {
    for (int i = 0; i < 256; i++) {
        for(int j = 0; j < 240; j++) {
            grid_overlay_data[j][i] = 0x00000000;
            if(i % 16 == 0 || j % 16 == 0) {
                grid_overlay_data[j][i] = 0xff600000;
            }
            if(i % 32 == 0 || j % 32 == 0) {
                grid_overlay_data[j][i] = 0xffff0000;
            }
        }
    }
}

void copy_buffer(uint32_t fb[240][256]) {

    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 240; j++) {
            frame_data[j][i] = fb[j][i];
        }
    }
}

void Render_Frame(SDL_Renderer * renderer) {

    if(debug_on)
        screen.x = 214;
    else
        screen.x = 0;

    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(*frame_data, 256, 240, 32, 256*4/*pitch*/, 0, 0, 0, 0);

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(renderer, texture, NULL, &screen);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    // grid
    if(nametable_overlay_on)
    {
        surface = SDL_CreateRGBSurfaceFrom(*grid_overlay_data, 256, 240, 32, 256*4/*pitch*/, 0, 0, 0, 0);
        grid_overlay_texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_SetTextureAlphaMod(grid_overlay_texture, 50);
        SDL_SetTextureBlendMode(grid_overlay_texture, SDL_BLENDMODE_BLEND);
        SDL_RenderCopy(renderer, grid_overlay_texture, NULL, &screen);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(grid_overlay_texture);
    }
}