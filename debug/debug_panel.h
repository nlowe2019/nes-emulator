#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_ttf.h>

/*
values used to generate renderable text for a
point in memory
*/
typedef struct debug_text_data {

    char* label;
    unsigned char data;
    char* display_string;

} debug_text_data;

/*
Panel used to display debug information such as
emulator memory values
*/
typedef struct Panel {

    char* name;
    bool active;

    float x;
    float y;
    float width;
    float height;

    int scroll_y;
    bool scroll_active;
    SDL_Rect scroll_rect;

    TTF_Font* font;
    TTF_Font* font_alt;
    SDL_Color font_color;
    int font_size;

    int ADDR_RANGE;

    int ARRAY_LEN;
    debug_text_data** debug_memory_render;

} Panel;

Panel* Create_Panel(char* name, float x, float y, float width, float height, int ADDR_RANGE, int ARRAY_LEN);
void Free_Panel(Panel* panel);
void Render_Panel(SDL_Renderer *renderer, Panel* panel);
void Update_Rendered_Content(Panel* panel);
debug_text_data* Create_Debug_Text(char *name, unsigned char value);

void Panel_Scroll(Panel* panel, int dy);

char* status_2_text(char* str, unsigned char num);