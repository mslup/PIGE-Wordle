#define NOMINMAX
#include "wordle.h"

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int show_command)
{
	srand(time(NULL));
	wordle app{ hInst };
	


	return app.run(show_command);
}