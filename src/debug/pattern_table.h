#include <SDL2/SDL_render.h>

void render_pattern_tables(SDL_Renderer* renderer);
void render_sprites(SDL_Renderer* renderer);
void render_palettes(SDL_Renderer* renderer);
void fetch_pattern_table(int table);
void init_tables();
void Shut_Down_PT();
void create_table_texture(SDL_Renderer* renderer);