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
	SDL_FreeSurface(sdl->tree);
	SDL_FreeSurface(sdl->line);
	SDL_Quit();
}

void drawRoadAndCars(SDL* sdl, Game* game, Colors* color) {
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

	// Draw enemy cars
	for (int i = 0; i < 5; ++i) {
		drawSurface(sdl->screen, sdl->enemyCarSurface, game->enemyCar[i].posX, game->enemyCar[i].posY);
	}
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
	game->enemyCar[0].posY = 700;
	game->enemyCar[1].posY = 1000;
	game->enemyCar[2].posY = 1500;
	game->enemyCar[3].posY = -1000;
	game->enemyCar[4].posY = -1000;
}

void updateCarInfo(Game* game) {
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


	// Blocking off-road driving
	if ((game->car.posX < (ROAD_WIDTH - ROADSIDE)) || (game->car.posX > (2 * ROAD_WIDTH) + ROADSIDE)) {
		game->car.life--;
		game->car.posX = DEFAULT_POS_CAR_X;
		game->car.posY = DEFAULT_POS_CAR_Y;
	}

	
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

void initEnemies(Game* game) {
	// Cars from front
	int posFrontCarDissapear = rand() % 221 + 480;
	for (int i = 0; i < MAX_ENEMIES; ++i) {
		if (game->enemyCar[i].posY > posFrontCarDissapear) {
			game->enemyCar[i].posX = rand() % 213 + 214;
			game->enemyCar[i].posY = rand() % 1000 - 1500;
			srand(rand());
		}
	}

	// Cars from back
	int posBackCarDissapear = rand() % 500 - 1000;
	for (int i = 3; i < 5; ++i) {
		if (game->enemyCar[i].posY < posBackCarDissapear) {
			game->enemyCar[i].posX = rand() % 213 + 214;
			game->enemyCar[i].posY = rand() % 1000 + 500;
			srand(rand());
		}
	}
}

void updateEnemyCarsInfo(Game* game) {
	for (int i = 0; i < MAX_ENEMIES; ++i) {
		game->enemyCar[i].posY += game->car.speed * game->time.deltaTime * 200;
	}
	for (int i = 3; i < 5; ++i) {
		int tempSpeed=1;
		if (game->car.speed > 1) tempSpeed = -1.5;
		else if (game->car.speed < 1) tempSpeed = 2.0;
		else tempSpeed = 1;

		game->enemyCar[i].posY -= tempSpeed * game->time.deltaTime * 100;
	}
}

void checkPlayerCarCollision(Game* game) {
	for (int i = 0; i < 5; ++i) {
		double playerPosX = game->car.posX;
		double playerPosY = game->car.posY;
		double enemyPosX = game->enemyCar[i].posX;
		double enemyPosY = game->enemyCar[i].posY;
		game->info.t2 = SDL_GetTicks();
		static int t1;
		int t2 = game->time.worldTime;
		if (t2 - t1 < 3) printf("e\n");

		if (abs(playerPosX - enemyPosX) < CAR_WIDTH
			&& abs(playerPosY - enemyPosY) < CAR_HEIGHT) {
			if (playerPosX > enemyPosX || playerPosX < enemyPosX) {
				game->car.posX = DEFAULT_POS_CAR_X;
				game->car.life--;
				game->enemyCar[0].posY = 700;
				game->enemyCar[1].posY = 1000;
				game->enemyCar[2].posY = 1500;
				game->enemyCar[3].posY = 2000;
				game->enemyCar[4].posY = 4000;
				t1 = game->time.worldTime;
				printf("%i", t1);
			}
		}
	}
}