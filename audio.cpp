#include "audio.hpp"
#include <iostream>

#ifndef USE_FMOD // USE_SDLMIXER

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <climits> // for PATH_MAX
#include <sstream>

static Mix_Music *music= NULL;
static bool music_playing = false;

static void music_finished()
{
    printf("music_finished()\n");
    music_playing = false;
}

void setvolume(float v, bool increase)
{
    printf("setvolume(v=%f,increase=%d)\n", v, (int)increase);
}

void loadtrack(int l)
{
    printf("loadtrack(l=%d)\n", l);
    glClear (GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapBuffers();

    if (music)
    {
        Mix_FreeMusic(music);
        music = NULL;
    }

    std::stringstream path;
    path << "ogg/" << l << ".ogg";

    music = Mix_LoadMUS(path.str().c_str());
    if(music == NULL) 
    {
        printf("Unable to load Ogg file: %s\n", Mix_GetError());
        return;
    }
    if(Mix_PlayMusic(music, 0) == -1) 
    {
        printf("Unable to play Ogg file: %s\n", Mix_GetError());
        return;
    }
}

void playsound(int i)
{
    printf("playsound(i=%d)\n", i);
}

void initsound()
{
    printf("initsound()\n");
    int audio_rate = 22050; // Playback frequency to be used by SDL_mixer
    Uint16 audio_format = AUDIO_S16SYS; // AUDIO_S16SYS will automatically match the user's system's byte order
    int audio_channels = 2; // 2 for stereo sound, and 1 for monaural
    int audio_buffers = 4096; // Size of the memory chunks used for storage and playback of samples

    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
        fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
        exit(-1);
    }

    Mix_HookMusicFinished(music_finished);
    loadtrack(0);
}

void releasesound()
{
    printf("releasesound()\n");
    Mix_HaltMusic();
    if (music)
    {
        Mix_FreeMusic(music);
        music = NULL;
    }
    Mix_CloseAudio();
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
