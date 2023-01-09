#pragma once
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>
#include<windows.h>
#include<conio.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define _USE_MATH_DEFINES
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define SECONDS_BETWEEN_REFRESH 0.5
#define REFRESH_RATE 1/SECONDS_BETWEEN_REFRESH

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
		int framesNumber;
		double fpsTimer;
		double fps;
	} frames;
	struct {
		bool quit;
		bool pause;
		bool finish;
		bool load;
		bool save;
		bool error;
	} status;
	struct {
		double distance;
		double speed;
		int score;
	} info;
	struct {
		double posX;
		double posY;
	} road;
	struct {
		double posX;
		double posY;
		double turn;
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

void drawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset);
void drawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y);
void drawPixel(SDL_Surface* surface, int x, int y, Uint32 color);
void drawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);
void drawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor);

int initialConfiguration(SDL* sdl, Colors* color);

SDL_Surface* loadBitmap(const char* filename, bool colorKey, Uint32 colorKeyValue);

void calculateData(Game* game);

void drawInterface(SDL sdl, Game game, Colors color);

void displaySurface(SDL* sdl);

void processEvents(SDL* sdl, Game* game, Colors color);

void cleanupAndQuit(SDL* sdl);

void drawRoad(SDL* sdl, Game* game, Colors* color);

void newGame(SDL* sdl, Game* game);

void setCarInfo(Game* game);

void finishGame(SDL sdl, Game game, Colors color);

void saveGame(SDL sdl, Game game, Colors color);

bool findFiles(char files[][25], int* fileNumber);

void showSaves(SDL sdl, Game* game, Colors color);

void loadGame(Game* game, char* filename);