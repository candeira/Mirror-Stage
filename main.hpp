//The headers

#ifdef __APPLE__
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

//pure function optimisation
#ifdef __GNUC__
#define pure_function __attribute__((pure))
#define const_function __attribute__((const))
#else
#define pure_function
#define const_function
#endif


#include "config.hpp"
#include <vector>

#include "timer.hpp"

#include "levelstructs.hpp"

#include "shapes.hpp"
#include "math.h"
#include "audio.hpp"


enum GameState {gs_intro,gs_title,gs_interlude,gs_level,gs_quit,gs_finishchapter};
enum TitleSubOption {ts_main,ts_newgameselect,ts_newgamechapterselect,ts_newcustomgameselect,ts_newcustomgamechapterselect,
ts_editgameselect, ts_editentername, ts_editchapterselect,ts_nofiles,ts_editnofiles};

class Game
{
public:
void handle_input();
void move();
void show();
Game();
};

//the following are loaded from config files

float MAXDEPTH=500;
float MAXREALDEPTH=30;

const int DEFAULT_SCREEN_WIDTH=640;
const int DEFAULT_SCREEN_HEIGHT=480;
const bool DEFAULT_FULLSCREEN=1;


void deallocOutlineLists();
void allocOutlineLists(int NUM);

 inline GLfloat min (GLfloat a, GLfloat b) const_function;
int getscalevisit(GLfloat s) const_function;
int getscaleexplore(GLfloat s) const_function;
 void moveEntity(Entity& E, int room);
 void drawEntity(Entity E,GLfloat a, bool parity);
 bool CanMove(int room,GLfloat x,GLfloat y);
 void RenderRoom(int room_t, int room_f, int edge_t, int edge_f, bool parity, bool globparity, GLfloat depth, GLfloat realdepth, GLfloat outers);
 void buildOutlines();
 void buildLists();
 bool init_GL();
 bool init();
 void initlevel();
 void restartlevel();
 void finishlevel();
 void clean_up();
 void levelinput();
 void interludeinput();
 void levelmove();
 void interludemove();
 void levelshow();
 void printletter(int i);
 int chartonum(char c) const_function;
 void print_straight_text(const string& s);
 void print_straight_letter(int i);
 void print_text(const char *s);
 void print_text2(const char *s);
 void interludeshow();
 void print_text(const char *s);
 void finishlevel();
 void restartlevel();
 void saveleveldat(const char *s);
 void loadleveldat(const char *s);
 void generate_extra_init_stuff();
 void writelevel(const char* path,const LevelState& level);
 void readlevel(const char* path,LevelState& level);
 void ls_initlevel(int l, const LevelState& level, int fps, int oldlevel);
 void blankLevel(LevelState& level);
 void generateShapeScales();
 void buildRoomRatios();
 void countPortals();
void deleteDir(string s);
void renameFile(string s,string t);
void getEpisodeData();
void resizethings(int newroomcount, bool forceresize=false);
void clearsizes();
void fadetoblack(bool fade=1);

struct ConnectionInfo {
	bool connected;
	int indices; //indices of empty slots
};

struct Portal {
	int room;
	int side;
 };

enum PageView {E_FileStuff, E_ColourScheme,E_LevelGoals,E_EntityChooser,E_TextChooser, E_RoomAdder,E_PortalMode,E_Other};

class EditorState {
public:
	bool allowmove;
	bool prompttodelete;
	Portal portalselected;
	Portal portaldropped;
	void overlay();
	int itemcount;
	void input (char c);
	PageView page;
	void setPage(PageView newpage);
	int itemselected;
	int shapeselected;
	int wallselected;
	int realwallselected;
	int roomcount;//calculate by loking atislands I guess?
	bool newportalparity;
	bool newportalloc;
	vector <ConnectionInfo> component;
private:
	void drawColourChart();
	void drawColourBox();
	void drawLevelObjectives();
	void drawFileMenu();
	void drawEntityChooser();
	void drawTextChooser();
	void drawRoomAdder();
	void drawPortalMode();
	void drawOtherMode();

	GLfloat rotamount;


	void traceRooms();
	void traceRoom(int r);
	void indexFreeRooms();

	void addPortal(Portal pFrom, Portal pTo, bool par, bool loc);
	void removePortal(Portal p);
	void addRoom(Portal pFrom, int shape, int shapeside,bool par, bool loc);


	Portal portalTo;
};

GLfloat roomNormalizations[SHAPECOUNT];


bool firstframe;
