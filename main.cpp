#include"functions.h"

#ifdef __cplusplus
extern "C"
#endif

SDL sdl = {};
Game game = {};
Colors color = {};

int main(int argc, char **argv) {
	if (initialConfiguration(&sdl, &color)) return 0;

	newGame(&sdl, &game);

	while (!game.status.quit) {
		processEvents(&sdl, &game, color);

		if (game.status.pause) {
			game.time.tick1 = SDL_GetTicks();
			sprintf(color.text, "PAUZA");
			drawString(sdl.screen, PAUSE_POS_X, PAUSE_POS_Y, color.text, sdl.charset);
			if (game.status.loadError) {
				sprintf(color.text, "BRAK PLIKOW DO WCZYTANIA");
				drawString(sdl.screen, PAUSE_POS_X, PAUSE_POS_Y-10, color.text, sdl.charset);
			}
			displaySurface(&sdl);
			continue;
		}

		if (game.status.finish) {
			finishGame(sdl, game, color);
			continue;
		}

		if (game.status.load) {
			showSaves(sdl, &game, color);
			continue;
		}
		else game.status.load = false;

		setCarInfo(&game);

		calculateData(&game);

		SDL_FillRect(sdl.screen, NULL, color.zielony);

		drawRoadAndCar(&sdl, &game, &color);

		drawInterface(sdl, game, color);

		displaySurface(&sdl);
	}

	cleanupAndQuit(&sdl);
	return 0;
};