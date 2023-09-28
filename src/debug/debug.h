#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <stdbool.h>

#include "debug_panel.h"

extern bool debug_on;
extern bool nametable_overlay_on;
extern bool next_frame;
extern bool next_instruction;

bool Init_Debug(int WIDTH, int HEIGHT);
void Shut_Down_Debug();
void Update_Debug(SDL_Renderer *renderer); 
void update_pattern_tables(SDL_Renderer* renderer);
void update_name_tables(SDL_Renderer* renderer);

void Debug_Scroll(SDL_Event event_wheel_y);
void Debug_Mouse_Down(SDL_Event event_click);
void Debug_Mouse_Up(void);
void Debug_Mouse_Motion(SDL_Event event_motion, int HEIGHT);