//Using SDL, SDL_image, standard IO, and strings
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

SDL_FPoint getPointOnCurve(std::vector<SDL_FPoint> controlPoints, double t)
{
	int degree = controlPoints.size()-1;
	
	for (int j = 1; j <= degree; j++)
	{
		for (int i = 0; i <= degree - j; i++)
		{
			controlPoints[i].x = (1 - t) * controlPoints[i].x + t * controlPoints[i + 1].x;
			controlPoints[i].y = (1 - t) * controlPoints[i].y + t * controlPoints[i + 1].y;
		}
	}
	return controlPoints[0];
}

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


int main(int argc, char* args[])
{
	//Start up SDL and create window
	init();
	loadMedia();
	
	bool quit = false;	//Main loop flag

	//Event handler
	SDL_Event e;



	std::vector<SDL_FPoint> controlPoints{ 4,{0.f,0.f} };
	//SDL_FPoint origin{ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	controlPoints[0]={ 0,0 };
	controlPoints[1]={ 0,SCREEN_HEIGHT/2 };
	controlPoints[2]={ SCREEN_WIDTH,SCREEN_HEIGHT/2 };
	controlPoints[3]={ SCREEN_WIDTH,0 };
	

	int numOfControlPoints = controlPoints.size();

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
					numOfControlPoints++;
				}
				if (e.key.keysym.sym >= SDLK_0 and e.key.keysym.sym <= SDLK_9 and e.key.keysym.sym - SDLK_0 <= numOfControlPoints - 1)
				{
					int imx, imy;
					SDL_GetMouseState(&imx, &imy);
					float mx = imx;
					float my = imy;
					controlPoints[e.key.keysym.sym - SDLK_0] = toScreenCoord({ mx,my });
				}
			}
		}

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0XFF, 0XFF, 0XFF, 0XFF);
		SDL_RenderClear(gRenderer);

		//Render texture to screen
		//SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
		SDL_FPoint Pt;

		SDL_SetRenderDrawColor(gRenderer, 0XFF, 0, 0, 0XFF);
		for (int i = 0; i < controlPoints.size() - 1; i++)
		{
			SDL_FPoint tempi = toScreenCoord(controlPoints[i]);
			SDL_FPoint tempi1 = toScreenCoord(controlPoints[i+1]);
			SDL_RenderDrawLine(gRenderer, tempi.x, tempi.y, tempi1.x, tempi1.y);
		}
		SDL_SetRenderDrawColor(gRenderer, 0,0,0XFF, 0XFF);
		for (double t = 0; t < 1; t += 0.00001)
		{
			Pt = getPointOnCurve(controlPoints,t);
			Pt = toScreenCoord(Pt);
			SDL_RenderDrawPoint(gRenderer,Pt.x,Pt.y);
		}
		//Update screen
		SDL_RenderPresent(gRenderer);
	}

	//Free resources and close SDL
	close();

	return 0;
}