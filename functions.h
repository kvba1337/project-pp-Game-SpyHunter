#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<windows.h>
#include<conio.h>
#include<time.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH			640
#define SCREEN_HEIGHT			480
#define ROAD_WIDTH				SCREEN_WIDTH / 3
#define ROAD_HEIGHT				SCREEN_HEIGHT
#define ROAD_POS_X				SCREEN_WIDTH / 3
#define ROAD_POS_Y				0
#define ROADSIDE				50
#define BOTTOM_BORDER			30
#define TREES_MOVING_RATIO		100
#define DISTANCE_BETWEEN_TREES	450
#define TREE_DISSAPEAR_DISTANCE 120
#define DEFAULT_POS_CAR_X		SCREEN_WIDTH / 2
#define DEFAULT_POS_CAR_Y		SCREEN_HEIGHT - 50
#define INFINITY_LIFE			100
#define INFINITY_LIFE_TIME		10
#define PTS_TO_GET_EXTRA_LIFES	700
#define CAR_DEFAULT_SPEED		1.0
#define CAR_ACCELERATION		2.0
#define CAR_ACCELERATION_RATIO	0.5
#define CAR_SLOWDOWN			0.5
#define CAR_SLOWDOWN_RATIO		0.1
#define CAR_TURN_RATIO			0.3
#define PAUSE_POS_X				10
#define PAUSE_POS_Y				460
#define CENTER_TEXT				sdl.screen->w / 2 - strlen(color.text) * 8 / 2
#define FINISH_INFO_POS_Y		100
#define SAVES_INFO_POS_Y		50
#define SCORE_RATIO				50
#define INTERFACE_TOP_BORDER	80
#define INTERFACE_HEIGHT		36
#define INTERFACE_WIDTH			SCREEN_WIDTH - 8
#define INTERFACE_POS			4
#define INTERFACE_TEXT_HEIGHT	10
#define BOX_SIZE				70
#define BOX_POS_X				SCREEN_WIDTH - 70
#define BOX_POS_Y				SCREEN_HEIGHT - 70

typedef struct SDL {
	SDL_Event event;
	SDL_Surface* screen, * charset, * carSurface, * tree, * line;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
};

typedef struct Game {
	struct {
		int tick1;
		int tick2;
		double deltaTime;
		double worldTime;
	} time;
	struct {
		bool quit;
		bool pause;
		bool finish;
		bool load;
		bool save;
		bool loadError;
	} status;
	struct {
		double distance;
		int score;
	} info;
	struct {
		double posX;
		double posY;
	} tree;
	struct {
		double posX;
		double posY;
		double turn;
		double speed;
		int life;
	} car;
};

typedef struct Colors {
	char text[128];
	int czarny;
	int bialy;
	int zielony;
	int czerwony;
	int niebieski;
};

// Drawing a string
void drawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset);

// Drawing a surface (images)
void drawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y);

// Drawing a single pixel
void drawPixel(SDL_Surface* surface, int x, int y, Uint32 color);

// Drawing a simple line
void drawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);

// Drawing a rectangle
void drawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor);

// Initialize SDL and set up a window, renderer, surfaces and textures
int initialConfiguration(SDL* sdl, Colors* color);

// Loading bitmap images
SDL_Surface* loadBitmap(const char* filename, bool colorKey, Uint32 colorKeyValue);

// Calculate elapsed time, distance travelled and score
void calculateData(Game* game);

// Draw the interface
void drawInterface(SDL sdl, Game game, Colors color);

// Display the surface on the window
void displaySurface(SDL* sdl);

// Handling events (keyboard inputs)
void processEvents(SDL* sdl, Game* game, Colors color);

// Clean up resources and shut down SDL
void cleanupAndQuit(SDL* sdl);

// Draw the road and a car
void drawRoadAndCar(SDL* sdl, Game* game, Colors* color);

// Start a new game
void newGame(SDL* sdl, Game* game);

// Set life and current position of the car
void setCarInfo(Game* game);

// Finish game
void finishGame(SDL sdl, Game game, Colors color);

// Save the current game state
void saveGame(SDL sdl, Game game, Colors color);

// Find saved game states in /saves directory
bool findFiles(char files[][25], int* fileNumber);

// Display saved game states on the screen
void showSaves(SDL sdl, Game* game, Colors color);

// Load game state chosen by player
void loadGame(Game* game, char* filename);