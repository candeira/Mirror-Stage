/*
 *  config.cpp
 *  Mirror Stage
 *
 *  Created by increpare on 19/12/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

//make these glfloats at some point...

#ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

void load_config_vars();
float getmaxdepth();
float getmaxrealdepth();
int getscreenwidth();
int getscreenheight();
bool getfullscreen();
bool getaspectflag();
