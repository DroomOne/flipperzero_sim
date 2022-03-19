#include "flipper.h"

#include "../img/micro4x6.xbm"
#include <string.h>
#include <stdlib.h>

#define FLAPPY_MAX_PILARS 6
#define FLAPPY_PILAR_WIDTH 7

#define FLAPPY_GAB_HEIGHT 25
#define FLAPPY_GAB_WIDTH 35
#define FLAPPY_GRAVITY_TICK 0.15

typedef struct {
    int width, height;
    int x, y; 
    uint8_t data[15][11];
}BIRD;

typedef struct {
    int x;
    int y;
} POINT;


typedef struct { 
    POINT point; 
    int height;
    int visible;
    int passed; 
} PILAR; 

typedef struct { 
    int active;
    int points; 
    int pilar_len; 
    float gravity; 
    PILAR pilar[FLAPPY_MAX_PILARS];
    BIRD bird; 
} GAME;

#define BUTTONS 4

int buttons[BUTTONS] = {
    FL_GPIO_BUTTON_RIGHT,
    FL_GPIO_BUTTON_DOWN,
    FL_GPIO_BUTTON_LEFT,
    FL_GPIO_BUTTON_UP,
};
// // low
// 0 0 0 0 0 0 1 1 1 1 1 0 0 0 0
// 0 0 0 0 1 1 0 0 1 0 0 1 0 0 0
// 0 0 0 1 0 0 0 1 0 0 0 0 1 0 0
// 0 0 0 1 0 0 0 1 0 0 0 1 0 1 0
// 0 0 0 1 0 0 0 1 0 0 0 1 0 1 0
// 0 1 1 1 1 0 0 0 0 1 1 1 1 1 0
// 1 0 0 0 0 1 0 0 1 0 0 0 0 0 1
// 1 0 0 0 0 0 1 1 0 1 1 1 1 1 1
// 0 1 0 0 0 1 0 0 1 0 0 0 0 0 1
// 0 0 1 1 1 0 0 0 0 1 1 1 1 1 0
// 0 0 0 0 0 1 1 1 1 0 0 0 0 0 0

// //mid
// 0 0 0 0 0 0 1 1 1 1 1 0 0 0 0
// 0 0 0 0 1 1 0 0 1 0 0 1 0 0 0
// 0 0 0 1 0 0 0 1 0 0 0 0 1 0 0
// 0 0 0 1 0 0 0 1 0 0 0 1 0 1 0
// 0 1 1 1 1 0 0 1 0 0 0 1 0 1 0
// 1 0 0 0 0 1 0 0 0 1 1 1 1 1 0
// 1 0 0 0 0 0 1 0 1 0 0 0 0 0 1
// 0 1 0 0 0 1 0 1 0 1 1 1 1 1 1
// 0 0 1 1 1 0 0 0 1 0 0 0 0 0 1
// 0 0 0 0 1 0 0 0 0 1 1 1 1 1 0
// 0 0 0 0 0 1 1 1 1 0 0 0 0 0 0

// //high
// 0 0 0 0 0 0 1 1 1 1 1 0 0 0 0
// 0 0 0 0 1 1 0 0 1 0 0 1 0 0 0
// 0 0 0 1 0 0 0 1 0 0 0 0 1 0 0
// 0 1 1 1 1 0 0 1 0 0 0 1 0 1 0
// 1 0 0 0 0 1 0 1 0 0 0 1 0 1 0
// 1 0 0 0 0 0 1 0 0 1 1 1 1 1 0
// 0 1 0 0 0 1 0 0 1 0 0 0 0 0 1
// 0 0 1 1 1 0 0 1 0 1 1 1 1 1 1
// 0 0 1 0 0 0 0 0 1 0 0 0 0 0 1
// 0 0 0 1 1 0 0 0 0 1 1 1 1 1 0
// 0 0 0 0 0 1 1 1 1 0 0 0 0 0 0


uint8_t bird_array[15][11] = {
    {0,0,0,0,0,0,1,1,0,0,0},
    {0,0,0,0,0,1,0,0,1,0,0},
    {0,0,0,0,0,1,0,0,0,1,0},
    {0,0,1,1,1,1,0,0,0,1,0},
    {0,1,0,0,0,1,0,0,0,1,0},
    {0,1,0,0,0,0,1,0,1,0,1},
    {1,0,0,0,0,0,0,1,0,0,1},
    {1,0,1,1,1,0,0,1,0,0,1},
    {1,1,0,0,0,0,1,0,1,0,1},
    {1,0,0,0,0,1,0,1,0,1,0},
    {1,0,0,0,0,1,0,1,0,1,0},
    {0,1,0,1,1,1,0,1,0,1,0},
    {0,0,1,0,0,1,0,1,0,1,0},
    {0,0,0,1,1,1,0,1,0,1,0},
    {0,0,0,0,0,0,1,1,1,0,0},
};

uint8_t bird_mid[15][11] = {
    {0,0,0,0,0,1,1,0,0,0,0},
    {0,0,0,0,1,0,0,1,0,0,0},
    {0,0,0,0,1,0,0,0,1,0,0},
    {0,0,1,1,1,0,0,0,1,0,0},
    {0,1,0,0,1,0,0,0,1,1,0},
    {0,1,0,0,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,1},
    {1,0,1,1,1,0,0,1,0,0,1},
    {1,1,0,0,0,0,1,0,1,0,1},
    {1,0,0,0,0,1,0,1,0,1,0},
    {1,0,0,0,0,1,0,1,0,1,0},
    {0,1,0,1,1,1,0,1,0,1,0},
    {0,0,1,0,0,1,0,1,0,1,0},
    {0,0,0,1,1,1,0,1,0,1,0},
    {0,0,0,0,0,0,1,1,1,0,0},
};

uint8_t bird_up[15][11] = {
    {0,0,0,0,1,1,0,0,0,0,0},
    {0,0,0,1,0,0,1,0,0,0,0},
    {0,0,0,1,0,0,0,1,1,0,0},
    {0,0,1,1,0,0,0,1,0,1,0},
    {0,1,0,1,0,0,0,1,0,1,0},
    {0,1,0,0,1,0,1,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0,1},
    {1,0,1,1,1,0,0,1,0,0,1},
    {1,1,0,0,0,0,1,0,1,0,1},
    {1,0,0,0,0,1,0,1,0,1,0},
    {1,0,0,0,0,1,0,1,0,1,0},
    {0,1,0,1,1,1,0,1,0,1,0},
    {0,0,1,0,0,1,0,1,0,1,0},
    {0,0,0,1,1,1,0,1,0,1,0},
    {0,0,0,0,0,0,1,1,1,0,0},
};


    // {0,0,0,0,0,1,0,1,1,0,0},
    // {0,0,0,1,1,1,0,1,0,1,0},
    // {0,0,1,0,0,1,0,1,0,1,0},
    // {0,1,0,1,1,1,0,1,0,1,0},
    // {1,0,0,0,0,1,0,1,0,1,0},
    // {1,0,0,0,0,1,0,1,0,1,0},
    // {1,1,0,0,0,0,1,0,1,0,1},
    // {1,0,1,1,1,0,0,1,0,0,1},
    // {1,0,0,0,0,0,0,1,0,0,1},
    // {0,1,0,0,0,0,1,0,1,0,1},
    // {0,1,0,0,0,1,0,0,0,1,0},
    // {0,0,1,1,1,1,0,0,0,1,0},
    // {0,0,0,0,0,1,0,0,0,1,0},
    // {0,0,0,0,0,1,0,0,1,0,0},
    // {0,0,0,0,0,0,1,1,0,0,0},


void draw_border() {
    for (int i = 0; i < FL_LCD_WIDTH; i++) { 
        flipper_pixel_set(i, FL_LCD_HEIGHT - 1); 
    }
}

void flappy_draw_horizontal(uint8_t x, uint8_t y, uint8_t l) {
    for (int i = 0; i < l; i++) {
        flipper_pixel_set(x + i, y);
    } 
}

void flappy_draw_vertical(uint8_t x, uint8_t y, uint8_t l) {
    for (int i = 0; i < l; i++) {
        flipper_pixel_set(x, y + i);
    } 
}

 

void draw_char(int x, int y, char c, int invert) {
    if (c < 0 || c > 127)
        return;
    int py = (c / 16) * font_glyph_height;
    int px = (c % 16) * font_glyph_width;

    // int bitchar[font_glyph_height][font_glyph_width]; 
    // int bitrotate[font_glyph_width][font_glyph_height]; 

    for (int j = 0; j < font_glyph_height; j++) {
        for (int i = 0; i < font_glyph_width; i++) {
            int yy = py + j;
            int xx = px + i;
            int bits = font_bits[yy * font_width / 8 + xx / 8]; 
            if (((bits >> (xx & 7)) & 1) == invert)
                flipper_pixel_set(x + i, y + j); 
            else
                flipper_pixel_clear(x + i, y + j);
        }
    }
    
} 

void flappy_draw_pilar(PILAR* p) {  
    // Upper 
    flappy_draw_horizontal(p->point.x, p->point.y + p->height , FLAPPY_PILAR_WIDTH); 
    flappy_draw_vertical(p->point.x, p->point.y, p->height);
    flappy_draw_vertical(p->point.x + FLAPPY_PILAR_WIDTH - 1, p->point.y, p->height);

    // Lower
    flappy_draw_horizontal(p->point.x, p->point.y + p->height + FLAPPY_GAB_HEIGHT, FLAPPY_PILAR_WIDTH);
    flappy_draw_vertical(p->point.x, p->point.y + p->height + FLAPPY_GAB_HEIGHT, FL_LCD_HEIGHT - p->height - FLAPPY_GAB_HEIGHT);
    flappy_draw_vertical(p->point.x + FLAPPY_PILAR_WIDTH - 1, p->point.y + p->height + FLAPPY_GAB_HEIGHT, FL_LCD_HEIGHT - p->height - FLAPPY_GAB_HEIGHT);
}

bool flappy_check_pilar(GAME *g) {

    for (int i = 0; i < FLAPPY_MAX_PILARS; i++){
        PILAR * p = &g->pilar[i]; 

        if (p != NULL && p->visible) {
            //        |  |
            //        |  |
            //        |__|    FLAPPY_PILAR_WIDTH
            //   _____X
            //  |     |
            //  |_____|
            // X <---->
            // [Bird Pos + Lenght of the bird] >= [Pilar]
            if (g->bird.x + g->bird.width - 2 >= p->point.x)
            {
                //   |  |
                //   |  |
                //   |__|    FLAPPY_PILAR_WIDTH
                //      X_____
                //      |     |
                //      |_____|
                //   X <-> 
                // [Pos + Width of pilar] >= [Bird Pos]
                if (p->point.x + FLAPPY_PILAR_WIDTH >= g->bird.x)
                {
                    // Not between the pipes
                    if (g->bird.y <= p->height || g->bird.y + g->bird.height >= p->height+FLAPPY_GAB_HEIGHT)
                        return true; 
                } 
            }
        }   
    }
    return false; 
}

void draw_string(int x, int y, char* string) {
    while (*string) {
        draw_char(x, y, *string, 0);
        x+= 4;
        string++;
    }
}


void draw_score(GAME *g) {
    int score = g->points;
    // because rotated, use lcd height instead of width
    int x = 1 + font_glyph_width * 2;
    int y = 1;
    do {
        char c = '0' + (score % 10);
        draw_char(x, y, c, 0);
        x -= font_glyph_width;
        score /= 10;
    } while (score > 0);
}

void draw_gravity(GAME *g) {
    int score = g->gravity;
    // because rotated, use lcd height instead of width
    int x = 1 + font_glyph_width * 2;
    int y = 10;
    do {
        char c = '0' + (score % 10);
        draw_char(x, y, c, 0);
        x -= font_glyph_width;
        score /= 10;
    } while (score > 0);
}


void flappy_draw(GAME *g) {
    if (g->active) {
        // Draw Pillars
        for (int i = 0; i < FLAPPY_MAX_PILARS; i++) {
            PILAR* p = &g->pilar[i];
            
            if (p != NULL && p->visible) { 
                flappy_draw_pilar(p);

                if (p->point.x < -FLAPPY_GAB_WIDTH) {
                    p->visible = 0;  
                }

                if (g->bird.x >= p->point.x + FLAPPY_PILAR_WIDTH && p->passed == 0) {
                    p->passed = 1; 
                    g->points++;
                }
            } 
        }
        draw_score(g);
        draw_gravity(g);
    } else {  
        draw_string(30, 20, "FLAPPY FLIPPER - BIRD"); 
    }

    // Draw Flappy
    for (int x = 0; x < g->bird.width; x++)
        for (int y = 0; y < g->bird.height; y++) 
        {
            int px = 0; 
            if (g->gravity < -1.1)
                px = bird_array[x][y];
            else { 
                if (g->gravity < -0.5)
                    px = bird_mid[x][y];
                else
                    px = bird_up[x][y];
            }

            
            if (px)
                flipper_pixel_set(g->bird.x + x, g->bird.y + y);
        }
}



void flappy_add_pilar(GAME *g) { 
    PILAR pilar; 
    pilar.visible = 1; 
    pilar.passed = 0;
    pilar.height = (rand() % (FL_LCD_HEIGHT - FLAPPY_GAB_HEIGHT)) + 1; 
    pilar.point.x = FL_LCD_WIDTH + 10; 
    pilar.point.y = 0; 
    g->pilar_len++;
    g->pilar[g->pilar_len % FLAPPY_MAX_PILARS] = pilar;
}


void flappy_game_tick(GAME *g) {
    if (g->active)
    {
        for (int i = 0; i < FLAPPY_MAX_PILARS; i++) {
            PILAR* p = &g->pilar[i];

            if (p != NULL) 
                p->point.x -= 1;
        }

        PILAR *last_pilar = &g->pilar[g->pilar_len % FLAPPY_MAX_PILARS]; 
        if (last_pilar->point.x == (FL_LCD_WIDTH - FLAPPY_GAB_WIDTH))
            flappy_add_pilar(g);

        g->gravity += FLAPPY_GRAVITY_TICK;
        g->bird.y += g->gravity;
    }
}

bool flappy_bird_check(GAME *g) {
    // Upper / Lower
    if (g->bird.y < 0 || (g->bird.y + g->bird.height > FL_LCD_HEIGHT))
        return true;

    return flappy_check_pilar(g);
}

void flappy_init(GAME* g) {
    BIRD bird; 
    bird.width = 15; 
    bird.height = 11; 
    bird.x = 5; 
    bird.y = 32;  
    memcpy(&bird.data, bird_array, sizeof(bird_array));

    g->points = 0; 
    g->pilar_len = 0;
    g->gravity = 0;
    g->active = 0; 
    g->bird = bird;
    memset(g->pilar, 0, sizeof(g->pilar));
    flappy_add_pilar(g); 
}

int main(int argc, char* argv[]) {
    if (!flipper_init(0))
        return 1;

    GAME game; 
    flappy_init(&game); 

    bool quit = false;
    while (!quit) {
        flipper_gpio_update(); 
        flappy_game_tick(&game); 

        if (flappy_bird_check(&game)){
            flappy_init(&game); 
        }
        if (flipper_gpio_get(FL_GPIO_SIMULATOR_EXIT)) {
            quit = true;
            break;
        }

        if (flipper_gpio_get(FL_GPIO_BUTTON_ENTER)) {
            if (game.active == 0)
                game.active = 1;
            game.gravity = -1.1;
        }

        flipper_pixel_reset();

        // draw border
        draw_border();
        flappy_draw(&game);
        
        flipper_lcd_update();
        flipper_lcd_constant_fps();
    }

    return 0;
}