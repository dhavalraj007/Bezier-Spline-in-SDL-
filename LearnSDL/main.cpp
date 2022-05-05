//Using SDL, SDL_image, standard IO, and strings
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Starts up SDL and creates window
void init();

//Loads media
void loadMedia();

//Frees media and shuts down SDL
void close();

//Loads individual image as texture
SDL_Texture* loadTexture(std::string path);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Current displayed texture
SDL_Texture* gTexture = NULL;

SDL_Color colors[] = { {0X08,0Xbd,0Xaf,0XFF},{0x46,0x7e,0xf4}, {0x9f,0xf0,0x7f} };

void init()
{

	
	SDL_Init(SDL_INIT_VIDEO);	//Initialize SDL
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");	//Set texture filtering to linear
	gWindow = SDL_CreateWindow("SDL app", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
	

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	}
}

void loadMedia()
{
	gTexture = loadTexture("../res/texture.png");
	if (gTexture == NULL)
	{
		printf("Failed to load texture image!\n");
	}
}

void close()
{
	//Free loaded image
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

SDL_Texture* loadTexture(std::string path)
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	
	newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);

	//Get rid of old loaded surface
	SDL_FreeSurface(loadedSurface);

	return newTexture;
}

SDL_FPoint toScreenCoord(SDL_FPoint pt)
{
	return { pt.x,-pt.y + SCREEN_HEIGHT };
}
SDL_FPoint getPointOnCurve(std::vector<SDL_FPoint> controlPoints, double t)
{
	int degree = controlPoints.size() - 1;

	for (int j = 1; j <= degree; j++)
	{
		for (int i = 0; i <= degree - j; i++)
		{
			
			controlPoints[i].x = (1 - t) * controlPoints[i].x + t * controlPoints[i + 1].x;
			controlPoints[i].y = (1 - t) * controlPoints[i].y + t * controlPoints[i + 1].y;

			if (i > 0)
			{
				SDL_FPoint p1, p2;
				p1 = toScreenCoord(controlPoints[i-1]);
				p2 = toScreenCoord(controlPoints[i]);
				SDL_SetRenderDrawColor(gRenderer, colors[j % 3].r, colors[j % 3].g, colors[j % 3].b, colors[j % 3].a);
				SDL_RenderDrawLine(gRenderer,p1.x,p1.y,p2.x,p2.y);
			}
			SDL_SetRenderDrawColor(gRenderer, 0XFF, 0XFF, 0XFF, 0XFF);
			SDL_FPoint pt = toScreenCoord(controlPoints[i]);
			SDL_FRect r{ pt.x-5.f, pt.y-5.f,10.f,10.f};
			SDL_RenderDrawRectF(gRenderer, &r);
			if (j == degree)
			{
				SDL_SetRenderDrawColor(gRenderer, 0XFF, 0XFF, 0XFF, 122);
				SDL_RenderFillRectF(gRenderer, &r);
			}
		}
	}
	return controlPoints[0];
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	init();
//	loadMedia();
	
	bool quit = false;	//Main loop flag

	//Event handler
	SDL_Event e;


	std::vector<SDL_FPoint> controlPoints{ 4,{0.f,0.f} };
	std::vector<SDL_FPoint> defPoints{ 4,{0.f,0.f} };
	//SDL_FPoint origin{ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	defPoints[0]={ 10,10 };
	defPoints[1]={ 10,SCREEN_HEIGHT/2+10 };
	defPoints[2]={ SCREEN_WIDTH-10,SCREEN_HEIGHT/2+10 };
	defPoints[3]={ SCREEN_WIDTH-10,10 };
	controlPoints = defPoints;
	

	int numOfControlPoints = controlPoints.size();
	double step = 0.0003;
	std::vector<SDL_FPoint> currentPoints = { {0.f,0.f} };
	std::vector<SDL_FPoint> otherPoints;
	double t = 0;

	//While application is running
	while (!quit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_a)
				{
					int imx, imy;
					SDL_GetMouseState(&imx, &imy);
					float mx = imx;
					float my = imy;
					controlPoints.push_back(toScreenCoord({ mx,my }));
					t = 0;
					currentPoints.clear();
				}
				if (e.key.keysym.sym == SDLK_d)
				{
					controlPoints.resize(controlPoints.size()-1);
					t = 0;
					currentPoints.clear();
				}
				if (e.key.keysym.sym >= SDLK_0 and e.key.keysym.sym <= SDLK_9 and e.key.keysym.sym - SDLK_0 <= controlPoints.size() - 1)
				{
					int imx, imy;
					SDL_GetMouseState(&imx, &imy);
					float mx = imx;
					float my = imy;
					controlPoints[e.key.keysym.sym - SDLK_0] = toScreenCoord({ mx,my });
					currentPoints.clear();
					t = 0;
				}
				if (e.key.keysym.sym == SDLK_s)
				{
					currentPoints.clear();
					t = 0;
				}
				if (e.key.keysym.sym == SDLK_r)
				{
					controlPoints = defPoints;
					currentPoints.clear();
					t = 0;
				}
			}
		}

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0,0,0, 0XFF);
		SDL_RenderClear(gRenderer);

		//Render texture to screen
		//SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
		
		//draw lines
		for (int i = 0; i < controlPoints.size() - 1; i++)
		{
			SDL_FPoint tempi = toScreenCoord(controlPoints[i]);
			SDL_FPoint tempi1 = toScreenCoord(controlPoints[i+1]);
			SDL_SetRenderDrawColor(gRenderer, 0XaF, 0Xca, 0, 0XFF);
			SDL_RenderDrawLine(gRenderer, tempi.x, tempi.y, tempi1.x, tempi1.y);
		
			SDL_SetRenderDrawColor(gRenderer, 0XFF, 0, 0, 0XFF);
			SDL_FRect r{ tempi.x - 5.f, tempi.y - 5.f,10.f,10.f };
			SDL_RenderFillRectF(gRenderer, &r);
		}
		SDL_FPoint tempi = toScreenCoord(controlPoints.back());
		SDL_FRect r{ tempi.x - 5.f, tempi.y - 5.f,10.f,10.f };
		SDL_RenderFillRectF(gRenderer, &r);

		SDL_FPoint Pt;
		if (t <= 1)
		{
			t += step;
			Pt = getPointOnCurve(controlPoints, t);
			Pt = toScreenCoord(Pt);
			currentPoints.push_back(Pt);
		}

		SDL_SetRenderDrawColor(gRenderer, 0,0,0XFF, 0XFF);
		for (SDL_FPoint pt : currentPoints)
		{
			SDL_RenderDrawPoint(gRenderer, pt.x, pt.y);
		}
		//Update screen
		SDL_RenderPresent(gRenderer);
	}

	//Free resources and close SDL
	close();

	return 0;
}