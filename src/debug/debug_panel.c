#include <SDL2/SDL.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "debug_panel.h"
#include "../memory/mem.h"
#include "../processing/cpu.h"
#include "../processing/ppu.h"


/*
Panel object constructor
*/
Panel* Create_Panel(char* name, float x, float y, float width, float height, int ADDR_RANGE, int ARRAY_LEN) {

    Panel* panel = (Panel*) malloc(sizeof(Panel));
    
    panel->name = (char*) malloc(sizeof(char) * 4);
    strcpy(panel->name, name);

    panel->x = x;
    panel->y = y;
    panel->width = width;
    panel->height = height;
  
    panel->font_color = (SDL_Color) {255,255,255};
    panel->font_size = 20;
    panel->font = TTF_OpenFont("../ARCADE_N.TTF", panel->font_size);
    panel->font_alt = TTF_OpenFont("../kongtext.ttf", panel->font_size);
    if(panel->font == NULL || panel->font_alt == NULL) {
        fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
    }

    panel->active = true;
    panel->scroll_active = false;
    panel->scroll_y = 0;
    panel->scroll_rect = (SDL_Rect) {
        x + width - 16,
        y,
        16,
        40,
    };

    panel->ADDR_RANGE = ADDR_RANGE;
    panel->ARRAY_LEN = ARRAY_LEN;
    panel->debug_memory_render = (debug_text_data**) malloc(sizeof(debug_text_data*) * ARRAY_LEN);
    for(int i = 0; i < panel->ARRAY_LEN; i++) {
        panel->debug_memory_render[i] = Create_Debug_Text("0x0000", 0);
    }

    Panel_Scroll(panel, 0);

    return panel;
}

void Free_Panel(Panel* panel) {

    if(panel->font) {
        TTF_CloseFont(panel->font);
    }
    if(panel->font_alt) {
        TTF_CloseFont(panel->font_alt);
    }
    free(panel->debug_memory_render);
    free(panel);
}

void Render_Panel(SDL_Renderer *renderer, Panel* panel) {

    Update_Rendered_Content(panel);

    SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_Rect panel_background = {
        panel->x,
        panel->y,
        panel->width,
        panel->height,
    };
    SDL_RenderFillRect(renderer, &panel_background);

    for(int i = 0; i < panel->ARRAY_LEN; i++) {

        SDL_Surface* surfaceText = TTF_RenderText_Solid(panel->font, panel->debug_memory_render[i]->display_string, panel->font_color); 
        if(i == 1 && !strcmp(panel->name, "REG"))
            surfaceText = TTF_RenderText_Solid(panel->font_alt, panel->debug_memory_render[i]->display_string, panel->font_color); 
        SDL_Texture* textureText = SDL_CreateTextureFromSurface(renderer, surfaceText);

        SDL_Rect text_background = {
            panel->x+4,
            panel->y
        };

        TTF_SizeText(panel->font, panel->debug_memory_render[i]->display_string, 
                &text_background.w, &text_background.h);
        //text_background.w /= 3;
        //text_background.h /= 3;
        text_background.y += (text_background.h + 2) * i;
        if(text_background.w > panel->width)
            text_background.w = panel->width - 8;

        SDL_RenderCopy(renderer, textureText, NULL, &text_background);
        SDL_FreeSurface(surfaceText);
        SDL_DestroyTexture(textureText);
    }
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &panel->scroll_rect);
}

void Update_Rendered_Content(Panel* panel) {

    if(!strcmp(panel->name, "REG")) {
        char* str = malloc(sizeof(char) * 9);
        strcpy(str, "nv--dizc");

        sprintf(panel->debug_memory_render[0]->display_string, "PC|%02x%02x", (uint8_t)(pc >> 8), (uint8_t)pc);
        sprintf(panel->debug_memory_render[1]->display_string, "ST|%s", status_2_text(str, status));
        sprintf(panel->debug_memory_render[2]->display_string, "A |%02x", accumulator);
        sprintf(panel->debug_memory_render[3]->display_string, "X |%02x", index_x);
        sprintf(panel->debug_memory_render[4]->display_string, "Y |%02x", index_y);
        sprintf(panel->debug_memory_render[5]->display_string, "SP|%02x", sp);
        sprintf(panel->debug_memory_render[6]->display_string, "Line:%d", scanline);
        sprintf(panel->debug_memory_render[7]->display_string, "Dot:%d", dot);
        sprintf(panel->debug_memory_render[8]->display_string, "v:%04x", ppu_address);
        free(str);
    }

    int start_val = panel->scroll_y;

    for(int i = 0; i < panel->ARRAY_LEN; i++) {
        
        //sprintf(panel->debug_memory_render[i]->label, "0x%04x", (i+start_val) % panel->ADDR_RANGE);
        if(!strcmp(panel->name, "RAM")) {
            panel->debug_memory_render[i]->data = peek_ram((i + start_val) % (panel->ADDR_RANGE+1));
            sprintf(panel->debug_memory_render[i]->display_string, "0x%04x|%02x", (i+start_val) % (panel->ADDR_RANGE+1), panel->debug_memory_render[i]->data);
        }
        else if(!strcmp(panel->name, "VRAM")) {
            panel->debug_memory_render[i]->data = peek_vram((i + start_val) % (panel->ADDR_RANGE+1));
            sprintf(panel->debug_memory_render[i]->display_string, "0x%04x|%02x", (i+start_val) % (0x4000), panel->debug_memory_render[i]->data);
        }
    }
}

/*
On panel scroll, update rendered content and scroll bar position
*/
void Panel_Scroll(Panel* panel, int dy) {
    
    // Calculate scroll bar position
    panel->scroll_y += dy;
    float scroll_ratio = panel->scroll_y / (float)panel->ADDR_RANGE;
    panel->scroll_rect.y = panel->y + ((panel->height - panel->scroll_rect.h) * scroll_ratio);
}

debug_text_data * Create_Debug_Text(char *name, unsigned char value) {

    debug_text_data *db_text = (debug_text_data*) malloc(sizeof(debug_text_data));
    db_text->label = (char*)malloc(sizeof(char)*8);
    strcpy(db_text->label, name);
    db_text->data = value;
    db_text->display_string = (char*)malloc(16 * sizeof(char));

    return db_text;
}

char* status_2_text(char* str, unsigned char number)
{
    for(int i = 0; i < 8; i++) {
        if((number >> i) & 1) {
            str[7-i] = toupper(str[7-i]);
        }
    }
    return str;
}