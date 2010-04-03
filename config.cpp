/*
 * test.c
 * Example of a C program that interfaces with Lua.
 * Based on Lua 5.0 code by Pedro Martelletto in November, 2003.
 * Updated to Lua 5.1. David Manura, January 2007.
 */

#include <iostream>
#include <math.h>

#include "config.hpp"


float C_MAXDEPTH=500;
float C_MAXREALDEPTH=50;

int C_SCREEN_WIDTH=640;
int C_SCREEN_HEIGHT=480;
bool C_FULLSCREEN=1;
bool C_ASPECTFLAG=0;

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <stdlib.h>
#include <stdio.h>
}


float getmaxdepth(){return C_MAXDEPTH;}
float getmaxrealdepth(){return C_MAXREALDEPTH;}
int getscreenwidth(){return C_SCREEN_WIDTH;}
int getscreenheight(){return C_SCREEN_HEIGHT;}
bool getfullscreen(){return C_FULLSCREEN;}
bool getaspectflag(){return C_ASPECTFLAG;}

void load_config_vars()
{
    int status, result;
    lua_State *L;

    /*
     * All Lua contexts are held in this structure. We work with it almost
     * all the time.
     */
    L = luaL_newstate();

    luaL_openlibs(L);

    /* Load the file containing the script we are going to run */
    status = luaL_loadfile(L, "settings/config.txt");
    if (status) {
        /* If something went wrong, error message is at the top of */
        /* the stack */
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(L, -1));
        exit(1);
    }

	/* Ask Lua to run our little script */
    result = lua_pcall(L, 0, LUA_MULTRET, 0);

    lua_getglobal(L,"screenwidth");
    C_SCREEN_WIDTH = (int)(lua_tonumber(L, -1));

    lua_getglobal(L,"screenheight");
    C_SCREEN_HEIGHT = (int)(lua_tonumber(L, -1));

    lua_getglobal(L,"fullscreen");
    C_FULLSCREEN = (int)(lua_toboolean(L, -1));

    lua_getglobal(L,"maxdepth");
    C_MAXDEPTH = (int)(lua_tonumber(L, -1));

    lua_getglobal(L,"maxrealdepth");
    C_MAXREALDEPTH = (int)(lua_tonumber(L, -1));

    lua_close(L);   /* Cya, Lua */

}



