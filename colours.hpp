#ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

typedef GLfloat Colour[4];

struct ColourScheme
{
	Colour outline;
  Colour oddroomunvisited;//used for uncompleted level
  Colour evenroomunvisited;//used for uncompleted but available level
  Colour oddroomvisited;//used for completed level
  Colour evenroomvisited;
	Colour background;
	bool edgetransparency;
	bool roomtransparency;
	bool entitytransparency;
	bool noplayerdup;
};

//these aren't currently changing from level to level

Colour c_mover = {1.0f,0.0f,0.0f};
Colour c_doctor = {0.3f,0.3f,1.0f};
Colour c_sentry = {0.0f,1.0f,1.0f};
Colour c_paritymoversafe = {0.0f,1.0f,1.0f};
Colour c_paritymoverharm = {1.0f,0.0f,0.0f};
Colour c_wall ={0.0f,0.0f,0.0f};
Colour c_table = {1.0f,0.0f,0.0f};
Colour c_flower = {1.0f,0.2f,0.2f};
Colour c_text = {1.0f,1.0f,1.0f};
Colour c_bg={0.0f,0.0f,0.0f};
Colour c_player={0.0f,1.0f,0.0f};


/*

notes about alternative rendering options:

 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

OR how about

 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 glBlendEquation(
 GL_FUNC_REVERSE_SUBTRACT
 );
 is really nice with the background white on the fractal level
 (needs GL_ARB_imaging to be around though)

 glBlendEquation(GL_FUNC_SUBTRACT);
 looks really nice on the pentagon level

 glBlendFunc(GL_SRC_ALPHA, GL_ONE);
 looks really nice on the irregular pent level :)

 also looks nice on the urban level, but would have to draw player in a different mode for it to play
 */
/*should I include a 'draw portal' option and portal colour
 option as well? (might be good for comic-style mandala levels?)
 */
