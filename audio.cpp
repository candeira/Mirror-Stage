#include "audio.hpp"
#include <iostream>

#ifndef USE_FMOD // USE_SDLMIXER

#include <SDL/SDL_mixer.h>
#include <climits> // for PATH_MAX

void setvolume(float v, bool increase)
{
	printf("setvolume(v=%f,increase=%d)\n", v, (int)increase);
}

void loadtrack(int l)
{
    printf("loadtrack(l=%d)\n", l);
}

void playsound(int i)
{
    printf("playsound(i=%d)\n", i);
}

void initsound()
{
    printf("initsound()\n");
}

void releasesound()
{
    printf("initsound()\n");
}

#else // USE_FMOD

/*
 FMOD STUFF
 */

#include "fmod/fmod.h"
#include "fmod/fmod_errors.h"

#struct fmod_dat {
	FMOD_SYSTEM* system;
	FMOD_SOUND* sound;
};

fmod_dat fmod;


void ERRCHECK(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
        exit(-1);
    }
}
FMOD_CHANNEL     *channel = 0;
//FMOD_CHANNEL	*channelalt = 0;

void setvolume(float v, bool increase)
{
    float u;
    FMOD_Channel_GetVolume(channel,&u);

    if (increase || v<u)
        FMOD_Channel_SetVolume(channel,v);
}

void loadtrack(int l)
{
	glClear (GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapBuffers();
	FMOD_RESULT result;
//
	if (fmod.sound!=NULL)
		FMOD_Sound_Release(fmod.sound);

	FMOD_Channel_Stop(channel);

	char* prefix =  "ogg/%d.ogg";
	char path[100];
	sprintf(path, prefix, l);

	result = FMOD_System_CreateSound(fmod.system,path, FMOD_SOFTWARE | FMOD_2D | FMOD_LOOP_NORMAL, 0, &(fmod.sound));
	//    result = FMOD_System_CreateSound(system, "xm/bassline.xm", FMOD_SOFTWARE | FMOD_2D | FMOD_LOOP_NORMAL | FMOD_CREATESTREAM, 0, &sound);
	ERRCHECK(result);

    result = FMOD_System_PlaySound(fmod.system, FMOD_CHANNEL_REUSE, fmod.sound, 0, &channel);
	setvolume(1,true);
    ERRCHECK(result);

}

FMOD_SOUND* blip1;
FMOD_CHANNEL* c[4];
int channelindex=0;

void playsound(int i)
{
	FMOD_RESULT result;

	switch (i)
	{
		case 0:
			result = FMOD_System_PlaySound(fmod.system, FMOD_CHANNEL_REUSE, blip1, 0, &(c[channelindex]));
			channelindex=(channelindex+1) % 4;
			break;
	}
}

void initsound()
{

	FMOD_RESULT       result;
	unsigned int      version;

    /*
	 Global Settings
	 */
    result = FMOD_System_Create(&(fmod.system));
    ERRCHECK(result);

    result = FMOD_System_GetVersion(fmod.system, &version);
    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
		//error!
    }

	result = FMOD_System_Init(fmod.system,25, FMOD_INIT_NORMAL, NULL);
    ERRCHECK(result);

	result=FMOD_System_CreateSound(fmod.system,"wav/1.wav",FMOD_HARDWARE|FMOD_2D|FMOD_LOOP_OFF,0, &blip1);
	ERRCHECK(result);

//	result = FMOD_System_PlaySound(fmod.system, FMOD_CHANNEL_FREE, oggsound, 0, &channelalt);
//  ERRCHECK(result);


	loadtrack(0);

    }

void releasesound()
{
	FMOD_RESULT       result;
	 result = FMOD_Sound_Release(fmod.sound);
	 ERRCHECK(result);
//	result = FMOD_Sound_Release(oggsound);
//	ERRCHECK(result);
	result = FMOD_System_Close(fmod.system);
	 ERRCHECK(result);
	 result = FMOD_System_Release(fmod.system);
	 ERRCHECK(result);
}

#endif // USE_FMOD
