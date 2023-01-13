#pragma once
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<windows.h>
#include<conio.h>
#include<time.h>

using namespace std;

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH			640
#define SCREEN_HEIGHT			480
#define ROAD_WIDTH				SCREEN_WIDTH / 3
#define ROAD_HEIGHT				SCREEN_HEIGHT
#define ROAD_START_POS_X		SCREEN_WIDTH / 3
#define ROAD_START_POS_Y		0
#define ROADSIDE				30
#define TREES_MOVING_RATIO		300
#define CARS_MOVING_RATIO		200
#define CAR_DEFAULT_SPEED		1.0
#define CAR_ACCELERATION		2.0
#define CAR_ACCELERATION_RATIO	0.5
#define CAR_SLOWDOWN			0.5
#define CAR_SLOWDOWN_RATIO		0.1
#define CAR_TURN_RATIO			0.3
#define BACK_CARS_ACCELERATION	-1.5
#define BACK_CARS_SLOWDOWN		2.0
#define DISTANCE_BETWEEN_TREES	450
#define TREE_DISSAPEAR_DISTANCE 120
#define DEFAULT_POS_CAR_X		SCREEN_WIDTH / 2
#define DEFAULT_POS_CAR_Y		SCREEN_HEIGHT - 150
#define MAX_FRONT_CARS_INDEX	1
#define MAX_BACK_CARS_INDEX		3
#define MAX_ENEMY_CARS_INDEX	3
#define MAX_FRIENDLY_CARS_INDEX	1
#define INFINITY_LIFE			100
#define GOD_MODE_TIME			10
#define PTS_TO_GET_EXTRA_LIFES	700
#define DEFAULT_LIFES			1
#define ADDITIONAL_LIFES		3
#define TIME_WITHOUT_POINTS		3
#define SCORE_RATIO				50
#define PAUSE_POS_X				10
#define PAUSE_POS_Y				460
#define CENTER_TEXT				sdl.screen->w / 2 - strlen(color.text) * 8 / 2
#define FINISH_INFO_POS_Y		100
#define SAVES_INFO_POS_Y		50
#define INTERFACE_HEIGHT		36
#define INTERFACE_WIDTH			SCREEN_WIDTH - 8
#define INTERFACE_POS			4
#define INTERFACE_TEXT_HEIGHT	10
#define BOX_SIZE				70
#define BOX_POS_X				SCREEN_WIDTH - 70
#define BOX_POS_Y				SCREEN_HEIGHT - 70
#define CAR_WIDTH				40
#define CAR_HEIGHT				90

struct SDL {
	SDL_Event event;
	SDL_Surface* screen, * charset, * carSurface, * enemyCarSurface, * friendlyCarSurface, * tree, * line;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
};

struct Game {
	struct {
		int tick1;
		int tick2;
		double deltaTime;
		double worldTime;
		int destroyTime;
	} time;
	struct {
		bool quit;
		bool pause;
		bool finish;
		bool load;
		bool destroyFriend;
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
	struct {
		double posX;
		double posY;
	} enemyCar[4];
	struct {
		double posX;
		double posY;
	} friendlyCar[2];
};

struct Colors {
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
void drawRoadAndCars(SDL* sdl, Game* game, Colors* color);

// Start a new game
void newGame(SDL* sdl, Game* game);

// Set life and current position of the car
void updateCarInfo(Game* game);

// Finish game
void finishGame(SDL sdl, Game game, Colors color);

// Save the current game state
void saveGame(SDL sdl, Game game, Colors color);

// Load game state chosen by player
void loadGame(SDL* sdl, Game* game, Colors color);

// Check game status
void checkGameStatus(SDL* sdl, Game* game, Colors color);

// Set bot's cars starting position
void initBotsCars(Game* game);

// Set bot's cars position driving from front
void initFrontCars(Game* game, double* botPosX, double* botPosY, double* frontCarDissapearPos);

// Set current position of the bot's cars
void updateBotsCarsInfo(Game* game);

// Check status of cars collision
void checkCarCollisionStatus(Game* game);

// Check if the cars have collided
void checkCollision(Game* game, double playerPosX, double playerPosY, double botPosX, double botPosY, const char* botStatus);

// Randomize new car positions
void randomizeCarsPositions(Game* game);

// Check if bot's cars are spawning on each other
bool checkDoublingPosition(Game* game, int newPosX, int newPosY);