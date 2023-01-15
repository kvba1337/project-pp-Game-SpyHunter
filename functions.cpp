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
	sdl->enemyCarSurface = loadBitmap("assets/enemyCar.bmp", false, 0x00000000);
	sdl->friendlyCarSurface = loadBitmap("assets/friendlyCar.bmp", false, 0x00000000);

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

	if (colorKey) SDL_SetColorKey(bitmap, true, colorKeyValue);

	return bitmap;
}

void calculateData(Game* game) {
	// Calculate elapsed time and distance
	game->time.deltaTime = (game->time.tick2 - game->time.tick1) * 0.001;
	game->time.tick1 = game->time.tick2;
	game->time.worldTime += game->time.deltaTime;

	// If car is on the road increase total distance
	if ((game->car.posX > ROAD_WIDTH) && (game->car.posX < (2 * ROAD_WIDTH)) && game->status.destroyFriend==false) 
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
	drawString(sdl.screen, BOX_POS_X + 5, BOX_POS_Y + 25, color.text, sdl.charset);
	sprintf(color.text, "g, i, j,");
	drawString(sdl.screen, BOX_POS_X + 5, BOX_POS_Y + 40, color.text, sdl.charset);
	sprintf(color.text, "m");
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
						break;
					case SDLK_f:
						game->status.finish = true;
						break;
					case SDLK_s:
						if (game->status.finish == false) {
							saveGame(*sdl, *game, color);
							game->status.pause = true;
						}
						break;
					case SDLK_l:
						loadGame(sdl, game, color);
						game->status.pause = true;
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
	SDL_FreeSurface(sdl->enemyCarSurface);
	SDL_FreeSurface(sdl->friendlyCarSurface);
	SDL_FreeSurface(sdl->tree);
	SDL_FreeSurface(sdl->line);
	SDL_Quit();
}

void drawRoadAndCars(SDL* sdl, Game* game, Colors* color) {
	// Draw road
	drawRectangle(sdl->screen, ROAD_START_POS_X, ROAD_START_POS_Y, ROAD_WIDTH, ROAD_HEIGHT, color->czarny, color->czarny);

	// Set position of trees
	game->tree.posY += game->car.speed * game->time.deltaTime * TREES_MOVING_RATIO;
	if (game->tree.posY > SCREEN_HEIGHT+TREE_DISSAPEAR_DISTANCE) {
		game->tree.posY = ROAD_START_POS_Y;
		game->tree.posX = rand() % 161 + 20; // Random X coordinate of new tree
	}

	// Draw trees and lines
	drawSurface(sdl->screen, sdl->tree, game->tree.posX, game->tree.posY);
	drawSurface(sdl->screen, sdl->tree, game->tree.posX+DISTANCE_BETWEEN_TREES, game->tree.posY - game->tree.posX);
	for (int i = -600; i <= 600; i += 100)
		drawSurface(sdl->screen, sdl->line, SCREEN_WIDTH / 2, game->tree.posY+i);

	// Draw a player car
	drawSurface(sdl->screen, sdl->carSurface, game->car.posX, game->car.posY);

	// Draw enemy cars
	for (int i = 0; i <= MAX_ENEMY_CARS_INDEX; i++)
		drawSurface(sdl->screen, sdl->enemyCarSurface, game->enemyCar[i].posX, game->enemyCar[i].posY);

	// Draw friendly cars
	for (int i = 0; i <= MAX_FRIENDLY_CARS_INDEX; i++)
		drawSurface(sdl->screen, sdl->friendlyCarSurface, game->friendlyCar[i].posX, game->friendlyCar[i].posY);
}

void newGame(SDL* sdl, Game* game) {
	game->time.destroyTime = 0;
	game->time.worldTime = 0;
	game->time.tick1 = SDL_GetTicks();
	game->status.finish = false;
	game->status.destroyFriend = false;
	game->info.score = 0;
	game->info.distance = 0;
	game->tree.posX = rand() % 100 + 20;
	game->tree.posY = 0;
	game->car.speed = CAR_DEFAULT_SPEED;
	game->car.posX = DEFAULT_POS_CAR_X;
	game->car.posY = DEFAULT_POS_CAR_Y;
	game->car.life = INFINITY_LIFE;
	randomizeCarsPositions(game);
}

void updateCarInfo(Game* game) {
	game->time.tick2 = SDL_GetTicks();

	// Additional lifes 
	if ((game->time.worldTime > GOD_MODE_TIME) && (game->time.worldTime < (GOD_MODE_TIME + 1))) {
		if (game->info.score > PTS_TO_GET_EXTRA_LIFES) {
			game->car.life = ADDITIONAL_LIFES;
		}
		else
			game->car.life = DEFAULT_LIFES;
	}

	// Setting posision X of the car
	game->car.posX += game->car.turn;


	// Losing health for off-road driving 
	if ((game->car.posX < (ROAD_WIDTH - ROADSIDE)) || (game->car.posX > (2 * ROAD_WIDTH) + ROADSIDE)) {
		game->car.life--;
		game->car.posX = DEFAULT_POS_CAR_X;
		game->car.posY = DEFAULT_POS_CAR_Y;
	}

	if (game->car.life <= 0) game->status.finish = true;
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

	if (game.status.load == false) {
		sprintf(filename, "saves/%s.txt", buffer);
	}
	else sprintf(filename, "%s.txt", buffer);

	FILE* file = fopen(filename, "w");

	if (file == NULL) {
		printf("Nie mozna otworzyc pliku\n");
		return;
	}

	fprintf_s(file, "%i %lf %lf %i %lf %lf", game.info.score, game.info.distance, game.time.worldTime, game.car.life, game.car.posX, game.car.posY);

	fclose(file);

	sprintf(color.text, "GRA ZAPISANA");
	drawString(sdl.screen, INTERFACE_TEXT_HEIGHT, PAUSE_POS_Y - 15, color.text, sdl.charset);
	displaySurface(&sdl);
}

void loadGame(SDL* sdl, Game* game, Colors color) {
	OPENFILENAMEA openedFile;
	char fileName[MAX_PATH] = "";
	ZeroMemory(&openedFile, sizeof(openedFile));
	openedFile.lStructSize = sizeof(openedFile);
	openedFile.lpstrFile = fileName;
	openedFile.nMaxFile = MAX_PATH;
	openedFile.lpstrFilter = "Pliki tekstowe (*.txt)\0*.txt\0 Wszystkie pliki (*.*)\0*.*\0";

	if (GetOpenFileNameA(&openedFile) == TRUE) {
		newGame(sdl, game);
		FILE* file = fopen(fileName, "r");

		if (file == NULL) {
			printf("Nie mo¿na otworzyæ pliku");
			return;
		}

		int score;
		double distance;
		double worldTime;
		int life;
		double carPosX;
		double carPosY;

		fscanf_s(file, "%i %lf %lf %i %lf %lf", &score, &distance, &worldTime, &life, &carPosX, &carPosY);
		
		game->time.tick1 = SDL_GetTicks();
		game->info.score = score;
		game->info.distance = distance;
		game->time.worldTime = worldTime;
		game->car.life = life;
		game->car.posX = carPosX;
		game->car.posY = carPosY;

		fclose(file);
		game->status.finish = false;
		game->status.load = true;
	}
}

void checkGameStatus(SDL* sdl, Game* game, Colors color) {
	while (1) {
		processEvents(sdl, game, color);
		if (game->status.pause) {
			game->time.tick1 = SDL_GetTicks();
			sprintf(color.text, "PAUZA");
			drawString(sdl->screen, PAUSE_POS_X, PAUSE_POS_Y, color.text, sdl->charset);
			displaySurface(sdl);
			continue;
		}
		if (game->status.finish) {
			finishGame(*sdl, *game, color);
			continue;
		}
		break;
	}
}

void initBotsCars(Game* game) {
	// Cars from front
	for (int CAR_INDEX = 0; CAR_INDEX <= MAX_FRONT_CARS_INDEX; CAR_INDEX++) {
		double frontCarDissapearPos = rand() % 100 + SCREEN_HEIGHT;

		// Initialitize enemy cars
		initFrontCars(game, &game->enemyCar[CAR_INDEX].posX, &game->enemyCar[CAR_INDEX].posY, &frontCarDissapearPos);

		// Initialitize friendly cars
		initFrontCars(game, &game->friendlyCar[CAR_INDEX].posX, &game->friendlyCar[CAR_INDEX].posY, &frontCarDissapearPos);
	}

	// Cars from back
	for (int i = MAX_FRONT_CARS_INDEX+1; i <= MAX_BACK_CARS_INDEX; i++) {
		int backCarDissapearPos = rand() % 100 - 100;
		if (game->enemyCar[i].posY < backCarDissapearPos) {
			game->enemyCar[i].posX = rand() % ROAD_WIDTH + ROAD_WIDTH;
			game->enemyCar[i].posY = rand() % (3 * SCREEN_HEIGHT) + SCREEN_HEIGHT;
		}
	}
}

void initFrontCars(Game* game, double* botPosX, double* botPosY, double* frontCarDissapearPos) {
	if (*botPosY > *frontCarDissapearPos) {
		double newPosX = rand() % ROAD_WIDTH + ROAD_WIDTH;
		double newPosY = rand() % (3 * SCREEN_HEIGHT) - 1500;
		while (checkDoublingPosition(game, newPosX, newPosY)) {
			newPosX = rand() % ROAD_WIDTH + ROAD_WIDTH;
			newPosY = rand() % (3 * SCREEN_HEIGHT) - 1500;
		}
		*botPosX = newPosX;
		*botPosY = newPosY;
	}
}

void updateBotsCarsInfo(Game* game) {
	// Front cars
	for (int i = 0; i <= MAX_FRONT_CARS_INDEX; i++) {
		game->enemyCar[i].posY += game->car.speed * game->time.deltaTime * CARS_MOVING_RATIO;
		game->friendlyCar[i].posY += game->car.speed * game->time.deltaTime * CARS_MOVING_RATIO;
	}

	// Back cars
	for (int i = MAX_FRONT_CARS_INDEX+1; i <= MAX_BACK_CARS_INDEX; i++) {
		// Adapting cars speed to the envirnoment
		int tempSpeed= CAR_DEFAULT_SPEED;
		if (game->car.speed > CAR_DEFAULT_SPEED) tempSpeed = BACK_CARS_ACCELERATION;
		else if (game->car.speed < CAR_DEFAULT_SPEED) tempSpeed = BACK_CARS_SLOWDOWN;
		else tempSpeed = CAR_DEFAULT_SPEED;

		game->enemyCar[i].posY -= tempSpeed * game->time.deltaTime * CARS_MOVING_RATIO/2;
	}
}

void checkCarCollisionStatus(Game* game) {
	for (int ENEMY_INDEX = 0; ENEMY_INDEX <= MAX_ENEMY_CARS_INDEX; ENEMY_INDEX++) {
		for (int FRIEND_INDEX = 0; FRIEND_INDEX <= MAX_FRIENDLY_CARS_INDEX; FRIEND_INDEX++){
			double playerPosX = game->car.posX;
			double playerPosY = game->car.posY;
			double enemyPosX = game->enemyCar[ENEMY_INDEX].posX;
			double enemyPosY = game->enemyCar[ENEMY_INDEX].posY;
			double friendPosX = game->friendlyCar[FRIEND_INDEX].posX;
			double friendPosY = game->friendlyCar[FRIEND_INDEX].posY;

			// Check if TIME_WITHOUT_POINTS has elapsed since the destruction of the friendly car
			if (game->time.worldTime - game->time.destroyTime < TIME_WITHOUT_POINTS && game->time.worldTime > TIME_WITHOUT_POINTS) 
				game->status.destroyFriend = true;
			else 
				game->status.destroyFriend = false;

			// Check collision with enemy cars
			checkCollision(game, playerPosX, playerPosY, enemyPosX, enemyPosY, "enemy");
			
			// Check collision with friendly cars
			checkCollision(game, playerPosX, playerPosY, friendPosX, friendPosY, "friend");
		}
	}
}

void checkCollision(Game* game, double playerPosX, double playerPosY, double botPosX, double botPosY, const char* botStatus) {
	if (abs(playerPosX - botPosX) < CAR_WIDTH && abs(playerPosY - botPosY) < CAR_HEIGHT) {
		if (playerPosX > botPosX || playerPosX < botPosX) {
			game->car.posX = DEFAULT_POS_CAR_X;
			game->car.life--;
			randomizeCarsPositions(game);
			if (botStatus == "friend") game->time.destroyTime = game->time.worldTime;
		}
	}
}

void randomizeCarsPositions(Game* game) {
	// Front cars
	for (int i = 0; i <= MAX_FRONT_CARS_INDEX; i++) {
		int newPosX = rand() % ROAD_WIDTH + ROAD_WIDTH;
		int newPosY = rand() % (SCREEN_HEIGHT + 520) - 1500;
		while (checkDoublingPosition(game, newPosX, newPosY)) {
			newPosX = rand() % ROAD_WIDTH + ROAD_WIDTH;
			newPosY = rand() % (SCREEN_HEIGHT + 520) - 1500;
		}
		game->enemyCar[i].posX = newPosX;
		game->enemyCar[i].posY = newPosY;
		game->friendlyCar[i].posX = newPosX;
		game->friendlyCar[i].posY = newPosY;
	}

	// Back cars
	for (int i = MAX_FRONT_CARS_INDEX + 1; i <= MAX_BACK_CARS_INDEX; i++) {
		int newPosX = rand() % ROAD_WIDTH + ROAD_WIDTH;
		int newPosY = rand() % (3 * SCREEN_HEIGHT) + (2 * SCREEN_HEIGHT);
		while (checkDoublingPosition(game, newPosX, newPosY)) {
			newPosX = rand() % ROAD_WIDTH + ROAD_WIDTH;
			newPosY = rand() % (3 * SCREEN_HEIGHT) + (2 * SCREEN_HEIGHT);
		}
		game->enemyCar[i].posX = newPosX;
		game->enemyCar[i].posY = newPosY;
	}
}

bool checkDoublingPosition(Game* game, int newPosX, int newPosY) {
	for (int i = 0; i <= MAX_ENEMY_CARS_INDEX; i++) {
		for (int j = 0; j <= MAX_FRIENDLY_CARS_INDEX; j++) {
			if (abs(newPosX - game->enemyCar[i].posX) < CAR_WIDTH && abs(newPosY - game->enemyCar[i].posY) < CAR_HEIGHT)
				return true;

			if (abs(newPosX - game->friendlyCar[j].posX) < CAR_WIDTH && abs(newPosY - game->friendlyCar[j].posY) < CAR_HEIGHT)
				return true;
		}
		return false;
	}
}