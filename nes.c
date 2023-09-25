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
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

#include "nes.h"
#include "debug/debug.h"
#include "processing/cpu.h"
#include "memory/mem.h"
#include "memory/rom.h"
#include "processing/ppu.h"
#include "devices/display.h"
#include "devices/controller.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

const int WIDTH = (256 * 2) + (2 * 214) + 128;
const int HEIGHT = 240 * 2;
const int CYCLES_PER_FRAME = 29780;

bool Initialize(void);
void Update(float);
void ShutDown(void);

double cycletime = (double)60/1790000;
float time = 0;
bool pause = true;


int cycles_per_loop = 29780;

u_int16_t breakpoints[] = {0xc074};

int main(int argc, const char * argv[]) {

    atexit(ShutDown);
    
    if(!Initialize()) {
        exit(1);
    }

    Uint32 lastTick = SDL_GetTicks(); // milliseconds since SDL_Init()
    bool quit = false;

    SDL_Event event;
    while(!quit) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                quit = true;
            }
            if(event.type == SDL_MOUSEWHEEL) {
                Debug_Scroll(event);
            }
            if(event.type == SDL_MOUSEBUTTONDOWN) {
                Debug_Mouse_Down(event);
            }
            if(event.type == SDL_MOUSEBUTTONUP) {
                Debug_Mouse_Up();
            }
            if(event.type == SDL_MOUSEMOTION) {
                Debug_Mouse_Motion(event, HEIGHT);
            }
            if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_P) {
                    pause = !pause;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_UP) {
                    next_frame = true;
                    pause = false;
                }if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
                    next_instruction = true;
                    pause = false;
                }

                if(event.key.keysym.scancode == SDL_SCANCODE_K) {
                    button_down(0);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_L) {
                    button_down(1);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_M) {
                    button_down(2);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_N) {
                    button_down(3);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_W) {
                    button_down(4);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_S) {
                    button_down(5);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_A) {
                    button_down(6);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_D) {
                    button_down(7);
                }
            }
            if(event.type == SDL_KEYUP) {

                if(event.key.keysym.scancode == SDL_SCANCODE_K) {
                    button_up(0);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_L) {
                    button_up(1);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_M) {
                    button_up(2);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_N) {
                    button_up(3);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_W) {
                    button_up(4);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_S) {
                    button_up(5);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_A) {
                    button_up(6);
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_D) {
                    button_up(7);
                }
            }
        }
        
        Update(0);
    }

    SDL_Quit();
}

bool Initialize(void) {

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to init SDL: %s\n", 
            SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("NES",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if(!window) {
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!window) {
        return false;
    }

    create_grid();

    //Load_Rom("./tests/cpu_flag_concurrency/test_cpu_flag_concurrency.nes");
    //Load_Rom("./tests/branch_timing_tests/2.Backward_Branch.nes");
    //Load_Rom("./tests/nes-test-roms-master/instr_test-v5/rom_singles/04-zero_page.nes");
    //Load_Rom("./tests/cpu_exec_space/test_cpu_exec_space_apu.nes");
    //Load_Rom("./tests/nes-test-roms-master/instr_test-v3/rom_singles/02-immediate.nes");
    
    //Load_Rom("./tests/nestest.nes");
    Load_Rom("./tests/TLOZ.nes");
    //Load_Rom("./tests/supermario.nes");
    //Load_Rom("./tests/DK.nes");
    //Load_Rom("./tests/nes-test-roms-master/instr_misc/rom_singles/04-dummy_reads_apu.nes");

    Init_CPU();

    if(!Init_Debug(WIDTH, HEIGHT)) {
        return false;
    }

    Update_Debug(renderer);
    SDL_RenderPresent(renderer);

    return true;
}

void Update(float elapsed) {

    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);

    if(next_instruction)
        cycles_per_loop = 1;

    if(!pause) {
        for(int i = 0; i < cycles_per_loop && !pause; i+=0) {

            int cpu_cycles = Update_CPU();
            i += cpu_cycles;

            int prev_scanline = scanline;
            for(int j = 0; j < cpu_cycles*3; j++) {
                Update_PPU();
                if(prev_scanline > scanline) {
                    i = cycles_per_loop;
                    break;
                }
                if(scanline == 241 && dot == 0) {
                    copy_buffer(frame_buffer);
                    clear_frame_buffer();
                }
            }
            if(scanline == 240 && prev_scanline != 240 && next_frame) {
                pause = true;
                next_frame = false;
            }
        }
    }
 
    Render_Frame(renderer);
    Update_Debug(renderer);
    SDL_RenderPresent(renderer);

    if(next_instruction) {
        cycles_per_loop = CYCLES_PER_FRAME;
        pause = true;
        next_instruction = false;
    }

}

void ShutDown(void) {

    if(renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if(window) {
        SDL_DestroyWindow(window);
    }
    Shut_Down_Debug();
    Shut_Down_ROM();

    SDL_Quit();
}