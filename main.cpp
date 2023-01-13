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

		updateCarInfo(&game);

		initBotsCars(&game);

		updateBotsCarsInfo(&game);

		calculateData(&game);

		SDL_FillRect(sdl.screen, NULL, color.zielony);

		drawRoadAndCars(&sdl, &game, &color);

		checkCarCollisionStatus(&game);

		drawInterface(sdl, game, color);

		displaySurface(&sdl);
	}

	cleanupAndQuit(&sdl);
	return 0;
}