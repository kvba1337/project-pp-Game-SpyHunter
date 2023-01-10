#include"functions.h"

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

void drawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};

void drawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};

void drawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		drawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

void drawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	drawLine(screen, x, y, k, 0, 1, outlineColor);
	drawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	drawLine(screen, x, y, l, 1, 0, outlineColor);
	drawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		drawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

int initialConfiguration(SDL* sdl, Colors* color) {
	// Initialitize SDL
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

	// Load bitmap images and charset
	sdl->charset = loadBitmap("assets/cs8x8.bmp", true, 0x000000);
	sdl->carSurface = loadBitmap("assets/car.bmp", false, 0x00000000);
	sdl->tree = loadBitmap("assets/tree.bmp", false, 0x00000000);
	sdl->line = loadBitmap("assets/line.bmp", false, 0x00000000);

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

void calculateData(Game* game) {
	// Calculate elapsed time and distance
	game->time.deltaTime = (game->time.tick2 - game->time.tick1) * 0.001;
	game->time.tick1 = game->time.tick2;
	game->time.worldTime += game->time.deltaTime;

	// If car is on the road increase total distance
	if ((game->car.posX > ROAD_WIDTH) && (game->car.posX < (2 * ROAD_WIDTH))) 
		game->info.distance += game->car.speed * game->time.deltaTime;

	// Calculate score (add every 50 points)
	int tempScore = (int)game->info.distance * SCORE_RATIO;
	if(tempScore%50==0) game->info.score = tempScore;
}

void drawInterface(SDL sdl, Game game, Colors color) {
	drawRectangle(sdl.screen, INTERFACE_POS, INTERFACE_POS, INTERFACE_WIDTH, INTERFACE_HEIGHT, color.czerwony, color.niebieski);

	sprintf(color.text, "Jakub Falk (193252)  wynik = %i  czas trwania = %.1lf s  zycia: %i", game.info.score, game.time.worldTime, game.car.life);
	drawString(sdl.screen, CENTER_TEXT, INTERFACE_TEXT_HEIGHT, color.text, sdl.charset);

	sprintf(color.text, "\030 \031 \032 \033 - sterowanie N - nowa gra S/L - stan gry F - koniec gry ESC - wyjscie");
	drawString(sdl.screen, CENTER_TEXT, INTERFACE_TEXT_HEIGHT+16, color.text, sdl.charset);

	drawRectangle(sdl.screen, BOX_POS_X, BOX_POS_Y, BOX_SIZE, BOX_SIZE, color.czerwony, color.niebieski);

	sprintf(color.text, "a, b, c,");
	drawString(sdl.screen, BOX_POS_X + 5, BOX_POS_Y + 10, color.text, sdl.charset);
	sprintf(color.text, "d, e, f,");
	drawString(sdl.screen, BOX_POS_X + 5, BOX_POS_Y + 25, color.text, sdl.charset);sprintf(color.text, "d, e, f");
	sprintf(color.text, "g, i, m,");
	drawString(sdl.screen, BOX_POS_X + 5, BOX_POS_Y + 40, color.text, sdl.charset);
	sprintf(color.text, "x, x, x");
	drawString(sdl.screen, BOX_POS_X + 5, BOX_POS_Y + 55, color.text, sdl.charset);
}

void displaySurface(SDL* sdl) {
	SDL_UpdateTexture(sdl->scrtex, NULL, sdl->screen->pixels, sdl->screen->pitch);
	SDL_RenderCopy(sdl->renderer, sdl->scrtex, NULL, NULL);
	SDL_RenderPresent(sdl->renderer);
}

void processEvents(SDL* sdl, Game* game, Colors color) {
	while (SDL_PollEvent(&sdl->event)) {
		switch (sdl->event.type) {
			case SDL_KEYDOWN:
				switch (sdl->event.key.keysym.sym) {
					case SDLK_ESCAPE:
						game->status.quit = true;
						break;
					case SDLK_UP:
						game->car.speed = CAR_ACCELERATION;
						break;
					case SDLK_DOWN:
						game->car.speed = CAR_SLOWDOWN;
						break;
					case SDLK_LEFT:
						game->car.turn = -CAR_TURN_RATIO;
						break;
					case SDLK_RIGHT:
						game->car.turn = CAR_TURN_RATIO;
						break;
					case SDLK_n:
						newGame(sdl, game);
						break;
					case SDLK_p:
						game->status.pause = !(game->status.pause);
						game->status.loadError = false;
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
						game->car.speed = CAR_DEFAULT_SPEED;
						break;
					case SDLK_DOWN:
						game->car.speed = CAR_DEFAULT_SPEED;
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
}

void cleanupAndQuit(SDL* sdl) {
	SDL_DestroyTexture(sdl->scrtex);
	SDL_DestroyWindow(sdl->window);
	SDL_DestroyRenderer(sdl->renderer);
	SDL_FreeSurface(sdl->screen);
	SDL_FreeSurface(sdl->charset);
	SDL_FreeSurface(sdl->carSurface);
	SDL_FreeSurface(sdl->tree);
	SDL_FreeSurface(sdl->line);
	SDL_Quit();
}

void drawRoadAndCar(SDL* sdl, Game* game, Colors* color) {
	// Draw road
	drawRectangle(sdl->screen, ROAD_POS_X, ROAD_POS_Y, ROAD_WIDTH, ROAD_HEIGHT, color->czarny, color->czarny);

	// Set position of trees
	game->tree.posY += game->car.speed * game->time.deltaTime * TREES_MOVING_RATIO;
	if (game->tree.posY > SCREEN_HEIGHT+TREE_DISSAPEAR_DISTANCE) {
		game->tree.posY = ROAD_POS_Y;
		game->tree.posX = rand() % 161 + 20; // Random X coordinate of new tree
	}

	// Draw trees and lines
	drawSurface(sdl->screen, sdl->tree, game->tree.posX, game->tree.posY);
	drawSurface(sdl->screen, sdl->tree, game->tree.posX+DISTANCE_BETWEEN_TREES, game->tree.posY - game->tree.posX);
	for (int i = -600; i <= 600; i += 100)
		drawSurface(sdl->screen, sdl->line, SCREEN_WIDTH / 2, game->tree.posY+i);

	// Draw a car
	drawSurface(sdl->screen, sdl->carSurface, game->car.posX, game->car.posY);
}

void newGame(SDL* sdl, Game* game) {
	game->status.finish = false;
	game->time.worldTime = 0;
	game->time.tick1 = SDL_GetTicks();
	game->info.score = 0;
	game->info.distance = 0;
	game->car.speed = CAR_DEFAULT_SPEED;
	game->car.posX = DEFAULT_POS_CAR_X;
	game->car.posY = DEFAULT_POS_CAR_Y;
	game->car.life = INFINITY_LIFE;
	game->tree.posX = rand() % 100 + 20;
	game->tree.posY = 0;
}

void setCarInfo(Game* game) {
	game->time.tick2 = SDL_GetTicks();

	// Additional lifes 
	if ((game->time.worldTime > INFINITY_LIFE_TIME) && (game->time.worldTime < (INFINITY_LIFE_TIME + 1))) {
		if (game->info.score > PTS_TO_GET_EXTRA_LIFES) {
			game->car.life = 3;
		}
		else
			game->car.life = 1;
	}

	// Setting posision X of the car
	game->car.posX += game->car.turn;

	// Setting position Y of the car
	if (game->car.speed > CAR_DEFAULT_SPEED) game->car.posY -= game->car.speed * CAR_SLOWDOWN_RATIO; 
	if (game->car.speed < CAR_DEFAULT_SPEED) game->car.posY += game->car.speed * CAR_ACCELERATION_RATIO;

	// Blocking off-road driving
	if ((game->car.posX < (ROAD_WIDTH - ROADSIDE)) || (game->car.posX > (2 * ROAD_WIDTH) + ROADSIDE)) {
		game->car.life--;
		game->car.posX = DEFAULT_POS_CAR_X;
		game->car.posY = DEFAULT_POS_CAR_Y;
	}

	if (game->car.posY < INTERFACE_TOP_BORDER) game->car.posY = INTERFACE_TOP_BORDER;
	if (game->car.posY > SCREEN_HEIGHT - BOTTOM_BORDER) game->car.posY = SCREEN_HEIGHT - BOTTOM_BORDER;
	if (game->car.life == 0) game->status.finish = true;
}

void finishGame(SDL sdl, Game game, Colors color) {
	SDL_FillRect(sdl.screen, NULL, color.czarny);

	sprintf(color.text, "KONIEC GRY");
	drawString(sdl.screen, CENTER_TEXT, FINISH_INFO_POS_Y, color.text, sdl.charset);

	sprintf(color.text, "WYNIK = %i  CZAS TRWANIA = %.1lf s", game.info.score, game.time.worldTime);
	drawString(sdl.screen, CENTER_TEXT, FINISH_INFO_POS_Y+50, color.text, sdl.charset);

	sprintf(color.text, "ABY ROZPOCZAC NOWA GRE WSCINIJ KLAWISZ 'N'");
	drawString(sdl.screen, CENTER_TEXT, FINISH_INFO_POS_Y+100, color.text, sdl.charset);

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
	drawString(sdl.screen, INTERFACE_TEXT_HEIGHT, PAUSE_POS_Y - 15, color.text, sdl.charset);
	displaySurface(&sdl);
}

bool findFiles(char files[][25], int* fileNumber) {
	int counter = 0;
	HANDLE findFile;
	WIN32_FIND_DATAA findData;
	findFile = FindFirstFileA("./saves/*.txt", &findData);

	if (findFile == INVALID_HANDLE_VALUE) {
		printf("Brak plikow do wczytania\n");
		return false;
	}

	do {
		strcpy(files[counter++], findData.cFileName);
	} while (FindNextFileA(findFile, &findData));

	*fileNumber = counter;
	FindClose(findFile);
	return true;
}

void showSaves(SDL sdl, Game* game, Colors color) {
	char files[25][25];
	int fileNumber = 0;
	if(findFiles(files, &fileNumber) == false) {
		game->status.loadError = true;
		game->status.pause = true;
		game->status.load = false;
		return;
	}
	SDL_FillRect(sdl.screen, NULL, color.czarny);

	sprintf(color.text, "ZEBY WYBRAC STAN GRY WCISNIJ ODPOWIEDNI NUMER NA KLAWIATURZE ||| ESC - ANULUJ");
	drawString(sdl.screen, CENTER_TEXT, SAVES_INFO_POS_Y-40, color.text, sdl.charset);

	sprintf(color.text, "ZAPISY STANOW GRY:");
	drawString(sdl.screen, CENTER_TEXT, SAVES_INFO_POS_Y, color.text, sdl.charset);

	for (int i = 0; i < fileNumber; i++) {
		sprintf(color.text, "%i: %s", i, files[i]);
		drawString(sdl.screen, CENTER_TEXT, ((SAVES_INFO_POS_Y+50) + (i * 50)), color.text, sdl.charset);
	}

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
					case SDLK_5:
						fileNumber = 5;
						break;
					case SDLK_6:
						fileNumber = 6;
						break;
					case SDLK_7:
						fileNumber = 7;
						break;
					case SDLK_ESCAPE:
						game->status.load = false;
						break;
				}
		}
	}

	if (fileNumber != -1) {
		loadGame(game, files[fileNumber]);
		game->status.load = false;
		game->status.loadError = false;
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
	fscanf_s(file, "%i %lf %lf %i", &score, &distance, &worldTime, &life);

	game->time.tick1 = SDL_GetTicks();
	game->info.score = score;
	game->info.distance = distance;
	game->time.worldTime = worldTime;
	game->car.life = life;
	game->car.posX = DEFAULT_POS_CAR_X;
	game->car.posY = DEFAULT_POS_CAR_Y;

	fclose(file);
}

void checkGameStatus(SDL* sdl, Game* game, Colors color) {
	while (1) {
		processEvents(sdl, game, color);

		if (game->status.pause) {
			game->time.tick1 = SDL_GetTicks();
			sprintf(color.text, "PAUZA");
			drawString(sdl->screen, PAUSE_POS_X, PAUSE_POS_Y, color.text, sdl->charset);
			if (game->status.loadError) {
				sprintf(color.text, "BRAK PLIKOW DO WCZYTANIA");
				drawString(sdl->screen, PAUSE_POS_X, PAUSE_POS_Y - 10, color.text, sdl->charset);
			}
			displaySurface(sdl);
			continue;
		}

		if (game->status.finish) {
			finishGame(*sdl, *game, color);
			continue;
		}

		if (game->status.load) {
			showSaves(*sdl, game, color);
			continue;
		}
		else {
			game->status.load = false;
		}
	break;
	}
}