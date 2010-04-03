#ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include <stdio.h>

const int MUSICCOUNT=38;

void initsound();
void releasesound();
void loadtrack(int l);
void playsound(int i);
void setvolume(float v, bool increase=false);
