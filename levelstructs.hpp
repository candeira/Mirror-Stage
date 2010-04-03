#ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif


#include <string>
#include <vector>
#include "boost/multi_array.hpp"

using namespace std;

// Start game-state definition


#include "mathstuff.hpp"
#include "entities.hpp"
#include "colours.hpp"
#include "roomshapes.hpp"

int ROOMCOUNT=80;
const int DEFAULTROOMCOUNT=80;


struct Room//eventually should have sprites associated with rooms
{
	int shape;
	bool portal[SHAPESIZE];
	bool portalparity[SHAPESIZE];
	//specifies the direction of the edge relative to the natural orientation
	//1 is natural matching, 0 will reverse parity
	int portaltoroom[SHAPESIZE];
	int portaltoedge[SHAPESIZE];
	bool localportal[SHAPESIZE];
	//portalcount here as well!
};

struct PlayerPos {
	GLfloat x;
	GLfloat y;
	GLfloat dir;
	int room;
	bool par;
	GLfloat scale;
};

struct PlayerState {
	GLfloat oldx;
	GLfloat oldy;
	bool movingforward;
	bool movingback;
	bool rotatingl;
	bool rotatingr;
};

struct LevelParams {
	bool exploreon;
	int roomstoexplore;
	bool paritymatters;
	bool scalematters;
	bool scaleandparitymatter;
	bool enterroom;
	int goalroom;	//if goalroom is -1, then area has entity-based objective
	int goalroomparity;
	int goalroomscale;
	bool mirrorexit;
	bool timeron;
	int timer;
	int music;
};

struct EntityData {
	vector <Entity> curEntityPositions;
};

struct DerivedData {
	vector <GLfloat> portalcount;
    boost::multi_array<GLfloat, 2> roomratios;
	boost::multi_array<bool,3> visited;
};


struct DisplayLists {
	GLuint shapelist;
	GLuint roomoutlines;
	GLuint e_mlist, e_plist, e_tlist,e_flist,e_bedroomlist,e_bedroomlist2, e_bathroomlist;
};

struct LevelState {
	string introtext;
	PlayerPos playerpos;
	PlayerState playerstate;
	LevelParams levelparams;
	ColourScheme colourscheme;
	vector<Room> rooms;
	EntityData entitydata;
	DerivedData deriveddata;
	DisplayLists displaylists;
};

// end level state defintion
