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

		checkGameStatus(&sdl, &game, color);

		setCarInfo(&game);

		calculateData(&game);

		SDL_FillRect(sdl.screen, NULL, color.zielony);

		drawRoadAndCar(&sdl, &game, &color);

		drawInterface(sdl, game, color);

		displaySurface(&sdl);
	}

	cleanupAndQuit(&sdl);
	return 0;
}