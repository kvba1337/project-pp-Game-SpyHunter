#pragma once
#include"configuration.h"
// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
void drawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
void drawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};

// rysowanie pojedynczego pixela
void drawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};

// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
void drawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		drawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

// rysowanie prostokπta o d≥ugoúci bokÛw l i k
void drawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	drawLine(screen, x, y, k, 0, 1, outlineColor);
	drawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	drawLine(screen, x, y, l, 1, 0, outlineColor);
	drawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		drawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

// Initialize SDL and create the window, renderer, surface and texture
int initialConfiguration(SDL* sdl, Colors* color) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdl->window, &sdl->renderer) != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(sdl->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(sdl->renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(sdl->window, "SpyHunter, Jakub Falk 193252");
	SDL_ShowCursor(SDL_DISABLE);

	sdl->screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	sdl->scrtex = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	// Load bitmap images
	sdl->charset = loadBitmap("./cs8x8.bmp", true, 0x000000);
	sdl->carSurface = loadBitmap("./car.bmp", false, 0x00000000);
	sdl->tree = loadBitmap("./tree.bmp", false, 0x00000000);
	sdl->line = loadBitmap("./line.bmp", false, 0x00000000);

	// Set color keys
	color->czarny = SDL_MapRGB(sdl->screen->format, 0x00, 0x00, 0x00);
	color->bialy = SDL_MapRGB(sdl->screen->format, 0xFF, 0xFF, 0xFF);
	color->zielony = SDL_MapRGB(sdl->screen->format, 0x1F, 0x2E, 0x2B);
	color->czerwony = SDL_MapRGB(sdl->screen->format, 0xFF, 0x00, 0x00);
	color->niebieski = SDL_MapRGB(sdl->screen->format, 0x11, 0x11, 0xCC);

	// Initialitize the pseudo-random number generator
	srand(time(NULL));
	return 0;
}

// Loading bitmap images
SDL_Surface* loadBitmap(const char* filename, bool colorKey, Uint32 colorKeyValue) {
	SDL_Surface* bitmap = SDL_LoadBMP(filename);
	if (bitmap == NULL) {
		printf("SDL_LoadBMP(%s) error: %s\n", filename, SDL_GetError());
		return NULL;
	}

	if (colorKey) {
		SDL_SetColorKey(bitmap, true, colorKeyValue);
	}

	return bitmap;
}

// Calculate elapsed time, distance travelled, frame rate and score
void calculateData(Game* game) {
	// Calculate elapsed time and distance
	game->time.deltaTime = (game->time.tick2 - game->time.tick1) * 0.001;
	game->time.tick1 = game->time.tick2;
	game->time.worldTime += game->time.deltaTime;
	if (game->car.posX > SCREEN_WIDTH / 3 && game->car.posX < 2 * SCREEN_WIDTH / 3) game->info.distance += game->info.speed * game->time.deltaTime;

	// Calculate frame rate
	game->frames.fpsTimer += game->time.deltaTime;
	if (game->frames.fpsTimer > SECONDS_BETWEEN_REFRESH) {
		game->frames.fps = game->frames.framesNumber * REFRESH_RATE;
		game->frames.framesNumber = 0;
		game->frames.fpsTimer -= SECONDS_BETWEEN_REFRESH;
	}

	// Calculate score
	int tempScore = game->info.distance * 50;
	if(tempScore%50==0) game->info.score = tempScore;
}

// Draw the bitmap images and text to the surface
void drawInterface(SDL sdl, Game game, Colors color) {
	// Draw info text
	drawRectangle(sdl.screen, 4, 4, SCREEN_WIDTH - 8, 36, color.czerwony, color.niebieski);

	sprintf(color.text, "Jakub Falk (193252)  wynik = %i  czas trwania = %.1lf s  zycia: %i", game.info.score, game.time.worldTime, game.car.life);
	drawString(sdl.screen, sdl.screen->w / 2 - strlen(color.text) * 8 / 2, 10, color.text, sdl.charset);

	sprintf(color.text, "\030 \031 - predkosc, \032 \033 - skrecanie, n - nowa gra, f - koniec gry, esc - wyjscie");
	drawString(sdl.screen, sdl.screen->w / 2 - strlen(color.text) * 8 / 2, 26, color.text, sdl.charset);

	drawRectangle(sdl.screen, SCREEN_WIDTH-70, SCREEN_HEIGHT-70, 70, 70, color.czerwony, color.niebieski);

	sprintf(color.text, "a, b, c,");
	drawString(sdl.screen, SCREEN_WIDTH - 65, SCREEN_HEIGHT - 60, color.text, sdl.charset);
	sprintf(color.text, "d, e, f,");
	drawString(sdl.screen, SCREEN_WIDTH - 65, SCREEN_HEIGHT - 45, color.text, sdl.charset);sprintf(color.text, "d, e, f");
	sprintf(color.text, "g, i, m,");
	drawString(sdl.screen, SCREEN_WIDTH - 65, SCREEN_HEIGHT - 30, color.text, sdl.charset);
	sprintf(color.text, "x, x, x");
	drawString(sdl.screen, SCREEN_WIDTH - 65, SCREEN_HEIGHT - 15, color.text, sdl.charset);

	// Draw a car
	drawSurface(sdl.screen, sdl.carSurface, game.car.posX, game.car.posY);
}

// Display the surface on the window
void displaySurface(SDL* sdl) {
	SDL_UpdateTexture(sdl->scrtex, NULL, sdl->screen->pixels, sdl->screen->pitch);
	//SDL_RenderClear(renderer);
	SDL_RenderCopy(sdl->renderer, sdl->scrtex, NULL, NULL);
	SDL_RenderPresent(sdl->renderer);
}

// Handling events
void processEvents(SDL* sdl, Game* game, Colors color) {
	while (SDL_PollEvent(&sdl->event)) {
		switch (sdl->event.type) {
		case SDL_KEYDOWN:
			switch (sdl->event.key.keysym.sym) {
			case SDLK_ESCAPE:
				game->status.quit = true;
				break;
			case SDLK_UP:
				game->info.speed = 2;
				break;
			case SDLK_DOWN:
				game->info.speed = 0.5;
				break;
			case SDLK_LEFT:
				game->car.turn = -0.3;
				break;
			case SDLK_RIGHT:
				game->car.turn = 0.3;
				break;
			case SDLK_n:
				newGame(sdl, game);
				break;
			case SDLK_p:
				game->status.pause = !(game->status.pause);
				game->status.error = false;
				break;
			case SDLK_f:
				game->status.finish = true;
				break;
			case SDLK_s:
				saveGame(*sdl, *game, color);
				game->status.pause = true;
				game->status.save = true;
				break;
			case SDLK_l:
				game->status.load = true;
				break;
			}
			break;
		case SDL_KEYUP:
			switch (sdl->event.key.keysym.sym) {
			case SDLK_UP:
				game->info.speed = 1.0;
				break;
			case SDLK_DOWN:
				game->info.speed = 1.0;
				break;
			case SDLK_LEFT:
			case SDLK_RIGHT:
				game->car.turn = 0;
				break;
			}
			break;
		case SDL_QUIT:
			game->status.quit = true;
			break;
		}
	}
	++(game->frames.framesNumber);
}

// Clean up resources and shut down SDL
void cleanupAndQuit(SDL* sdl) {
	SDL_FreeSurface(sdl->screen);
	SDL_DestroyTexture(sdl->scrtex);
	SDL_DestroyWindow(sdl->window);
	SDL_DestroyRenderer(sdl->renderer);
	SDL_FreeSurface(sdl->charset);
	SDL_FreeSurface(sdl->carSurface);
	SDL_FreeSurface(sdl->tree);
	SDL_FreeSurface(sdl->line);
	SDL_Quit();
}

// Draw the road
void drawRoad(SDL* sdl, Game* game, Colors* color) {
	drawRectangle(sdl->screen, SCREEN_WIDTH / 3, 0, SCREEN_WIDTH / 3, SCREEN_HEIGHT, color->czarny, color->czarny);
	game->road.posY += game->info.speed * game->time.deltaTime * 100;

	if (game->road.posY > SCREEN_HEIGHT+120) {
		game->road.posY = 0;
		game->road.posX = rand() % 161 + 20;
	}

	// Draw trees and lines
	drawSurface(sdl->screen, sdl->tree, game->road.posX+450, game->road.posY - game->road.posX);
	drawSurface(sdl->screen, sdl->tree, game->road.posX, game->road.posY);
	for (int i = -600; i <= 600; i += 100) {
		drawSurface(sdl->screen, sdl->line, SCREEN_WIDTH / 2, game->road.posY+i);
	}
}

// Starting new game
void newGame(SDL* sdl, Game* game) {
	game->status.finish = false;
	game->time.tick1 = SDL_GetTicks();
	game->info.speed = 1;
	game->info.distance = 0;
	game->info.score = 0;
	game->time.worldTime = 0;
	game->road.posY = 0;
	game->car.posX = SCREEN_WIDTH / 2;
	game->car.posY = SCREEN_HEIGHT-50;
	game->car.life = 100;
	game->road.posX = 20;
	game->road.posY = 0;
}

// Setting current position of the car
void setCarInfo(Game* game) {
	game->time.tick2 = SDL_GetTicks();

	// Setting life
	if ((game->time.worldTime > 5.00) && (game->time.worldTime < 6.00)) game->car.life = 1;

	// Setting posision X of the car
	game->car.posX += game->car.turn;

	// Setting position Y of the car
	if (game->info.speed > 1) game->car.posY -= game->info.speed * 0.1;
	if (game->info.speed < 1) game->car.posY += game->info.speed * 0.5;

	// Blocking off-road
	if ((game->car.posX < (SCREEN_WIDTH / 3) - 50) || (game->car.posX > (2 * SCREEN_WIDTH / 3) + 50)) {
		game->car.life--;
		game->car.posX = SCREEN_WIDTH / 2;
		game->car.posY = SCREEN_HEIGHT - 50;
	}

	if (game->car.posY < 80) game->car.posY = 80;
	if (game->car.posY > SCREEN_HEIGHT - 30) game->car.posY = SCREEN_HEIGHT - 30;
	if (game->car.life == 0) game->status.finish = true;
}

void finishGame(SDL sdl, Game game, Colors color) {
	SDL_FillRect(sdl.screen, NULL, color.czarny);

	sprintf(color.text, "KONIEC GRY");
	drawString(sdl.screen, sdl.screen->w / 2 - strlen(color.text) * 8 / 2, 100, color.text, sdl.charset);

	sprintf(color.text, "WYNIK = %i  CZAS TRWANIA = %.1lf s", game.info.score, game.time.worldTime);
	drawString(sdl.screen, sdl.screen->w / 2 - strlen(color.text) * 8 / 2, 150, color.text, sdl.charset);

	sprintf(color.text, "ABY ROZPOCZAC NOWA GRE WSCINIJ KLAWISZ 'N'");
	drawString(sdl.screen, sdl.screen->w / 2 - strlen(color.text) * 8 / 2, 200, color.text, sdl.charset);

	// Display the surface on the window
	displaySurface(&sdl);
}

void saveGame(SDL sdl, Game game, Colors color) {
	time_t currentTime = time(NULL);
	tm* timeInfo = localtime(&currentTime);
	char buffer[32];
	char filename[32];

	strftime(buffer, 32, "%d.%m.%Y_%H.%M.%S", timeInfo);

	sprintf(filename, "saves/%s.txt", buffer);

	FILE* file = fopen(filename, "w");

	if (file == NULL) {
		printf("Nie mozna otworzyc pliku");
		return;
	}

	fprintf_s(file, "%i %lf %lf %i", game.info.score, game.info.distance, game.time.worldTime, game.car.life);

	fclose(file);

	sprintf(color.text, "GRA ZAPISANA");
	drawString(sdl.screen, 10, SCREEN_HEIGHT-20, color.text, sdl.charset);
	displaySurface(&sdl);
}

bool findFiles(char files[][25], int* fileNumber) {
	int counter = 0;
	HANDLE hFind;
	WIN32_FIND_DATAA findData;

	hFind = FindFirstFileA("./saves/*.txt", &findData);
	if (hFind == INVALID_HANDLE_VALUE) {
		printf("Brak plikow do wczytania\n");
		return false;
	}

	do {
		strcpy(files[counter], findData.cFileName);
		counter++;
	} while (FindNextFileA(hFind, &findData));

	*fileNumber = counter;
	FindClose(hFind);
}

void showSaves(SDL sdl, Game* game, Colors color) {
	char files[25][25];
	int fileNumber;
	if(findFiles(files, &fileNumber) == false) {
		game->status.error = true;
		game->status.pause = true;
		game->status.load = false;
		return;
	}
	SDL_FillRect(sdl.screen, NULL, color.czarny);

	sprintf(color.text, "ZAPISY STANOW GRY:");
	drawString(sdl.screen, sdl.screen->w / 2 - strlen(color.text) * 8 / 2, 50, color.text, sdl.charset);

	for (int i = 0; i < fileNumber; i++) {
		sprintf(color.text, "%i: %s", i, files[i]);
		drawString(sdl.screen, sdl.screen->w / 2 - strlen(color.text) * 8 / 2, 100 + 50 * i, color.text, sdl.charset);
	}

	sprintf(color.text, "WYBIERZ PLIK KTORY CHCESZ ZALADOWAC");
	drawString(sdl.screen, sdl.screen->w / 2 - strlen(color.text) * 8 / 2, 20 + fileNumber*50, color.text, sdl.charset);

	displaySurface(&sdl);
	fileNumber = -1;
	while (SDL_PollEvent(&sdl.event)) {
		switch (sdl.event.type) {
			case SDL_KEYDOWN:
			switch (sdl.event.key.keysym.sym) {
				case SDLK_0:
					fileNumber = 0;
					break;
				case SDLK_1:
					fileNumber = 1;
					break;
				case SDLK_2:
					fileNumber = 2;
					break;
				case SDLK_3:
					fileNumber = 3;
					break;
				case SDLK_4:
					fileNumber = 4;
					break;
				case SDLK_ESCAPE:
					fileNumber = -1;
					break;
			}
		}
	}

	if (fileNumber != -1) {
		loadGame(game, files[fileNumber]);
		game->status.load = false;
		game->status.error = false;
	}
}

void loadGame(Game* game, char* filename) {
	char buffer[32];
	sprintf(buffer, "saves/%s", filename);

	FILE* file = fopen(buffer, "r");

	if (file == NULL) {
		printf("Nie mozna otworzyc pliku");
		return;
	}

	int score;
	double distance;
	double worldTime;
	int life;
	fscanf(file, "%i %lf %lf %i", &score, &distance, &worldTime, &life);

	game->time.tick1 = SDL_GetTicks();
	game->info.distance = distance;
	game->info.score = score;
	game->time.worldTime = worldTime;
	game->car.posX = SCREEN_WIDTH / 2;
	game->car.posY = SCREEN_HEIGHT - 50;
	game->car.life = life;

	fclose(file);
}