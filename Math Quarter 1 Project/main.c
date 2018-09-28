#include <stdio.h>
#include <stdbool.h>
#include "SDL.h"

//#include "main.h"

#define SDL_MAIN_HANDLED

#define CLOSEST_TILE_X (int)floor(x / TILE_SIZE)
#define CLOSEST_TILE_Y (int)floor(y / TILE_SIZE)
//#define TILE_SIZE 16;

#define STANDING 0
#define RUNNING  1

typedef struct {
	int width;
	int height;
	int **tiles;
	SDL_Texture **spritesheet;

	void (*draw)(Dungeon);
} Dungeon;

int up;
int down;
int left;
int right;

int testDungeon__tempTiles[20][15];/* = { // x and y flipped - tiles[y][x]
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};*/

const int TILE_SIZE = 16;

int           link__xOffset = 8;
int           link__yOffset = 8;
int			  link__w = 16;
int			  link__h = 16;
double        link__x = 64;
double        link__y = 64;
int			  link__dir;
int			  link__frame = 1;
int			  link__timer;
int			  link__state = STANDING;
int			  link__animation = STANDING;
SDL_Texture **link__spritesheet;

int running = 1;

Dungeon testDungeon;

SDL_Window *window;
SDL_Renderer *renderer;
//SDL_Texture** dungeonTiles;

SDL_Texture** computer__loadSpritesheet(char *path, int spriteWidth, int spriteHeight) {
	SDL_Surface* surface = SDL_LoadBMP(path);
	int n = ((surface->w / spriteWidth) * (surface->h / spriteHeight) + 1);
	SDL_Surface** spritesheet = (SDL_Surface**)malloc(sizeof(SDL_Surface*) * n);
	SDL_Texture** texturesheet = (SDL_Texture**)malloc(sizeof(SDL_Texture*) * n);
	int x, y;
	SDL_Rect rect = {0, 0, spriteWidth, spriteHeight};

	for(int i = 0; i < n; i++) {
		spritesheet[i] = SDL_CreateRGBSurface(0, spriteWidth, spriteHeight, 32, 0x00, 0x00, 0x00, 0x00);

		SDL_SetColorKey(spritesheet[i], 1, 0xFF00FF);
		SDL_FillRect(spritesheet[i], 0, 0xFF00FF);

		if(i != 0) { // first sprite (spritesheet[0]) is completely transparent for easy Tiled implementation (added, not included in image)
			x = (i - 1) % (surface->w / spriteWidth);
			y = (i - x) / (surface->w / spriteHeight);

			rect.x = x * spriteWidth;
			rect.y = y * spriteHeight;

			SDL_BlitSurface(surface, &rect, spritesheet[i], NULL);

			texturesheet[i] = SDL_CreateTextureFromSurface(renderer, spritesheet[i]);
		}
	}

	SDL_FreeSurface(surface);

	return(texturesheet);
}
void computer__renderSprite(SDL_Texture* sprite, int x, int y) {
	int w, h;
	
	SDL_QueryTexture(sprite, NULL, NULL, &w, &h);

	SDL_Rect rect = {x, y, w, h};

	SDL_RenderCopy(renderer, sprite, NULL, &rect);
}

void drawWord(SDL_Texture** spritesheet, int index, int x, int y, int w, int h) {
	computer__renderSprite(spritesheet[index],  x,       y);
	computer__renderSprite(spritesheet[index + 1],  x + w,  y);
	computer__renderSprite(spritesheet[index + 2],  x,      y + h);
	computer__renderSprite(spritesheet[index + 3],  x + w,  y + h);
}

int* closestTile(double x, double y) {return((int[2]){(int)floor(x / TILE_SIZE), (int)floor(y / TILE_SIZE)});}
bool checkCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {return((x1 >= x2 && x1 < x2 + w2 || x1 + w1 > x2 && x1 + w1 <= x2 + w2) && (y1 >= y2 && y1 < y2 + h2 || y1 + h1 > y2 && y1 + h1 <= y2 + h2));}

double* smallestDistance(int x, int y, int w, int h) {
	double xPos = 0;
	double yPos = 0;

	if(abs(y - ((link__y + link__yOffset) + link__h)) <= abs((y + h) - (link__y + link__yOffset))) {yPos = y - ((link__y + link__yOffset) + link__h);}
	else {yPos = (y + h) - (link__y + link__yOffset);}
	if(abs(x - ((link__x + link__xOffset) + link__w)) <= abs((x + w) - (link__x + link__xOffset))) {xPos = x - ((link__x + link__xOffset) + link__w);}
	else {xPos = (x + w) - (link__x + link__xOffset);}

	if(abs(yPos) <= abs(xPos)) {return((double[3]){xPos, yPos, 1});}
	else {return((double[3]){xPos, yPos, 0});}
}

/* int* tileCollision(Dungeon dungeon, double x, double y, int w, int h) {
	if(w <= TILE_SIZE * 2 && h <= TILE_SIZE * 2) {
		if(testDungeon__tempTiles[(int)floor(y / TILE_SIZE)][(int)floor(x / TILE_SIZE)] == 1) {return((int[]){(int)floor(y / TILE_SIZE), (int)floor(x / TILE_SIZE)});}
		else {return((int[2]){-1, -1});}
	}
	else {
		running = 2;
		return(0);
	}
}
/*int* tileCollision2(double x1, double y1, int w1, int h1, int x2, int y2) {
	if(testDungeon__tempTiles[closestTile(x, y)[1]][closestTile(x, y)[0]] == 1) {
		if(checkCollision(ling__x + x1),
						  y + (y1 << 12),
					  	  w1 << 12,
						  h1 << 12,
						  getTilePos(x, x1 + x2),
						  getTilePos(y, y1 + y2),
						  TILE_SIZE,
						  TILE_SIZE)) {return(1);}}
	}

	return(0);
}*/
/*int* tileResponse(Dungeon dungeon, double x, double y, int w, int h, int x2, int y2, int w2, int h2) {
	int *result = tileCollision(testDungeon, x, y, w, h);

	if(result[0] != -1 && result[1] != -1) {
		int xMove, yMove;

		if(abs((y2 * h2 + h2) - y) <= abs(y2 * h2 - (y + h))) {yMove = (y2 * h2 + h2) - y;}
		else {yMove = y2 * h2 - (y + h);}
		if(abs((x2 * w2 + w2) - x) <= abs(x2 * w2 - (x + w))) {xMove = (x2 * w2 + w2) - x;}
		else {xMove = x2 * w2 - (x + w);}

		if(abs(yMove) <= abs(xMove)) {return((int[2]){0, yMove});}
		else {return((int[2]){xMove, 0});}
	}
	else {return((int[2]){0, 0});}
}
/*void link__tileResponse() {
	int x = link__x;
	int y = link__y;

	bool ul = tileCollision2(testDungeon, link__x, link__y, link__w, link__h);
	bool ur = tileCollision2(testDungeon, link__x + TILE_SIZE, link__y, link__w, link__h);
	bool dl = tileCollision2(testDungeon, link__x, link__y + TILE_SIZE, link__w, link__h);
	bool dr = tileCollision2(testDungeon, link__x + TILE_SIZE, link__y + TILE_SIZE, link__w, link__h);
	
	int *move = (int[2]){0, 0};

	if( ul && !ur && !dl && !dr) {move = tileResponse(testDungeon, link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, CLOSEST_TILE_X,             CLOSEST_TILE_Y, TILE_SIZE, TILE_SIZE);}
	if(!ul &&  ur && !dl && !dr) {move = tileResponse(testDungeon, link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, CLOSEST_TILE_X + TILE_SIZE, CLOSEST_TILE_Y, TILE_SIZE, TILE_SIZE);}
	if(!ul && !ur &&  dl && !dr) {move = tileResponse(testDungeon, link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, CLOSEST_TILE_X,             CLOSEST_TILE_Y + TILE_SIZE, TILE_SIZE, TILE_SIZE);}
	if(!ul && !ur && !dl &&  dr) {move = tileResponse(testDungeon, link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, CLOSEST_TILE_X + TILE_SIZE, CLOSEST_TILE_Y + TILE_SIZE, TILE_SIZE, TILE_SIZE);}
	
	if( ul &&  ur && !dl && !dr) {move = tileResponse(testDungeon, link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, CLOSEST_TILE_X,             CLOSEST_TILE_Y,             TILE_SIZE * 2, TILE_SIZE);}
	if( ul && !ur &&  dl && !dr) {move = tileResponse(testDungeon, link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, CLOSEST_TILE_X,             CLOSEST_TILE_Y,             TILE_SIZE,     TILE_SIZE * 2);}
	if(!ul &&  ur && !dl &&  dr) {move = tileResponse(testDungeon, link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, CLOSEST_TILE_X + TILE_SIZE, CLOSEST_TILE_Y,             TILE_SIZE,     TILE_SIZE * 2);}
	if(!ul && !ur &&  dl &&  dr) {move = tileResponse(testDungeon, link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, CLOSEST_TILE_X,             CLOSEST_TILE_Y * TILE_SIZE, TILE_SIZE * 2, TILE_SIZE);}

	link__x += move[0];
	link__y += move[1];
}*/

void link__tileResponse() {
	bool ul = checkCollision(link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE,             closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE,             TILE_SIZE, TILE_SIZE) && testDungeon__tempTiles[closestTile(link__x + link__xOffset, link__y + link__yOffset)[0]    ][closestTile(link__x + link__xOffset, link__y + link__yOffset)[1]    ] == 1;
	bool ur = checkCollision(link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE + TILE_SIZE, closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE,             TILE_SIZE, TILE_SIZE) && testDungeon__tempTiles[closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] + 1][closestTile(link__x + link__xOffset, link__y + link__yOffset)[1]    ] == 1;
	bool dl = checkCollision(link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE,             closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE + TILE_SIZE, TILE_SIZE, TILE_SIZE) && testDungeon__tempTiles[closestTile(link__x + link__xOffset, link__y + link__yOffset)[0]    ][closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] + 1] == 1;
	bool dr = checkCollision(link__x + link__xOffset, link__y + link__yOffset, link__w, link__h, closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE + TILE_SIZE, closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE + TILE_SIZE, TILE_SIZE, TILE_SIZE) && testDungeon__tempTiles[closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] + 1][closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] + 1] == 1;

	if(ul) {printf("ul = 1\n");}
	else   {printf("ul = 0\n");}
	if(ur) {printf("ur = 1\n");}
	else   {printf("ur = 0\n");}
	if(dl) {printf("dl = 1\n");}
	else   {printf("dl = 0\n");}
	if(dr) {printf("dr = 1\n\n");}
	else   {printf("dr = 0\n");}

	printf("link__x = %d	link__y = %d\n\n", (int)link__x, (int)link__y);

	double *move = (double[2]){0, 0}; // [x][y]
	double *tempMove = (double[3]){0, 0, 0}; // [x][y][index]

	     if( ul && !ur && !dl && !dr) {tempMove = smallestDistance(closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE     , closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE     , 16, 16);}
    else if( ul &&  ur && !dl && !dr) {tempMove = smallestDistance(closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE     , closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE     , 32, 16);}
	else if(!ul &&  ur && !dl && !dr) {tempMove = smallestDistance(closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE + 16, closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE     , 16, 16);}
	else if(!ul &&  ur && !dl &&  dr) {tempMove = smallestDistance(closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE + 16, closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE     , 16, 32);}
	else if(!ul && !ur && !dl &&  dr) {tempMove = smallestDistance(closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE + 16, closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE + 16, 16, 16);}
	else if(!ul && !ur &&  dl &&  dr) {tempMove = smallestDistance(closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE     , closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE + 16, 32, 16);}
	else if(!ul && !ur &&  dl && !dr) {tempMove = smallestDistance(closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE     , closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE + 16, 16, 16);}
	else if( ul && !ur &&  dl && !dr) {tempMove = smallestDistance(closestTile(link__x + link__xOffset, link__y + link__yOffset)[0] * TILE_SIZE     , closestTile(link__x + link__xOffset, link__y + link__yOffset)[1] * TILE_SIZE     , 16, 32);}

	move[(int)tempMove[2]] = tempMove[(int)tempMove[2]];

	link__x += move[0];
	link__y += move[1];
}

void link__init() {
	link__spritesheet = computer__loadSpritesheet("link.bmp", 32, 32);

	int tempTiles[15][20] = { // x and y flipped - tiles[y][x]
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
	};

	for(int ix = 0; ix < 20; ix++) {
		for(int iy = 0; iy < 15; iy++) {
			testDungeon__tempTiles[ix][iy] = tempTiles[iy][ix];
		}
	}
}
void link__update() {
	int xTrig = 0;
	int yTrig = 0;

	if(up) {
		yTrig = -1;
		if(up && !down && !left && !right) {link__dir = 0;}
		if(up && !down && link__dir == 1) {link__dir = 0;}
	}
	if(down) {
		yTrig = 1;
		if(!up && down && !left && !right) {link__dir = 1;}
		if(!up && down && link__dir == 0) {link__dir = 1;}
	}
	if(left) {
		xTrig = -1;
		if(!up && !down && left && !right) {link__dir = 2;}
		if(left && !right && link__dir == 3) {link__dir = 2;}
	}
	if(right) {
		xTrig = 1;
		if(!up && !down && !left && right) {link__dir = 3;}
		if(!left && right && link__dir == 2) {link__dir = 3;}
	}

	if(xTrig != 0 && yTrig != 0) {
		if(xTrig > 0) {link__x += cos(atan(yTrig / xTrig)) * 1.5;}
		else {link__x -= cos(atan(yTrig / xTrig)) * 1.5;}
	}
	else if(yTrig == 0) {link__x += xTrig * 1.5;}

	link__tileResponse();

	if(xTrig != 0 && yTrig != 0) {
		if(xTrig > 0) {link__y += sin(atan(yTrig / xTrig)) * 1.5;}
		else {link__y -= sin(atan(yTrig / xTrig)) * 1.5;}
	}
	else if(xTrig == 0) {link__y += yTrig * 1.5;}

	link__tileResponse();

	if(up || down || left || right) {link__state = RUNNING;}
	else {link__state = STANDING;}
}
void link__draw() {
	int frameIndex = 0;

	if(link__state == STANDING) {
		frameIndex = 0;

		switch(link__dir) {
			case(0): link__frame = 3; break;
			case(1): link__frame = 1; break;
			case(2): link__frame = 4; break;
			case(3): link__frame = 2; break;
		}
	}
	if(link__state == RUNNING) {
		switch(link__dir) {
			case(0): frameIndex = 25; break;
			case(1): frameIndex =  5; break;
			case(2): frameIndex = 15; break;
			case(3): frameIndex = 35; break;
		}

		if(link__timer > 0) {
			link__timer = -1;
			link__frame++;
			if(link__frame > 9) {link__frame = 0;}
		}
		link__timer++;
	}

	computer__renderSprite(link__spritesheet[link__frame + frameIndex], (int)link__x, (int)link__y);
}

void dungeon__draw(Dungeon self) {
	drawWord(self.spritesheet,  1, 0, 0, TILE_SIZE, TILE_SIZE);
	for(int i = 2; i < self.width - 3; i += 2) {drawWord(self.spritesheet, 17, i * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE);}

	drawWord(self.spritesheet,  5, self.width * TILE_SIZE - TILE_SIZE * 2, 0, TILE_SIZE, TILE_SIZE);
	for(int i = self.height - 3; i > 1; i -= 2) {drawWord(self.spritesheet, 25, 0, i * TILE_SIZE, TILE_SIZE, TILE_SIZE);}

	drawWord(self.spritesheet,  9, 0, self.height * TILE_SIZE - TILE_SIZE * 2, TILE_SIZE, TILE_SIZE);
	drawWord(self.spritesheet, 13, self.width * TILE_SIZE - TILE_SIZE * 2, self.height * TILE_SIZE - TILE_SIZE * 2, TILE_SIZE, TILE_SIZE);
}

void computer__init() {
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow(
		"CE Mario",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		320, 240, 0
	);
	renderer = SDL_CreateRenderer(
		window, -1,
		SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC
	);

	//dungeonTiles = computer__loadSpritesheet("dungeonTiles.bmp", 16, 16);
	
	//tempTiles = malloc(20 * 15 * sizeof(int*));
		
	/*testDungeon__tempTiles = (int[15][20]){ // x and y flipped - tiles[y][x]
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
	};*/

	/*for(int ix = 0; ix < 15; ix++) {
		for(int iy = 0; iy < 20; iy++) {
			tempTiles[ix][iy] = tempTiles2[ix][iy];
		}
	}*/

	testDungeon = (Dungeon){
		.width = 20,
		.height = 15,
		.spritesheet = computer__loadSpritesheet("dungeonTiles.bmp", TILE_SIZE, TILE_SIZE),
		.draw = dungeon__draw,
		.tiles = testDungeon__tempTiles
	};

	link__init();
}
void computer__update() {
	SDL_Event event;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_KEYDOWN:
				switch(event.key.keysym.sym) {
					case(SDLK_LEFT): left = 1; break;
					case(SDLK_RIGHT): right = 1; break;
					case(SDLK_UP): up = 1; break;
					case(SDLK_DOWN): down = 1; break;
				}
				break;
			case SDL_KEYUP:
				switch(event.key.keysym.sym) {
					case(SDLK_LEFT): left = 0; break;
					case(SDLK_RIGHT): right = 0; break;
					case(SDLK_UP): up = 0; break;
					case(SDLK_DOWN): down = 0; break;
				}
				break;
			case SDL_QUIT: running = 0; break;
		}
	}

	link__update();

	SDL_RenderClear(renderer);

	testDungeon.draw(testDungeon);
	link__draw();

	/*computer__renderSprite(dungeonTiles[ 1],   0,   0);
	computer__renderSprite(dungeonTiles[ 2],  16,   0);
	computer__renderSprite(dungeonTiles[ 3],   0,  16);
	computer__renderSprite(dungeonTiles[ 4],  16,  16);
	computer__renderSprite(dungeonTiles[ 5], 288,   0);
	computer__renderSprite(dungeonTiles[ 6], 304,   0);
	computer__renderSprite(dungeonTiles[ 7], 288,  16);
	computer__renderSprite(dungeonTiles[ 8], 304,  16);
	computer__renderSprite(dungeonTiles[ 9],   0, 208);
	computer__renderSprite(dungeonTiles[10],  16, 208);
	computer__renderSprite(dungeonTiles[11],   0, 224);
	computer__renderSprite(dungeonTiles[12],  16, 224);
	computer__renderSprite(dungeonTiles[13], 288, 208);
	computer__renderSprite(dungeonTiles[14], 304, 208);
	computer__renderSprite(dungeonTiles[15], 288, 224);
	computer__renderSprite(dungeonTiles[16], 304, 224);*/

	SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
	computer__init();
	
	while(running) {computer__update();}

	return(running);
}