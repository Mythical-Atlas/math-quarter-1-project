/*#include <stdio.h>
#include <stdbool.h>
#include <simple2d.h>

#include "main.h"

static const int UP = 0;
static const int DOWN = 1;
static const int LEFT = 2;
static const int RIGHT = 3;

static bool input[4];*/

/*static S2D_Sprite sprite;*/

/*static void computerMain__draw(int input) {
	S2D_DrawTriangle(
		320,  50, 1, 0, 0, 1,
		540, 430, 0, 1, 0, 1,
		100, 430, 0, 0, 1, 1
	);
}
static bool* computerMain__getInput2() {}

static void computerMain__getInput(S2D_Event event) {
	if(strcmp(event.key, "Up")) {
		if(event.type == S2D_KEY_DOWN) {input[UP] = 1;}
		if(event.type == S2D_KEY_UP) {input[UP] = 0;}
	}
	if(strcmp(event.key, "Down")) {
		if(event.type == S2D_KEY_DOWN) {input[DOWN] = 1;}
		if(event.type == S2D_KEY_UP) {input[DOWN] = 0;}
	}
	if(strcmp(event.key, "Left")) {
		if(event.type == S2D_KEY_DOWN) {input[LEFT] = 1;}
		if(event.type == S2D_KEY_UP) {input[LEFT] = 0;}
	}
	if(strcmp(event.key, "Right")) {
		if(event.type == S2D_KEY_DOWN) {input[RIGHT] = 1;}
		if(event.type == S2D_KEY_UP) {input[RIGHT] = 0;}
	}
}

int main() {
	S2D_Window *window = S2D_CreateWindow(
		"CE Mario",  // title of the window
		320, 240,        // width and height
		NULL, NULL,  // callback function pointers (these can be NULL)
		0                // flags
	);

	window->fps_cap = 60;
	window->on_key = computerMain__getInput;

	S2D_Show(window);

	main__init(computerMain__draw, computerMain__getInput2);

	while(true) {main__update();}

	S2D_Close(window);
	return(0);
}*/