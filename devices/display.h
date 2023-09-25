#include <SDL2/SDL_render.h>
#include <sys/types.h>

void Render_Frame(SDL_Renderer* renderer);
void create_grid();
void copy_buffer(u_int32_t fb[240][256]);