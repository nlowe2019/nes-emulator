#include <SDL2/SDL_render.h>
#include <stdint.h>

void Render_Frame(SDL_Renderer* renderer);
void create_grid();
void copy_buffer(uint32_t fb[240][256]);