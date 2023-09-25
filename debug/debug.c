#include <SDL2/SDL.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "pattern_table.h"

int panel_num = 3;
Panel* panels[4];

bool next_frame = false;
bool next_instruction = false;

bool Init_Debug(int WIDTH, int HEIGHT) {

    if(TTF_Init() == -1) {
        fprintf(stderr, "Error initializing SDL_ttf: %s\n",
            TTF_GetError()
        );
        return false;
    }

    panels[0] = Create_Panel("RAM", 0, HEIGHT/2, 214, HEIGHT/2, 0xFFFF, 16);
    panels[1] = Create_Panel("VRAM", 214+ 512, 0, 214, HEIGHT, 0xFFFF, 32);
    panels[2] = Create_Panel("REG", 0, 0, 214, HEIGHT/2, 0, 9);

    init_tables();
    fetch_table(0);
    fetch_table(1);

    return true;
}

void Shut_Down_Debug() {

    for(int i = 0; i < panel_num; i++) {
        Free_Panel(panels[i]);
    }

    Shut_Down_PT();
}

void Update_Debug(SDL_Renderer* renderer) {

    for(int i = 0; i < panel_num; i++) {
        Render_Panel(renderer, panels[i]);
    }

    Update_Pattern_Tables(renderer);
    render_pattern_tables(renderer);
}

void Update_Pattern_Tables(SDL_Renderer* renderer) {

    fetch_table(0);
    fetch_table(1);
}

// SDL event

void Debug_Mouse_Up() {
    
    for(int i=0; i<panel_num; i++) {
        panels[i]->scroll_active = false;
    }
}

void Debug_Mouse_Down(SDL_Event event) {

    SDL_Rect click_area = {
        event.button.x,
        event.button.y,
        1,1
    };

    for(int i=0; i<panel_num; i++) {
        if(SDL_HasIntersection(&click_area, &panels[i]->scroll_rect)) {
            panels[i]->scroll_active = true;
        }   
    }
}

void Debug_Mouse_Motion(SDL_Event event, int HEIGHT) {

    for(int i=0; i<panel_num; i++) {
        if(panels[i]->scroll_active) { 

            float scroll_ratio = (event.motion.y - panels[i]->y) / (panels[i]->height - panels[i]->scroll_rect.h);
            if(scroll_ratio < 0)
                scroll_ratio = 0;
            else if(scroll_ratio > 1)
                scroll_ratio = 1;

            panels[i]->scroll_y = scroll_ratio * panels[i]->ADDR_RANGE;
            panels[i]->scroll_rect.y = panels[i]->y + ((panels[i]->height - panels[i]->scroll_rect.h) * scroll_ratio);
        }
    }
}

void Debug_Scroll(SDL_Event event) {

    int x = 0; int y = 0;

    SDL_GetMouseState(&x, &y);

    SDL_Rect panel_bounds;
    SDL_Rect mouse_pos = {
        x, y, 1, 1
    };

    for(int i=0; i<panel_num; i++) {

        panel_bounds = (SDL_Rect) {
            panels[i]->x, panels[i]->y, panels[i]->width, panels[i]->height
        };

        if(SDL_HasIntersection(&panel_bounds, &mouse_pos)) { 

            if(event.wheel.y > 0) {
                panels[i]->scroll_y -= panels[i]->ARRAY_LEN/2;
                if(panels[i]->scroll_y < 0)
                    panels[i]->scroll_y += panels[i]->ADDR_RANGE;
            }
            else if(event.wheel.y < 0) {
                panels[i]->scroll_y += panels[i]->ARRAY_LEN/2;
            }
            panels[i]->scroll_y %= (panels[i]->ADDR_RANGE);

            panels[i]->scroll_rect.y = panels[i]->y + ((float) panels[i]->scroll_y / panels[i]->ADDR_RANGE) * ((panels[i]->height - panels[i]->scroll_rect.h));
        }
    }
}