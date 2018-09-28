#ifndef MAIN_H
#define MAIN_H

static int up;
static int down;
static int left;
static int right;

static bool running = 1;

static SDL_Window* window;
static SDL_Renderer* renderer;
static SDL_Texture** dungeonTiles;

extern SDL_Texture** computer__loadSpritesheet(char *path, int spriteWidth, int spriteHeight);
extern void computer__renderSprite(SDL_Texture* sprite, int x, int y);

extern void computer__init();
extern void computer__update();

#endif