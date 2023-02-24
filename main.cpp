#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<time.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
#include "main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define BG_WIDTH	5.0
#define	LIFES_AMOUNT 3
#define MAXYVEL 3
#define FPS 80
#define BGVELOCITY 3
#define ARROWSPEED 4
#define ATTACKSPEED 4



void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
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


void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};



void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};

void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void DrawObstacle(double bgmove, SDL_Surface* screen, double x, int y, double l, int k,
	Uint32 outlineColor, Uint32 fillColor, bool* stuck, SDL_Rect unic)
{
	if (x + bgmove + l > 0 && x + bgmove < SCREEN_WIDTH && y < SCREEN_HEIGHT && y + k>0) { // jezeli miesci sie w oknie
		if (x + l + bgmove > SCREEN_WIDTH && x + bgmove < SCREEN_WIDTH) //operacje wykonywane aby nie nadrysowalo figur
		{
			l = SCREEN_WIDTH - (x + bgmove);
		}
		else if (x + bgmove < 0)
		{
			l += (x + bgmove);
			x = -bgmove;
		}
		if (y + k > SCREEN_HEIGHT && y < SCREEN_HEIGHT)
		{
			k = SCREEN_HEIGHT - y;
		}
		SDL_Rect rect1; rect1.x = x + bgmove; rect1.y = y; rect1.w = l; rect1.h = k;
		if (SDL_HasIntersection(&unic, &rect1))
			*stuck = true;
		DrawRectangle(screen, x + bgmove, y, l, k, outlineColor, fillColor);
	}
}
void DrawPlatform(double bgmove, SDL_Surface* screen, double x, int y, double l, int k,
	Uint32 outlineColor, Uint32 fillColor, bool* stuck, bool* blocky, SDL_Rect& unic, int* movey)
{
	if (x + bgmove + l > 0 && x + bgmove < SCREEN_WIDTH && y < SCREEN_HEIGHT && y + k>0) {
		if (y < 0)
		{
			k = k + y;
			y = 0;
		}
		if (x + l + bgmove > SCREEN_WIDTH && x + bgmove < SCREEN_WIDTH)
		{
			l = SCREEN_WIDTH - (x + bgmove);
		}
		else if (x + bgmove < 0)
		{
			l += (x + bgmove);
			x = -bgmove;
		}
		if (y + k > SCREEN_HEIGHT && y < SCREEN_HEIGHT)
		{
			k = SCREEN_HEIGHT - y;
		}
		SDL_Rect rect1; rect1.x = x + bgmove; rect1.y = y; rect1.w = l; rect1.h = k;
		if (SDL_HasIntersection(&unic, &rect1))
		{
			if (unic.y + unic.h > rect1.y && unic.y + unic.h < rect1.y + rect1.h && unic.y + unic.h <= rect1.y + 7) // jednorozec dotyka podloza
			{
				*blocky = true;
				y += unic.y - rect1.y + unic.h;
			}
			else
			{
				*stuck = true;
			}
		}
		DrawRectangle(screen, x + bgmove, y, l, k, outlineColor, fillColor);
	}
}


void reset_all(bool& stuck, int& movey, double& bgmove, int& moveUni, int& lifes, bool& jump, bool& falling, double& velocityY, bool& unpress, int& jumpT, bool change_control, double& move_velocity, int& frame)
{
	stuck = false;
	SDL_Delay(1000);
	movey = 0;
	bgmove = 1;
	moveUni = 0;
	lifes -= 1;
	jump = false;
	falling = false;
	velocityY = 0;
	unpress = true;
	jumpT = 0;
	if (!change_control)
		move_velocity = 0;
	frame = 2;
	if (lifes == 0)
		frame = 3;
}
void keyboard_actions(SDL_Event& event, int& quit, int& movey, double& worldTime, double& bgmove, int& moveUni, int& lifes, bool& unpressD, bool& change_control, bool& falling, double& move_velocity, double& velocityY, double& velocityBG, bool blocky, bool stuck, int& jumpT, bool& attack, bool& unpress, bool& jump, int& attackT)
{
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
			else if (event.key.keysym.sym == SDLK_n) //nowa gra
			{
				movey = 0;
				worldTime = 0;
				bgmove = 1;
				moveUni = 0;
				lifes = LIFES_AMOUNT;
			}
			else if (event.key.keysym.sym == SDLK_d && unpressD == true)
			{
				unpressD = false;
				if (!change_control) {
					change_control = true;
					falling = true;
					move_velocity = BGVELOCITY;
				}
				else {
					falling = false;
					change_control = false;
					velocityY = 0;
					velocityBG = 0;
					move_velocity = 0;
				}


			}
			else
			{
				if (!change_control)
				{
					if (event.key.keysym.sym == SDLK_DOWN && blocky == false && stuck == false) velocityY = -ARROWSPEED;
					if (event.key.keysym.sym == SDLK_UP && stuck == false) velocityY = ARROWSPEED;
					if (event.key.keysym.sym == SDLK_RIGHT) move_velocity = ARROWSPEED;
				}
				else
				{
					if (event.key.keysym.sym == SDLK_z && jumpT != 2 && stuck == false && attack == false && unpress == true)
					{
						jumpT++;
						jump = true;
						velocityY = 6;
						falling = false;
						unpress = false;
					}
					if (event.key.keysym.sym == SDLK_x && attackT == 0)
					{
						velocityBG = ATTACKSPEED;
						velocityY = 0;
						attack = true;
						attackT = 1;
						if (jumpT == 2)
							jumpT = 1;
						falling = false;
						jump = false;

					}
				}
			}
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_d)
				unpressD = true;
			if (!change_control)
			{
				if (event.key.keysym.sym == SDLK_DOWN && blocky == false && stuck == false) velocityY = 0;
				if (event.key.keysym.sym == SDLK_UP && stuck == false) velocityY = 0;
				if (event.key.keysym.sym == SDLK_LEFT) move_velocity = 0;
				if (event.key.keysym.sym == SDLK_RIGHT) move_velocity = 0;
			}
			else
			{
				if (event.key.keysym.sym == SDLK_z)
				{
					if (velocityY > 0)
						velocityY = 0;
					unpress = true;
				}
			}

			break;
		case SDL_QUIT:
			quit = 1;
			break;
		};
	};
}


void unicorn_placement(int& movey, int& moveUni)
{
	if (movey < 0) //ruszanie jednorozcem gdy zbliza sie do dolnej krawedzi
	{
		moveUni += movey;
		movey = 0;
	}
	else if (movey > 0 && moveUni < 0) //ustawianie jednorozca do srodka od dolu
	{
		moveUni += movey;
		movey = 0;
		if (moveUni > 0)
			moveUni = 0;
	}

	if (movey > SCREEN_HEIGHT)	//ruszanie jednorozcem gdy zbliza sie do gornej krawedzi
	{
		moveUni += (movey - SCREEN_HEIGHT);
		movey = SCREEN_HEIGHT;
	}  // ustawianie jednorozca do srodka od gory
	else if (movey < SCREEN_HEIGHT && moveUni>0)
	{
		moveUni += (movey - SCREEN_HEIGHT);
		movey = SCREEN_HEIGHT;
		if (moveUni < 0)
			moveUni = 0;
	}
}

void unicorn_placementV2(int& moveUni, SDL_Surface* unicorn, bool change_control, SDL_Rect& unic, bool& stuck, double& bgmove, double move_velocity, double worldTime, double velocityBG, int& movey, double& velocityY, bool blocky, bool& falling, int& jumpT, bool attack, int& attackT, bool jump)
{
	if (moveUni > SCREEN_HEIGHT / 2 - unicorn->h / 2) //zablokowanie wychodzenia poza gorna krawedz
	{
		moveUni = SCREEN_HEIGHT / 2 - unicorn->h / 2;
	}
	if (moveUni < -SCREEN_HEIGHT / 2 + unicorn->h / 2) //zablokowanie wychodzenia poza dolna krawedz
	{
		if (!change_control)
			moveUni = -SCREEN_HEIGHT / 2 + unicorn->h / 2;
		else
		{
			if (moveUni + unic.h < -SCREEN_HEIGHT / 2 + unicorn->h / 2)
				stuck = true;
		}
	}

	if (!stuck)
	{
		if (change_control)
			bgmove -= (move_velocity + worldTime / 50 + velocityBG); //przyspieszenie jednorozca z czasem
		else
			bgmove -= move_velocity;
		movey += velocityY;
	}

	if (blocky == true && velocityY < 0)
	{
		falling = false;
		jumpT = 0;
		velocityY = 0;
	}
	else if (blocky == true && attack == false)
	{
		attackT = 0;
	}
	if (blocky == false && jump == false && change_control && !attack)
	{
		falling = true;
	}
}

void count_fps_time(int& t2, double& delta, int& t1, double& worldTime, bool& change_frame, double& fpsTimer, double& fps, int& frames)
{
	t2 = SDL_GetTicks();
	delta = (t2 - t1) * 0.001;
	t1 = t2;
	worldTime += delta;
	if (change_frame) {
		change_frame = false;
		worldTime = 0;
	}
	fpsTimer += delta;
	if (fpsTimer > 0.5) {
		fps = frames * 2;
		frames = 0;
		fpsTimer -= 0.5;
	};
}
void change_velocity(bool& jump, double& velocityY, bool& falling, bool blocky, bool& attack, double& velocityBG)
{
	if (jump == true) // spowolnienie skoku
	{
		velocityY -= 0.1;
		if (velocityY <= 0)
		{
			velocityY = 0;
			jump = false;
			falling = true;
		}
	}
	if (falling == true && blocky == false)  //coraz szybsze opadanie
	{
		velocityY -= 0.1;
		if (velocityY < -MAXYVEL)
			velocityY = -MAXYVEL;
	}
	if (attack == true) //spowolnienie zrywu
	{
		velocityBG -= 0.2;
		if (velocityBG <= 0)
		{
			attack = false;
			velocityBG = 0;
			falling = true;
		}
	}
}

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	int t1, t2, quit, frames, rc, movey = 0, moveUni = 0, jumpT = 0, attackT = 0, lifes = LIFES_AMOUNT, frame = 1;
	double delta, worldTime, fpsTimer, fps, delay = 0, move_velocity = 0, bgmove = 0, velocityY = 0, velocityBG = 0;
	SDL_Event event;
	SDL_Surface* screen, * charset;
	SDL_Surface* unicorn;
	SDL_Surface* bg;
	SDL_Surface* life;
	SDL_Surface* menu;
	SDL_Surface* less_life;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	const int SCREEN_FPS = FPS;
	const int SCREEN_TICK = 1000 / SCREEN_FPS;
	bool stuck = false, change_control = false, blocky = false, jump = false, falling = false, attack = false, unpress = true, unpressD = true, change_frame = false;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Robot unicorn attack");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	SDL_ShowCursor(SDL_DISABLE);

	charset = SDL_LoadBMP("./Photos/cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(Photos/cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);

	unicorn = SDL_LoadBMP("./Photos/unicorn.bmp");
	if (unicorn == NULL) {
		printf("SDL_LoadBMP(Photos/unicorn.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_Rect unic;
	unic.h = unicorn->h; unic.w = unicorn->w;
	bg = SDL_LoadBMP("./Photos/bg.bmp");
	if (bg == NULL) {
		printf("SDL_LoadBMP(Photos/unicorn.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_FreeSurface(unicorn);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	life = SDL_LoadBMP("./Photos/life.bmp");
	if (life == NULL) {
		printf("SDL_LoadBMP(Photos/unicorn.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_FreeSurface(unicorn);
		SDL_FreeSurface(bg);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	less_life = SDL_LoadBMP("./Photos/less_life.bmp");
	if (less_life == NULL) {
		printf("SDL_LoadBMP(Photos/unicorn.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_FreeSurface(unicorn);
		SDL_FreeSurface(bg);
		SDL_FreeSurface(life);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	menu = SDL_LoadBMP("./Photos/menu.bmp");
	if (menu == NULL) {
		printf("SDL_LoadBMP(Photos/unicorn.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_FreeSurface(unicorn);
		SDL_FreeSurface(bg);
		SDL_FreeSurface(life);
		SDL_FreeSurface(less_life);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int rozowy = SDL_MapRGB(screen->format, 0xFF, 0xC0, 0xCB);
	int fioletowy = SDL_MapRGB(screen->format, 0xB8, 0x03, 0xFF);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;

	while (!quit) {
		if (frame == 1) {


			SDL_FillRect(screen, NULL, czarny);

			DrawSurface(screen, bg, SCREEN_WIDTH * 2.5 + bgmove, movey);
			DrawSurface(screen, bg, SCREEN_WIDTH * 7.5 + bgmove, movey);

			unic.x = 0;
			unic.y = SCREEN_HEIGHT / 2 - unicorn->h / 2 - moveUni;

			count_fps_time(t2, delta, t1, worldTime, change_frame, fpsTimer, fps, frames);

			DrawObstacle(bgmove, screen, SCREEN_WIDTH + 300.0, SCREEN_HEIGHT - 200 + movey, 20, 200, rozowy, fioletowy, &stuck, unic); // rysowanie przeszkod
			DrawObstacle(bgmove, screen, SCREEN_WIDTH + 1200.0, SCREEN_HEIGHT - 300 + movey, 20, 300, rozowy, fioletowy, &stuck, unic);
			DrawObstacle(bgmove, screen, SCREEN_WIDTH + 1600.0, SCREEN_HEIGHT - 400 + movey, 20, 400, rozowy, fioletowy, &stuck, unic);
			DrawObstacle(bgmove, screen, SCREEN_WIDTH + 2250.0, SCREEN_HEIGHT - 450 + movey, 50, 50, rozowy, fioletowy, &stuck, unic);
			DrawObstacle(bgmove, screen, SCREEN_WIDTH + 700.0, SCREEN_HEIGHT - 630 + movey, 30, 30, rozowy, fioletowy, &stuck, unic);

			DrawPlatform(bgmove, screen, 0, SCREEN_HEIGHT - 200 + movey, SCREEN_WIDTH, 40, fioletowy, rozowy, &stuck, &blocky, unic, &movey); // rysowanie platform
			DrawPlatform(bgmove, screen, SCREEN_WIDTH + 50.0, SCREEN_HEIGHT - 400 + movey, 400, 40, fioletowy, rozowy, &stuck, &blocky, unic, &movey);
			DrawPlatform(bgmove, screen, SCREEN_WIDTH + 250.0, SCREEN_HEIGHT - 440 + movey, 200, 80, fioletowy, rozowy, &stuck, &blocky, unic, &movey);
			DrawPlatform(bgmove, screen, SCREEN_WIDTH + 500.0, SCREEN_HEIGHT - 600 + movey, 400, 40, fioletowy, rozowy, &stuck, &blocky, unic, &movey);
			DrawPlatform(bgmove, screen, SCREEN_WIDTH + 500.0, SCREEN_HEIGHT - 800 + movey, 350, 40, fioletowy, rozowy, &stuck, &blocky, unic, &movey);
			DrawPlatform(bgmove, screen, SCREEN_WIDTH + 1050.0, SCREEN_HEIGHT - 800 + movey, 400, 40, fioletowy, rozowy, &stuck, &blocky, unic, &movey);
			DrawPlatform(bgmove, screen, SCREEN_WIDTH + 1550.0, SCREEN_HEIGHT - 600 + movey, 400, 40, fioletowy, rozowy, &stuck, &blocky, unic, &movey);
			DrawPlatform(bgmove, screen, SCREEN_WIDTH + 1800.0, SCREEN_HEIGHT - 660 + movey, 150, 100, fioletowy, rozowy, &stuck, &blocky, unic, &movey);
			DrawPlatform(bgmove, screen, SCREEN_WIDTH + 1950.0, SCREEN_HEIGHT - 400 + movey, 400, 40, fioletowy, rozowy, &stuck, &blocky, unic, &movey);
			DrawPlatform(bgmove, screen, SCREEN_WIDTH + 2560.0, SCREEN_HEIGHT - 200 + movey, SCREEN_WIDTH, 40, fioletowy, rozowy, &stuck, &blocky, unic, &movey);
			DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, fioletowy);

			sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s", worldTime, fps);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
			sprintf(text, "Esc - wyjscie, N - nowa gra Z - skok X - atak D - zmiana trybu");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
			for (int i = 1; i <= lifes; i++) // rysowanie zyc
				DrawSurface(screen, life, -20 + i * 40, 60);

			DrawSurface(screen, unicorn, unicorn->w / 2, SCREEN_HEIGHT / 2 - moveUni); //rysowanie unicorna

			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);   //update ekranu
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);

			change_velocity(jump, velocityY, falling, blocky, attack, velocityBG);

			keyboard_actions(event, quit, movey, worldTime, bgmove, moveUni, lifes, unpressD, change_control, falling, move_velocity, velocityY, velocityBG, blocky, stuck, jumpT, attack, unpress, jump, attackT);
			unicorn_placementV2(moveUni, unicorn, change_control, unic, stuck, bgmove, move_velocity, worldTime, velocityBG, movey, velocityY, blocky, falling, jumpT, attack, attackT, jump);
			if (bgmove < -BG_WIDTH * SCREEN_WIDTH)
				bgmove = 1;
			delay = (SCREEN_TICK - (t2 - t1));
			if (delay > 0)  //ustawienie fpsow
			{
				SDL_Delay(delay);
			}

			unicorn_placement(movey, moveUni);

			if (stuck == true) //gdy jednorozec zderza sie z przeszkoda
				reset_all(stuck, movey, bgmove, moveUni, lifes, jump, falling, velocityY, unpress, jumpT, change_control, move_velocity, frame);
			blocky = false;
			frames++;
		}
		if (frame == 2)

		{
			change_frame = true;
			SDL_FillRect(screen, NULL, czarny);
			DrawSurface(screen, less_life, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_q) //gra dalej
					{
						frame = 1;
					}
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			}
		}
		if (frame == 3)

		{
			change_frame = true;
			lifes = LIFES_AMOUNT;
			SDL_FillRect(screen, NULL, czarny);
			DrawSurface(screen, menu, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
					else if (event.key.keysym.sym == SDLK_RETURN) //nowa gra
					{
						frame = 1;
					}
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			}
		}
	};

	// zwolnienie powierzchni
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(unicorn);
	SDL_FreeSurface(bg);
	SDL_FreeSurface(life);
	SDL_FreeSurface(less_life);
	SDL_FreeSurface(menu);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
};
