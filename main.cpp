#include"configuration.h"

#ifdef __cplusplus
extern "C"
#endif

SDL sdl = {};
Game game = {};
Colors color = {};

int main(int argc, char **argv) {

	// Initialize SDL and create the window, renderer, surface and texture
	if (initialConfiguration(&sdl, &color)) return 1;

	// start new game
	newGame(&sdl, &game);

	// Main loop
	while (!game.status.quit) {
		// Process events 
		processEvents(&sdl, &game, color);

		// Pausing game
		if (game.status.pause) {
			game.time.tick1 = SDL_GetTicks();
			sprintf(color.text, "PAUZA");
			drawString(sdl.screen, 10, SCREEN_HEIGHT - 10, color.text, sdl.charset);
			if (game.status.error) {
				sprintf(color.text, "BRAK PLIKOW DO WCZYTANIA");
				drawString(sdl.screen, 10, SCREEN_HEIGHT - 20, color.text, sdl.charset);
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

		// Setting current position of the car and life
		setCarInfo(&game);

		// Calculate elapsed time, distance travelled, frame rate and score
		calculateData(&game);

		// Clear the screen
		SDL_FillRect(sdl.screen, NULL, color.zielony);

		// Draw the road
		drawRoad(&sdl, &game, &color);

		// Draw the bitmap images and text to the surface
		drawInterface(sdl, game, color);

		// Display the surface on the window
		displaySurface(&sdl);
	}

	// Clean up resources and shut down SDL
	cleanupAndQuit(&sdl);
	return 0;
};