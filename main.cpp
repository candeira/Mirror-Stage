/*
    Mirror Stage Source (public beta 0.9)
    copyright Stephen Lavelle 2008-2009
    linux port copyright Miriam Ruiz y Javier Candeira 2010
*/

#ifdef USE_GETTEXT
#include "libintl.h"
#include "locale.h"
#define _(String) gettext (String)
#else
#define _(String) (String)
#endif

#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
using namespace std;


#include "main.hpp"
#include "readwrite.hpp"
#include "savegame.hpp"
#include "filesys.hpp"

/******
*
* Global Variables
*
******/

int SCREEN_WIDTH = DEFAULT_SCREEN_WIDTH;
int SCREEN_HEIGHT = DEFAULT_SCREEN_HEIGHT;
bool FULLSCREEN = DEFAULT_FULLSCREEN;

//keyboard state
SDL_Event event;

LevelState level;
GameState gamestate = gs_intro;
bool wantquit = false; //always skips to main menu
int curlevel = 0;
string curchapter;
string enteringname;
int episodecount = 0;
bool customchapter = false;
bool editorenabled = false;
bool resetlistbounds;
TitleSubOption tso = ts_main;
bool saveasmode = false;

int curtitleselection1 = 0;
int curtitleselection2 = 0;
int curtitleselection3 = 0;
bool afterselection3 = false;
bool beforeselection3 = false; //used only on first item
bool ignorenextspaceup = false;

#include "editor.hpp"

/*********
* Global Constants
*
**********/

const int SCREEN_BPP = 32;
const int FRAMES_PER_SECOND = 30;
const GLfloat pl_s = 0.1f;        //player line size
const GLfloat pspeed = 0.015f;    //player speed

/*********
* Memory Management Code; used in editor
*
**********/

int lastroomcountallocated = DEFAULTROOMCOUNT;
bool allocated = false;

void allocOutlineLists(int NUM)
{
    deallocOutlineLists();
    level.displaylists.roomoutlines = glGenLists(NUM);

    lastroomcountallocated = NUM;
    allocated = false;
}

void deallocOutlineLists()
{
    if (allocated)
    {
        glDeleteLists(level.displaylists.roomoutlines, lastroomcountallocated);
        allocated = false;
    }
}

/*********
* Font drawing routines hidden away here
*
**********/

#include "font.hpp"


/*********
* Level manipulation functions
*
**********/

void clearsizes()
{
    level.entitydata.curEntityPositions.clear();
    level.deriveddata.portalcount.clear();
    level.deriveddata.roomratios.resize(boost::extents[0][0]);
    level.deriveddata.visited.resize(boost::extents[0][0][0]);
    editor.component.clear();
    level.rooms.clear();
    ROOMCOUNT = 0;
}

void resizethings(int newroomcount, bool forceresize)
{
    if (newroomcount <= ROOMCOUNT && !forceresize)
        return;

    level.entitydata.curEntityPositions.resize(newroomcount);
    level.deriveddata.portalcount.resize(newroomcount);
    level.deriveddata.roomratios.resize(boost::extents[newroomcount][SHAPESIZE]);
    level.deriveddata.visited.resize(boost::extents[newroomcount][5][2]);
    editor.component.resize(newroomcount);
    level.rooms.resize(newroomcount);
    ROOMCOUNT = newroomcount;
    allocOutlineLists(ROOMCOUNT);
}


/*********
* Functions dealing with scale
*
* Backwards-compatibility concerns lead to a rather unwieldly grading system of scales;
* different systems of scales are used for the two level objective modes, whence the two
* different functions.
*
**********/

int getscalevisit(GLfloat s)
{
    if (s < 0.0025f)
        return 11;
    else if (s < 0.005f)
        return 12;
    else if (s < 0.01f)
        return 13;
    else if (s < 0.02f)
        return 0;
    else if (s < 0.03f)
        return 9;
    else if (s < 0.06f)
        return 10;
    else if (s < 0.1f)
        return 1;
    else if (s < 0.15f)
        return 7;
    else if (s < 0.2f)
        return 2;
    else if (s < 0.25f)
        return 8;
    else if (s < 0.3f)
        return 3;
    else if (s < 0.4f)
        return 4;
    else if (s < 0.6f)
        return 5;
    else
        return 6;
}


//inverse of above
GLfloat invgetscalevisit(int s)
{
    switch (s)
    {
        case 0:
        return  0.02f;
        break;
        case 1:
        return 0.1f;
        break;
        case 2:
        return 0.2f;
        break;
        case 3:
        return 0.3f;
        break;
        case 4:
        return 0.4f;
        break;
        case 5:
        return 0.6f;
        break;
        case 6:
        return 0.7f;
        break;
        case 7:
        return 0.15f;
        break;
        case 8:
        return 0.25f;
        break;
        case 9:
        return 0.03f;
        break;
        case 10:
        return 0.06f;
        break;
        case 11:
        return 0.0025f;
        break;
        case 12:
        return 0.005f;
        break;
        case 13:
        return 0.01f;
        break;
    }
//should never get here
 return 0.7f;
}

int getscaleexplore(GLfloat s)
{
    if (s < 0.05f)
        return 0;
    else if (s < 0.1f)
        return 1;
    else if (s < 0.4f)
        return 2;
    else
        return 3;
}

/*********
* Entity movement
*
**********/

void moveEntity(Entity& E, int room)
{
    switch (E.type)
    {
    case e_EmptyRoom:
        break;
    case e_Mover:
    case e_TableMover:
    case e_BigMover:
        if (room == level.playerpos.room)
        {
            GLfloat l = length(level.playerpos.x - E.x, level.playerpos.y - E.y);

            if (l >= 0.03f)
            {
                E.x += (level.playerpos.x - E.x) * (E.dat / l);
                E.y += (level.playerpos.y - E.y) * (E.dat / l);
            }

            if (l < E.dat)
                restartlevel();
        }
        break;

    case e_Doctor:
    case e_BigDoctor:
        if (room == level.playerpos.room)
        {
            GLfloat l = length(level.playerpos.x - E.x, level.playerpos.y - E.y);

            if (l >= 0.03f)
            {
                E.x += (level.playerpos.x - E.x) * (E.dat / l); //e.x2 is used to store velocity for movers
                E.y += (level.playerpos.y - E.y) * (E.dat / l);
            }

            if (l < E.dat * 2)
                finishlevel();
        }
        break;

    case e_Sentry:
    {
        GLfloat x1 = E.x;
        GLfloat y1 = E.y;
        GLfloat x2old = x1 + cos(E.x3) * 3.0;
        GLfloat y2old = y1 + sin(E.x3) * 3.0;
        E.x3 += E.dat;                              //the angle
        //first construct the putative endpoints (px2,py2), then extend the line until it intersects with the edge...
        GLfloat x2 = x1 + cos(E.x3) * 60.0;
        GLfloat y2 = y1 + sin(E.x3) * 60.0f;
        GLfloat x3;
        GLfloat y3;
        for (int i = shapesize[level.rooms[room].shape] - 1; i >= 0; i--)
        {
            if (lineSegmentIntersection(x1, y1,
                                        x2, y2,
                                        shapes[level.rooms[room].shape][i][0], shapes[level.rooms[room].shape][i][1],
                                        shapes[level.rooms[room].shape][i + 1][0], shapes[level.rooms[room].shape][i + 1][1],
                                        &x3, &y3
                                       ))
            {
                if (room == level.playerpos.room)    //COLLISION CODE
                {
                    if (triangleintersect(x1, y1, x2old, y2old, x2, y2, level.playerpos.x, level.playerpos.y, level.playerstate.oldx, level.playerstate.oldy))
                        restartlevel();
                    else if (segmentsIntersect(x1, y1, x2old, y2old, level.playerpos.x, level.playerpos.y, level.playerstate.oldx, level.playerstate.oldy))
                        restartlevel();
                }
                E.x2 = x3;
                E.y2 = y3;
                break;
            }
        }
        break;
    }
    case e_ParityMover:
        //if x2>10, invisible in good parity, if x2>5,visible but doesn't win you the level
        if (room == level.playerpos.room && !(E.x2 > 10 && !level.playerpos.par)) //invisible one shouldn't affect visible one...
        {
            GLfloat l = length(level.playerpos.x - E.x, level.playerpos.y - E.y);

            if (l >= 0.03f)
            {
                E.x += (level.playerpos.x - E.x) * (E.dat / l); //e.x2 is used to store velocity for movers
                E.y += (level.playerpos.y - E.y) * (E.dat / l);
            }

            if (l < E.dat)
            {
                if (level.playerpos.par)
                    restartlevel();
                else if (E.x2 < 5)
                    finishlevel();
            }
        }
        break;

    case e_Wall:
        //store angle in x3, extremities in x1 and x2
        if (room == level.playerpos.room)
        {
            GLfloat l = distfromline(E.x, E.y, E.x2, E.y2, level.playerpos.x, level.playerpos.y);

            //take the midpoint;
            GLfloat x = (E.x + E.x2) / 2.0f;
            GLfloat y = (E.y + E.y2) / 2.0f;

            if (l >= 0.03f)
            {
                x += (level.playerpos.x - x) * (E.dat / l);
                y += (level.playerpos.y - y) * (E.dat / l);
            }

            E.x = x - cos(E.x3) * 60.0f;
            E.y = y - sin(E.x3) * 60.0f;

            E.x2 = x + cos(E.x3) * 60.0f;
            E.y2 = y + sin(E.x3) * 60.0f;

            GLfloat xc1;
            GLfloat yc1;
            for (int i = shapesize[level.rooms[room].shape] - 1; i >= 0; i--)
            {
                if (lineSegmentIntersection(x, y,
                                            E.x, E.y,
                                            shapes[level.rooms[room].shape][i][0], shapes[level.rooms[room].shape][i][1],
                                            shapes[level.rooms[room].shape][i + 1][0], shapes[level.rooms[room].shape][i + 1][1],
                                            &xc1, &yc1
                                           ))
                {
                    E.x = xc1;
                    E.y = yc1;
                }

                if (lineSegmentIntersection(x, y,
                                            E.x2, E.y2,
                                            shapes[level.rooms[room].shape][i][0], shapes[level.rooms[room].shape][i][1],
                                            shapes[level.rooms[room].shape][i + 1][0], shapes[level.rooms[room].shape][i + 1][1],
                                            &xc1, &yc1
                                           ))
                {
                    E.x2 = xc1;
                    E.y2 = yc1;
                }
            }

            if (l/2 <= E.dat)
                restartlevel();
            else if (segmentsIntersect(E.x, E.y, E.x2, E.y2, level.playerpos.x, level.playerpos.y, level.playerstate.oldx, level.playerstate.oldy))
                restartlevel();
        }
        break;
    case e_Goal_Table:
        if (room == level.playerpos.room)
        {
            GLfloat ls = lengthsq(level.playerpos.x - E.x, level.playerpos.y - E.y);
            const GLfloat r = 3.5 * 2.5 + 6 * 6 + 10;
            if ((r > ls) && !level.playerpos.par)
                finishlevel();
        }
        break;
    case e_Goal_Flower:
        if (room == level.playerpos.room)
        {
            GLfloat ls = lengthsq(level.playerpos.x - E.x, level.playerpos.y - E.y);
            const GLfloat r = 0.4 * 0.4;
            if ((r > ls) && (getscalevisit(level.playerpos.scale) == 5) || (getscalevisit(level.playerpos.scale) == 6))
                finishlevel();
        }
        break;
    case e_Goal_Bedroom:
        if (room == level.playerpos.room)
        {
            GLfloat ls = lengthsq(level.playerpos.x, level.playerpos.y + 3);
            const GLfloat r = 3.5 * 2.5 + 6 * 6 + 10;
            if ((r > ls) && !level.playerpos.par)
                finishlevel();
        }
        break;
    default:
        break;
    }
}

/*********
* Entity drawing
*
**********/

void drawEntity(Entity E, GLfloat a, bool par)
{
    switch (E.type)
    {
    case e_EmptyRoom:
        break;
    case e_Mover:
        c_mover[3] = a;
        glColor4fv(c_mover);
        glTranslatef(E.x, E.y, 0.0f);
        glCallList(level.displaylists.e_mlist);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_Doctor:
        c_doctor[3] = a;
        glColor4fv(c_doctor);
        glTranslatef(E.x, E.y, 0.0f);
        glCallList(level.displaylists.e_mlist);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_Sentry:
        c_mover[3] = a;
        glColor4fv(c_mover);
        glBegin(GL_LINES);
        glVertex2fv(&E.x);
        glVertex2fv(&E.x2);
        glEnd();
        break;
    case e_ParityMover:
        if (par)
        {
            if (E.x2 > 10)
                break;
            c_paritymoversafe[3] = a;
            glColor4fv(c_paritymoversafe);
        }
        else
        {
            c_paritymoverharm[3] = a;
            glColor4fv(c_paritymoverharm);
        }
        glTranslatef(E.x, E.y, 0.0f);
        glCallList(level.displaylists.e_mlist);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_Wall:
        c_wall[3] = a;
        glColor4fv(c_wall);
        glBegin(GL_LINES);
        glVertex2fv(&E.x);
        glVertex2fv(&E.x2);
        glEnd();
        break;
    case e_Goal_Table:
        c_table[3] = a;
        glColor4fv(c_table);
        glTranslatef(E.x, E.y, 0.0f);
        glCallList(level.displaylists.e_tlist);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_Goal_Flower:
        c_flower[3] = a;
        glColor4fv(c_flower);
        glTranslatef(E.x, E.y, 0.0f);
        glScalef(0.2, 0.2, 1);
        glCallList(level.displaylists.e_flist);
        glScalef(5, 5, 1);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_Table:
        c_table[3] = a;
        glColor4fv(c_table);
        glTranslatef(E.x, E.y, 0.0f);
        glCallList(level.displaylists.e_tlist);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_Bedroom:
        c_table[3] = a;
        glColor4fv(c_table);
        glTranslatef(E.x, E.y, 0.0f);
        glCallList(level.displaylists.e_bedroomlist);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_Goal_Bedroom:
        c_table[3] = a;
        glColor4fv(c_table);
        glTranslatef(E.x, E.y, 0.0f);
        glCallList(level.displaylists.e_bedroomlist2);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_Bathroom:
        c_table[3] = a;
        glColor4fv(c_table);
        glTranslatef(E.x, E.y, 0.0f);
        glCallList(level.displaylists.e_bathroomlist);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_TableMover:
        c_table[3] = a;
        glColor4fv(c_table);
        glCallList(level.displaylists.e_tlist);
        c_mover[3] = a;
        glColor4fv(c_mover);
        glTranslatef(E.x, E.y, 0.0f);
        glScalef(4, 4, 4);
        glCallList(level.displaylists.e_mlist);
        glScalef(1.0 / 4, 1.0 / 4, 1.0 / 4);
        glTranslatef(-E.x, -E.y, 0.0f);
    case e_BigMover:
        c_mover[3] = a;
        glColor4fv(c_mover);
        glTranslatef(E.x, E.y, 0.0f);
        glScalef(4,4,4);
        glCallList(level.displaylists.e_mlist);
        glScalef(0.25,0.25,0.25);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    case e_BigDoctor:
        c_doctor[3] = a;
        glColor4fv(c_doctor);
        glTranslatef(E.x, E.y, 0.0f);
        glScalef(4,4,4);
        glCallList(level.displaylists.e_mlist);
        glScalef(0.25,0.25,0.25);
        glTranslatef(-E.x, -E.y, 0.0f);
        break;
    default:
        break;
    }
    ;
}




/*********
* COLLISION DETECTION
*
* CanMove returns true if the player can move to x,y (the player coordinates being global variables here).
* If the player hits a wall, it returns false
* If the player passes through a portal, it does the moving itself, and returns false
*
* There are sometimes level objectives associated with passing through portals,
* so the function isn't exactly 'pure'
*
**********/

bool CanMove(int room, GLfloat x, GLfloat y)
{
    Room &loc = level.rooms[room];

    for (int i = shapesize[loc.shape] - 1; i >= 0; i--)
    {
        GLfloat ax = shapes[loc.shape][i][0];
        GLfloat ay = shapes[loc.shape][i][1];
        GLfloat bx = shapes[loc.shape][i + 1][0];
        GLfloat by = shapes[loc.shape][i + 1][1];

        if (segmentsIntersect(0, 0, x, y, ax, ay, bx, by))
        {
            if (loc.portal[i])
            {
                if (level.levelparams.mirrorexit)
                {
                    if (level.playerpos.room == loc.portaltoroom[i] && !loc.portalparity[i] && loc.portaltoedge[i] == i)
                    {
                        finishlevel();
                        return false;
                    }
                    else if (level.levelparams.timer == 5)                    //can turn off portals for bootcamp levels
                    {
                        return false;
                    }
                }

                {
                    level.playerpos.room = loc.portaltoroom[i];

                    if (editmode)
                    {
                        editor.realwallselected = loc.portaltoedge[i];
                    }

                    Room &newloc = level.rooms[level.playerpos.room];

                    GLfloat r = MidpointRatio(ax, ay, bx, by, x, y);
                    GLfloat cx = shapes[newloc.shape][loc.portaltoedge[i]][0];
                    GLfloat cy = shapes[newloc.shape][loc.portaltoedge[i]][1];
                    GLfloat dx = shapes[newloc.shape][loc.portaltoedge[i] + 1][0];
                    GLfloat dy = shapes[newloc.shape][loc.portaltoedge[i] + 1][1];

                    if (loc.portalparity[i] == 0)
                    {
                        level.playerpos.par = !level.playerpos.par;
                        level.playerpos.dir = -level.playerpos.dir - angle(dx - cx, dy - cy) - angle(bx - ax, by - ay);

                        level.playerpos.x = cx * (1 - r) + dx * (r);
                        level.playerpos.y = cy * (1 - r) + dy * (r);
                    }
                    else
                    {
                        level.playerpos.dir -= angle(dx - cx, dy - cy, bx - ax, by - ay) + Pi;
                        level.playerpos.x = dx * (1 - r) + cx * (r);
                        level.playerpos.y = dy * (1 - r) + cy * (r);
                    }

                    level.playerpos.scale *= level.deriveddata.roomratios[room][i];

                    if (level.levelparams.enterroom)
                    {
                        if (level.playerpos.room == level.levelparams.goalroom)
                        {
                            if (level.levelparams.goalroomparity < 0 || level.levelparams.goalroomparity == level.playerpos.par)
                                if (
                                (level.levelparams.goalroomscale < 0)
                                ||//use timer to store the condition for goal room scale stuff...
                                ((level.levelparams.timer==0) &&level.levelparams.goalroomscale == getscalevisit(level.playerpos.scale))
                                ||
                                ((level.levelparams.timer>0) &&invgetscalevisit( level.levelparams.goalroomscale) <= (level.playerpos.scale))
                                ||
                                ((level.levelparams.timer<0) &&invgetscalevisit( level.levelparams.goalroomscale) >= (level.playerpos.scale))
                                )
                                {
                                    levelshow();
                                    SDL_GL_SwapBuffers();
                                    finishlevel();
                                }
                        }
                    }
                    else if (level.levelparams.exploreon)
                    {
                        if (!level.deriveddata.visited[level.playerpos.room][getscaleexplore(level.playerpos.scale)][!level.playerpos.par])
                        {
                            if (level.levelparams.scaleandparitymatter)
                                level.deriveddata.visited[level.playerpos.room][getscaleexplore(level.playerpos.scale)][!level.playerpos.par] = true;
                            else if (level.levelparams.scalematters)
                            {
                                level.deriveddata.visited[level.playerpos.room][getscaleexplore(level.playerpos.scale)][0] = true;
                                level.deriveddata.visited[level.playerpos.room][getscaleexplore(level.playerpos.scale)][1] = true;
                            }
                            else if (level.levelparams.paritymatters)
                            {
                                for (int i = 0; i < 5; i++)
                                    level.deriveddata.visited[level.playerpos.room][i][!level.playerpos.par] = true;
                            }
                            else
                            {
                                for (int i = 0; i < 5; i++)
                                {
                                    level.deriveddata.visited[level.playerpos.room][i][0] = true;
                                    level.deriveddata.visited[level.playerpos.room][i][1] = true;
                                }
                            }
                            if (!editmode)
                                level.levelparams.roomstoexplore--;
                            if (level.levelparams.roomstoexplore <= 0)
                            {
                                levelshow();
                                SDL_GL_SwapBuffers();
                                finishlevel();
                            }
                        }
                    }
                }
                return false;
            }
            return false;
        }
    }
    return true;
}


/*********
* Room rendering
*
* the first five parameters index the portal
*
* glob parity stores the orientation of the room drawn before
*
* depth and realdepth store information about how many rooms to traverse;
*   one is related to the modelview stack depth,
*   the other is related to the total number of rooms that will be drawn.
*
* outers gives the scale the last room was drawn at
*
**********/

void RenderRoom(int room_t, int room_f, int edge_t, int edge_f, bool parity, bool globparity, GLfloat depth, GLfloat realdepth, GLfloat outers)
{
    glPushMatrix();

    /*
        Step 1: do the transformation
    */

    //old portal
    int fromshape = level.rooms[room_f].shape;
    GLfloat xa0 = shapes[fromshape][edge_f][0];
    GLfloat ya0 = shapes[fromshape][edge_f][1];
    GLfloat xa1 = shapes[fromshape][edge_f + 1][0];
    GLfloat ya1 = shapes[fromshape][edge_f + 1][1];

    //new portal
    int toshape = level.rooms[room_t].shape;
    GLfloat xb0 = shapes[toshape][edge_t][0];
    GLfloat yb0 = shapes[toshape][edge_t][1];
    GLfloat xb1 = shapes[toshape][edge_t + 1][0];
    GLfloat yb1 = shapes[toshape][edge_t + 1][1];
    GLfloat a;

    if (parity)
    {
        glTranslatef(xa0, ya0, 0.0f);
        a = angle(xb1 - xb0, yb1 - yb0, xa1 - xa0, ya1 - ya0) + Pi;
    }
    else
    {
        glTranslatef(xa1, ya1, 0.0f);
        glMultMatrixf(*flip_y);
        a = angle(xb1 - xb0, yb1 - yb0) + angle(xa1 - xa0, ya1 - ya0);
    }

    glRotatef(a * (180.0 / Pi), 0.0f, 0.0f, 1.0f);

    GLfloat s = level.deriveddata.roomratios[room_f][edge_f];
    if (s != 1.0f)
        glScalef(s, s, 1);

    outers *= s;
    glTranslatef(-xb1, -yb1, 0.0f);


    /*
        Step 2: draw all the rooms connecting out from this one
    */

    for (int i = shapesize[toshape] - 1; i >= 0; i--)
        if (i != edge_t)
            if ((level.rooms[room_t].portal[i]) && (depth >= 1) && (realdepth < (MAXREALDEPTH - 1)) && (!level.rooms[room_t].localportal[i]))
                RenderRoom(level.rooms[room_t].portaltoroom[i],
                           room_t, level.rooms[room_t].portaltoedge[i],
                           i,
                           level.rooms[room_t].portalparity[i],
                           (!globparity) ^ level.rooms[room_t].portalparity[i],
                           (depth - 1) / (level.deriveddata.portalcount[room_t] - 0.99f),
                           realdepth + 1,
                           outers);

    /*
      Step 3: figure out what colour to use
    */

    GLfloat* curcolour;

    if (level.deriveddata.visited[room_t][getscaleexplore(outers)][globparity])
    {
        if (globparity)
            curcolour = &level.colourscheme.evenroomvisited[0];
        else
            curcolour = &level.colourscheme.oddroomvisited[0];
    }
    else
    {
        if (globparity)
            curcolour = &level.colourscheme.evenroomunvisited[0];
        else
            curcolour = &level.colourscheme.oddroomunvisited[0];
    }

    /*
      Step 4: draw the room
    */

    GLfloat alpha = sqrt(0.9f * min((depth) / MAXDEPTH, (MAXREALDEPTH - realdepth) / MAXREALDEPTH));

    curcolour[3] = (level.colourscheme.roomtransparency ? alpha : 1.0f);

    glColor4fv(curcolour);

    glCallList(level.displaylists.shapelist + level.rooms[room_t].shape);

    level.colourscheme.outline[3] = (level.colourscheme.edgetransparency ? alpha : 1.0f);

    glColor4fv(level.colourscheme.outline);
    glCallList(level.displaylists.roomoutlines + room_t);

    if (level.entitydata.curEntityPositions[room_t].type != e_EmptyRoom)
        drawEntity(level.entitydata.curEntityPositions[room_t], (level.colourscheme.entitytransparency ? alpha : 1.0f), globparity);

    if ((level.playerpos.room == room_t) && (!level.colourscheme.noplayerdup))
    {
        c_player[3] = alpha;
        glColor4fv(c_player);

        glBegin(GL_LINES);
        //TODO: store player x,y vals in struct so can use vertex2fv
        glVertex2f(level.playerpos.x, level.playerpos.y);

        glVertex2f(level.playerpos.x + cos(level.playerpos.dir) * (pl_s / (level.playerpos.scale)), level.playerpos.y + sin(level.playerpos.dir) * (pl_s / (level.playerpos.scale)));
        glEnd();
    }

    glPopMatrix();
}


/*********
* Set up the Display lists
*
**********/

/*
  this function builds the outlines for all of the rooms
*/

void buildOutlines()
{
    allocOutlineLists(ROOMCOUNT);

    for (int j = 0; j < ROOMCOUNT; j++)
    {
        glNewList(level.displaylists.roomoutlines + j, GL_COMPILE);

        for (int i = shapesize[level.rooms[j].shape] - 1; i >= 0; i--)
        {
            if (!level.rooms[j].portal[i])
            {
                glBegin(GL_LINES);
                glVertex2fv(&shapes[level.rooms[j].shape][i][0]);
                glVertex2fv(&shapes[level.rooms[j].shape][i + 1][0]);
                glEnd();
            }
        }
        glEndList();
    }

    //Entities Draw

    level.displaylists.e_mlist = glGenLists(1);

    glNewList(level.displaylists.e_mlist, GL_COMPILE);

    glBegin(GL_QUADS);
    glVertex2f(-0.05f, +0.05f);
    glVertex2f(+0.05f, +0.05f);
    glVertex2f(+0.05f, -0.05f);
    glVertex2f(-0.05f, -0.05f);
    glEnd();
    glEndList();



    level.displaylists.e_plist = glGenLists(1);
    glNewList(level.displaylists.e_plist, GL_COMPILE);
    glBegin(GL_QUADS);
    glVertex2f(-0.05f, +0.05f);
    glVertex2f(+0.05f, +0.05f);
    glVertex2f(+0.05f, -0.05f);
    glVertex2f(-0.05f, -0.05f);
    glEnd();
    glEndList();


    //Bedroom
    level.displaylists.e_bedroomlist = glGenLists(1);

    glNewList(level.displaylists.e_bedroomlist, GL_COMPILE);

    for (int i = 0; i < SUBOBJECTCOUNT; i++)
    {
        if (m_bedroom[i][0][0] > E)
            break;

        glBegin(GL_LINE_LOOP);

        for (int j = 0; j < POLYSIZE; j++)
        {
            if (m_bedroom[i][j][0] > E)
                break;

            glVertex2f(m_bedroom[i][j][0], m_bedroom[i][j][1]);
        }

        glEnd();
    }

    glEndList();

    //Bedroom2
    level.displaylists.e_bedroomlist2 = glGenLists(1);

    glNewList(level.displaylists.e_bedroomlist2, GL_COMPILE);

    for (int i = 0; i < SUBOBJECTCOUNT; i++)
    {
        if (m_bedroom2[i][0][0] > E)
            break;

        glBegin(GL_LINE_LOOP);

        for (int j = 0; j < POLYSIZE; j++)
        {
            if (m_bedroom2[i][j][0] > E)
                break;

            glVertex2f(m_bedroom2[i][j][0], m_bedroom2[i][j][1]);
        }

        glEnd();
    }

    glEndList();

    //Bathroom
    level.displaylists.e_bathroomlist = glGenLists(1);

    glNewList(level.displaylists.e_bathroomlist, GL_COMPILE);

    for (int i = 0; i < SUBOBJECTCOUNT; i++)
    {
        if (m_bathroom[i][0][0] > E)
            break;

        glBegin(GL_LINE_LOOP);

        for (int j = 0; j < POLYSIZE; j++)
        {
            if (m_bathroom[i][j][0] > E)
                break;

            glVertex2f(m_bathroom[i][j][0], m_bathroom[i][j][1]);
        }

        glEnd();
    }

    glEndList();

    level.displaylists.e_tlist = glGenLists(1);

    glNewList(level.displaylists.e_tlist, GL_COMPILE);

    for (int i = 0; i < SUBOBJECTCOUNT; i++)
    {
        if (m_table[i][0][0] > E)
            break;

        glBegin(GL_LINE_LOOP);

        for (int j = 0; j < POLYSIZE; j++)
        {
            if (m_table[i][j][0] > E)
                break;

            glVertex2f(m_table[i][j][0], m_table[i][j][1]);
        }

        glEnd();
    }

    glEndList();

    level.displaylists.e_flist = glGenLists(1);

    glNewList(level.displaylists.e_flist, GL_COMPILE);

    for (int i = 0; i < SUBOBJECTCOUNT; i++)
    {
        if (m_table[i][0][0] > E)
            break;

        glBegin(GL_LINE_LOOP);

        for (int j = 0; j < POLYSIZE; j++)
        {
            if (m_flower[i][j][0] > E)
                break;

            glVertex2f(m_flower[i][j][0], m_flower[i][j][1]);
        }

        glEnd();
    }

    glEndList();
}

/*
  this builds the solid polygons that constitute the rooms themselves
*/

void buildLists()
{
    //the rooms
    level.displaylists.shapelist = glGenLists(SHAPECOUNT);

    for (int i = 0; i < SHAPECOUNT; i++)
    {
        glNewList(level.displaylists.shapelist + i, GL_COMPILE);

        glBegin(GL_POLYGON);
        for (int j = shapesize[i] - 1; j >= 0; j--)
        {
            glVertex2f(shapes[i][j][0], shapes[i][j][1]);
        }
        glEnd();

        glEndList();                                                                              // Done Building The box List
    }
}


bool init_GL()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DITHER);
    glShadeModel(GL_FLAT);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    GLint maxhardwaredepth;
    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &maxhardwaredepth);
    MAXREALDEPTH = min(maxhardwaredepth - 2, MAXREALDEPTH);

    allocOutlineLists(ROOMCOUNT);
    buildLists();
    glClearColor(0, 0, 0, 1);

    if (glGetError() != GL_NO_ERROR)
    {
        return false;
    }

    return true;
}

/*********
* Set up the game
*
**********/

bool init()
{
    editmode = false;

    episodeList.clear();
    customEpisodeList.clear();

    getEpisodeData();

    //Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        return false;
    }


    load_config_vars();
    MAXDEPTH = getmaxdepth();
    MAXREALDEPTH = getmaxrealdepth();
    SCREEN_WIDTH = getscreenwidth();
    SCREEN_HEIGHT = getscreenheight();
    FULLSCREEN = getfullscreen();

    //should add extra aspect ratio code here at some point...

    SDL_EnableUNICODE(true);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);


    if (FULLSCREEN)
    {
        //Create Window
        if (SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_OPENGL | SDL_FULLSCREEN) == NULL)
        {
            return false;
        }
    }
    else
    {
        //Create Window
        if (SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_OPENGL /* | SDL_FULLSCREEN*/) == NULL)
        {
            return false;
        }
    }

    SDL_ShowCursor(SDL_DISABLE);

    if (init_GL() == false)
    {
        return false;
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    initsound();

    SDL_WM_SetCaption("Mirror Stage", NULL);

    generateShapeScales();

    loadsavedata();

    return true;
}

/*********
* Called when quitting game
*
**********/

void clean_up()
{
    releasesound();
    SDL_Quit();
}

/*********
* The following is used to generate thumbnail images for the rooms;
* it finds their sizes, so the editor can later scale them down to
* fit in a small panel
*
**********/

void generateShapeScales()
{
    for (int i = 0; i < SHAPECOUNT; i++)
    {
        roomNormalizations[i] = 0.5;

        for (int j = 0; j < shapesize[i]; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                if (fabs(shapes[i][j][k]) > roomNormalizations[i])
                    roomNormalizations[i] = fabs(shapes[i][j][k]);
            }
        }
    }
}

/*********
* This calculates the scale-changes between each portal.
* originally I calculated this dynamically, but given that there's
* no portal gun, it is neater to calculate it statically.
*
**********/


void buildRoomRatios()
{
    for (int r = 0; r < ROOMCOUNT; r++)
        for (int e = 0; e < SHAPESIZE; e++)
            if (level.rooms[r].portal[e])
            {
                GLfloat ax = shapes[level.rooms[r].shape][e][0];
                GLfloat ay = shapes[level.rooms[r].shape][e][1];
                GLfloat bx = shapes[level.rooms[r].shape][e + 1][0];
                GLfloat by = shapes[level.rooms[r].shape][e + 1][1];

                int r2 = level.rooms[r].portaltoroom[e];
                GLfloat cx = shapes[level.rooms[r2].shape][level.rooms[r].portaltoedge[e]][0];
                GLfloat cy = shapes[level.rooms[r2].shape][level.rooms[r].portaltoedge[e]][1];
                GLfloat dx = shapes[level.rooms[r2].shape][level.rooms[r].portaltoedge[e] + 1][0];
                GLfloat dy = shapes[level.rooms[r2].shape][level.rooms[r].portaltoedge[e] + 1][1];

                level.deriveddata.roomratios[r][e] = sqrt(lengthsq(bx - ax, by - ay) / lengthsq(dx - cx, dy - cy));
            }
}


/*********
* Counts the number of portals in each room; used
* in rendering calculations.
*
**********/

void countPortals()
{
    for (int i = 0; i < ROOMCOUNT; i++)
    {
        level.deriveddata.portalcount[i] = 0;

        for (int j = shapesize[level.rooms[i].shape] - 1; j >= 0; j--)
        {
            if (level.rooms[i].portal[j])
                level.deriveddata.portalcount[i]++;
        }
    }
}

/*********
* Load and initialize level
*
**********/

void initlevel()
{
    editmode = editorenabled;
    editor.allowmove=true;
    editor.prompttodelete=false;

    int l = curlevel;
    level.playerpos.room = 0;

    editor.realwallselected = 0;
    editor.newportalparity = true;

    clearsizes();

    char blah[50];
    if (!customchapter)
        sprintf(blah, "chapters/%s/%d.dat", curchapter.c_str(), l);
    else
        sprintf(blah, "custom/%s/%d.dat", curchapter.c_str(), l);
    readlevel(blah, level);

    level.playerstate.oldx = level.playerpos.x;
    level.playerstate.oldy = level.playerpos.y;
    level.playerstate.rotatingl = false;
    level.playerstate.rotatingr = false;
    level.playerstate.movingforward = false;
    level.playerstate.movingback = false;

    //don't load a new track for text only levels
    if ((level.levelparams.exploreon ||
            level.levelparams.enterroom ||
            level.levelparams.mirrorexit ||
            level.levelparams.timeron || (level.levelparams.goalroom == -1)))
        loadtrack(level.levelparams.music);

    resizethings(level.rooms.size());

    /* STANDARD INITIALIZATION CODE BEGIN */

    level.playerstate.movingforward = 0;
    level.playerstate.movingback = 0;
    level.playerstate.rotatingl = 0;
    level.playerstate.rotatingr = 0;

    /* STANDARD INITIALIZATION CODE END */


    /* RUNTIME GENERATION BEGIN */

    countPortals();

    for (int i = 0; i < ROOMCOUNT; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 2; k++)
                level.deriveddata.visited[i][j][k] = false;

    if (level.levelparams.exploreon)
    {
        level.levelparams.scaleandparitymatter = level.levelparams.scalematters && level.levelparams.paritymatters;

        if (level.levelparams.scaleandparitymatter)
            level.deriveddata.visited[level.playerpos.room][getscaleexplore(level.playerpos.scale)][!level.playerpos.par] = true;
        else if (level.levelparams.scalematters)
        {
            level.deriveddata.visited[level.playerpos.room][getscaleexplore(level.playerpos.scale)][0] = true;
            level.deriveddata.visited[level.playerpos.room][getscaleexplore(level.playerpos.scale)][1] = true;
        }
        else if (level.levelparams.paritymatters)
        {
            for (int i = 0; i < 5; i++)
                level.deriveddata.visited[level.playerpos.room][i][!level.playerpos.par] = true;
        }
        else
        {
            for (int i = 0; i < 5; i++)
            {
                level.deriveddata.visited[level.playerpos.room][i][0] = true;
                level.deriveddata.visited[level.playerpos.room][i][1] = true;
            }
        }
    }

    buildRoomRatios();
    buildOutlines();

    /* RUNTIME GENERATION END */

    if (!editmode)
    {
        glClearColor(0, 0, 0, 1);
        gamestate = gs_interlude;
    }
    else
    {
        glClearColor(level.colourscheme.background[0], level.colourscheme.background[1], level.colourscheme.background[2], 1.0f);
        gamestate = gs_level;
    }

}

/*********
* Restart level
*
**********/

void restartlevel()
{
    if (!editmode && !firstframe)
    {
        fadetoblack();
        initlevel();
    }
}

/*********
* Finish level
*
**********/

void finishlevel()
{
    if (editmode || firstframe || editorenabled)
        return;

    if (curlevel < (episodecount - 1))
    {
        if (!customchapter)
        {
            if (savedata.completed[curtitleselection2][curlevel] != 1)
            {
                savedata.completed[curtitleselection2][curlevel] = 1;
                savesavedata();
            }
        }

        curlevel++;
        fadetoblack();
        initlevel();
    }
    else
    {
        if (!customchapter)
        {
            if (savedata.completed[curtitleselection2][curlevel] != 1)
            {
                savedata.completed[curtitleselection2][curlevel] = 1;
                savesavedata();
            }
        }

        fadetoblack();
        wantquit = true;
        gamestate = gs_finishchapter;
    }
}


void levelinput()
{
    //If a key was pressed
    if (event.type == SDL_KEYDOWN)
    {
        //Adjust the velocity
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            level.playerstate.movingforward = 1;
            break;

        case SDLK_DOWN:
            level.playerstate.movingback = 1;
            break;

        case SDLK_LEFT:
            level.playerstate.rotatingl = 1;
            break;

        case SDLK_RIGHT:
            level.playerstate.rotatingr = 1;
            break;

            // TODO: make esc go from level to level select to title to exit...
        case SDLK_SPACE:
        case SDLK_RETURN:
            editor.input(' ');
            break;

        case SDLK_q:
            editor.input('q');
            break;

        case SDLK_w:
            editor.input('w');
            break;

        case SDLK_e:
            editor.input('e');
            break;

        case SDLK_a:
            editor.input('a');
            break;

        case SDLK_s:
            editor.input('s');
            break;

        case SDLK_d:
            editor.input('d');
            break;

        case SDLK_z:
            editor.input('z');
            break;

        case SDLK_c:
            editor.input('c');
            break;

        case SDLK_r:
            editor.input('r');
            break;

        case SDLK_f:
            editor.input('f');
            break;

        case SDLK_v:
            editor.input('v');
            break;

        case SDLK_t:
            editor.input('t');
            break;

        case SDLK_x:
            editor.input('x');
            break;

        case SDLK_LEFTBRACKET:
            editor.input('[');
            break;

        case SDLK_RIGHTBRACKET:
            editor.input(']');
            break;

        case SDLK_F1:
            editor.setPage(E_FileStuff);
            break;

        case SDLK_F2:
            editor.setPage(E_ColourScheme);
            break;

        case SDLK_F3:
            editor.setPage(E_LevelGoals);
            break;

        case SDLK_F4:
            editor.setPage(E_EntityChooser);
            break;

        case SDLK_F5:
            editor.setPage(E_TextChooser);
            break;

        case SDLK_F6:
            editor.setPage(E_RoomAdder);
            break;

        case SDLK_F7:
            editor.setPage(E_PortalMode);
            break;

        case SDLK_F8:
            editor.setPage(E_Other);
            break;

        case SDLK_1:
            if (editmode)
            {
                if (editor.page == E_TextChooser)
                    break;

                curlevel = (curlevel - 1 + episodecount) % episodecount;
                curtitleselection3 = curlevel;
                initlevel();
                editor.itemselected = 0;
            }
            break;

        case SDLK_2:
            if (editmode)
            {
                if (editor.page == E_TextChooser)
                    break;
                curlevel = (curlevel + 1) % episodecount;
                curtitleselection3 = curlevel;
                initlevel();
                editor.itemselected = 0;
            }
            break;
        case SDLK_m:
            if (editmode)
            {
                editor.allowmove=!editor.allowmove;
            }

        default:
            break;
        }

        //deal with text input here as well

        if (editmode && (editor.page == E_TextChooser))
        {
            if (event.key.keysym.sym == SDLK_BACKSPACE)
            {
                level.introtext = level.introtext.substr(0, level.introtext.length() - 1);
            }
            else if (event.key.keysym.sym == SDLK_RETURN)
            {
                level.introtext.resize((level.introtext.length() + 16) - ((level.introtext.length() + 16) % 16), ' ');
            }
            else if (event.key.keysym.unicode < 0x80 && event.key.keysym.unicode > 0)
            {
                char ch = (char)event.key.keysym.unicode;
                level.introtext.append(1, tolower(ch));
                if (level.introtext.length() > 176)
                    level.introtext.resize(176);
            }
        }
    }
    //If a key was released
    else if (event.type == SDL_KEYUP)
    {
        //Adjust the velocity
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
            level.playerstate.movingforward = 0;
            break;

        case SDLK_DOWN:

            level.playerstate.movingback = 0;
            break;

        case SDLK_LEFT:
            level.playerstate.rotatingl = 0;
            break;

        case SDLK_RIGHT:
            level.playerstate.rotatingr = 0;
            break;

        case SDLK_SPACE:
            //if (!editmode)
            //{
            //space does nothing for now; have esc take back to title screen

            /*
               if (curlevel==0)
                    initlevel(level.playerpos.room+1);
               else
                    initlevel(0);*/
            //}
            break;

        case SDLK_TAB:
        {
            if (editorenabled)
            {
                editmode = !editmode;
                if (editmode == true)
                {
                    editor.realwallselected = 0;
                }
            }
        }
        break;

        case SDLK_w:
            break;

        case SDLK_s:
            break;

        case SDLK_ESCAPE:
            if (editorenabled)
            {
                if (!editmode)
                    editmode = true;
                else
                {
                    if (editor.page != E_FileStuff)
                    {
                        editor.page = E_FileStuff;
                        editor.itemselected = 1;                                //select "save"
                    }
                    else
                    {
                        //go to save as screen
                        wantquit = true;
                        gamestate = gs_title;
                    }
                }
            }
            else
            {
                fadetoblack(false);
                wantquit = true;
                gamestate = gs_title;
            }
            break;

        default:
            break;
        }
    }
}


void interludeinput()
{
    if (event.type == SDL_KEYDOWN)
    {
        //Adjust the velocity
        switch (event.key.keysym.sym)
        {
        default:
            break;
        }
    }
    //If a key was released
    else if (event.type == SDL_KEYUP)
    {
        //Adjust the velocity
        switch (event.key.keysym.sym)
        {
        case SDLK_SPACE:
        case SDLK_RETURN:
            if (!(level.levelparams.exploreon ||
                    level.levelparams.enterroom ||
                    level.levelparams.mirrorexit ||
                    level.levelparams.timeron || (level.levelparams.goalroom == -1)))
                finishlevel();
            else
            {
                glClearColor(level.colourscheme.background[0], level.colourscheme.background[1], level.colourscheme.background[2], 1);
                gamestate = gs_level;
            }
            break;

        case SDLK_ESCAPE:
            wantquit = true;
            gamestate = gs_title;
            break;

        default:
            break;
        }
    }
}


/*********
* Game initialization
*
**********/

Game::Game()
{
    gamestate = gs_intro;
}

void Game::handle_input()
{
    if (gamestate == gs_level)
        levelinput();
    else
        interludeinput();
}

void Game::move()
{
    if (gamestate == gs_level)
        levelmove();
    else
        interludemove();
}

void Game::show()
{
    if (gamestate == gs_level)
    {
        levelshow();
    }
    else if (!wantquit)
    {
        interludeshow();
    }
}

/*********
* Basic movement code put here
*
**********/

void levelmove()
{
    //these are used for collision-detection


    /*  move player    */
    if (level.playerstate.movingforward)
    {
        GLfloat dpx = pspeed * cos(level.playerpos.dir) / level.playerpos.scale;
        GLfloat dpy = pspeed * sin(level.playerpos.dir) / level.playerpos.scale;
        if (CanMove(level.playerpos.room, level.playerpos.x + dpx, level.playerpos.y + dpy))
        {
            level.playerstate.oldx = level.playerpos.x;
            level.playerstate.oldy = level.playerpos.y;
            level.playerpos.x += dpx;
            level.playerpos.y += dpy;
        }
        else
        {
            level.playerstate.oldx = level.playerpos.x;
            level.playerstate.oldy = level.playerpos.y;
        }
        //   moving=false;
    }
    else if (level.playerstate.movingback)
    {
        GLfloat dpx = pspeed * cos(level.playerpos.dir) / level.playerpos.scale;
        GLfloat dpy = pspeed * sin(level.playerpos.dir) / level.playerpos.scale;
        if (CanMove(level.playerpos.room, level.playerpos.x - dpx, level.playerpos.y - dpy))
        {
            level.playerstate.oldx = level.playerpos.x;
            level.playerstate.oldy = level.playerpos.y;
            level.playerpos.x -= dpx;
            level.playerpos.y -= dpy;
        }
        else
        {
            level.playerstate.oldx = level.playerpos.x;
            level.playerstate.oldy = level.playerpos.y;
        }
    }

    if (level.playerstate.rotatingr)
    {
        level.playerpos.dir -= (level.playerpos.par * 2 - 1) * 0.06f;
    }
    if (level.playerstate.rotatingl)
    {
        level.playerpos.dir += (level.playerpos.par * 2 - 1) * 0.06f;
    }

    if (!editmode)
    {
        if (level.levelparams.timeron)
        {
            level.levelparams.timer--;
            if (level.levelparams.timer <= 0)
                finishlevel();
        }

    }
    if ((!editmode) || editor.allowmove)
    {

        for (int i = 0; i < ROOMCOUNT; i++)
        {
            if (level.entitydata.curEntityPositions[i].type != e_EmptyRoom)
                moveEntity(level.entitydata.curEntityPositions[i], i);
        }
    }

}


void interludemove()
{
}

/*********
* Room drawing code here
*
* Contains some duplicate code from renderRoom; the drawing procedure
* for the first room is almost the same as, but *not quite*, that of
* other rooms.  It was easier to duplicate the code than to try separate
* out some function common to both.
*
**********/

void levelshow()
{

    glClear(GL_COLOR_BUFFER_BIT);                   // | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glPushMatrix();

    //set up coordinates here to center player

    glScalef(level.playerpos.scale, level.playerpos.scale, 1);

    if (!level.playerpos.par)
    {
        glMultMatrixf(*flip_x);
    }

    //set up rotation
    glRotatef(90 - level.playerpos.dir * (180.0 / Pi), 0.0f, 0.0f, 1.0f);

    //center position
    glTranslatef(-level.playerpos.x, -level.playerpos.y, 0.0f);

    glTranslatef(0.0f, 0.0f, 0.1f);

    for (int i = shapesize[level.rooms[level.playerpos.room].shape] - 1; i >= 0; i--)
        if (level.rooms[level.playerpos.room].portal[i])
        {
            RenderRoom(level.rooms[level.playerpos.room].portaltoroom[i], level.playerpos.room, level.rooms[level.playerpos.room].portaltoedge[i], i, level.rooms[level.playerpos.room].portalparity[i], (level.playerpos.par) ^ level.rooms[level.playerpos.room].portalparity[i], MAXDEPTH / (level.deriveddata.portalcount[level.playerpos.room]), 0, level.playerpos.scale);
        }

    /*
        choose room color
    */

    GLfloat* curcolour;


    if (level.deriveddata.visited[level.playerpos.room][getscaleexplore(level.playerpos.scale)][!level.playerpos.par])
    {
        if (!level.playerpos.par)
            curcolour = &level.colourscheme.evenroomvisited[0];
        else
            curcolour = &level.colourscheme.oddroomvisited[0];
    }
    else
    {
        if (!level.playerpos.par)
            curcolour = &level.colourscheme.evenroomunvisited[0];
        else
            curcolour = &level.colourscheme.oddroomunvisited[0];
    }

    glColor3fv(curcolour);

    glCallList(level.displaylists.shapelist + level.rooms[level.playerpos.room].shape);

    glColor3fv(level.colourscheme.outline);

    glCallList(level.displaylists.roomoutlines + level.playerpos.room);


    drawEntity(level.entitydata.curEntityPositions[level.playerpos.room], 1.0f, !level.playerpos.par);

    //draw the player

    c_player[3] = 1.0f;
    glColor4fv(c_player);

    glBegin(GL_LINES);
    //TODO: store player x,y vals in struct so can use vertex2fv
    glVertex2f(level.playerpos.x, level.playerpos.y);

    glVertex2f(level.playerpos.x + cos(level.playerpos.dir) * (pl_s / (level.playerpos.scale)), level.playerpos.y + sin(level.playerpos.dir) * (pl_s / (level.playerpos.scale)));
    glEnd();


    /*
      if you have the editor window open, and are in a mode where you can select walls, then draw the selection:
    */

    if (editmode && (editor.page == E_RoomAdder || editor.page == E_PortalMode))
    {
        glColor3f(0, 0.7, 0);
        glBegin(GL_TRIANGLES);
        glVertex2f(level.playerpos.x, level.playerpos.y);
        glVertex2f(shapes[level.rooms[level.playerpos.room].shape][editor.realwallselected][0], shapes[level.rooms[level.playerpos.room].shape][editor.realwallselected][1]);
        glVertex2f(shapes[level.rooms[level.playerpos.room].shape][editor.realwallselected + 1][0], shapes[level.rooms[level.playerpos.room].shape][editor.realwallselected + 1][1]);
        glEnd();
    }

    glPopMatrix();
}




void interludeshow()
{
    glClear(GL_COLOR_BUFFER_BIT);                   // | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glPushMatrix();
    print_text(level.introtext.c_str());

    glPopMatrix();
}

void fadetoblack(bool fade)
{
    Timer fps;

    glClearColor(0, 0, 0, 1);

    for (float alpha = 0; alpha <= 1; alpha += 0.03)
    {
        if (fade)
            setvolume(1 - alpha);

        fps.start();


        glDisable(GL_BLEND);
        glReadBuffer(GL_FRONT);
        glDrawBuffer(GL_BACK);
        glCopyPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR);
        glEnable(GL_BLEND);

        while (SDL_PollEvent(&event))
        {
            //Gobble up all events while fading
        }

        glLoadIdentity();

        glColor4f(0, 0, 0, alpha);
        glBegin(GL_QUADS);
        glVertex2f(-3, -3);
        glVertex2f(-3, 3);
        glVertex2f(3, 3);
        glVertex2f(3, -3);
        glEnd();

        if (fps.get_ticks() < 1000 / FRAMES_PER_SECOND)
        {
            SDL_Delay((1000 / FRAMES_PER_SECOND) - fps.get_ticks());
        }


        SDL_GL_SwapBuffers();
    }
}

bool draw_intro()
{
    static GLfloat t;
    static GLfloat u;
    static GLfloat v = 1;

    if (t < 1)
        t += 0.02;
    else
    {
        if (u < 1.5)
            u += 0.02;
        else
        {
            if (v > 0)
                v -= 0.04;
            else return false;
        }
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glColor4f(0.7, 0.7, 0.7, min(t, v));
    glBegin(GL_QUADS);
    glVertex2f(-0.5f, +0.5f);
    glVertex2f(+0.5f, +0.5f);
    glVertex2f(+0.5f, -0.5f);
    glVertex2f(-0.5f, -0.5f);
    glEnd();

    glColor4f(0.5, 0.5, 0.5, min(1, v));
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.4, -2.0f + 1.55 * t);
    glVertex2f(+0.4f, -2.0f + 1.55 * t);
    glVertex2f(0.0f, -1.1f + 1.55 * t);
    glEnd();


    glColor4f(0.4, 0.4, 0.4, min(u, v));
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.4 + 0.45, 2.0f - 1.525);
    glVertex2f(+0.4f + 0.45, 2.0f - 1.525);
    glVertex2f(0.0f + 0.45, 1.1f - 1.525);
    glEnd();

    glTranslatef(-0.7, 0.05, 0);
    glScalef(1.5, 1.5, 1.5);
    glColor4f(1, 1, 1, min(u, v));
    print_straight_text (_("increpare presents"));
    glTranslatef(0, -0.15, 0);

    SDL_GL_SwapBuffers();

    return true;
}


bool draw_finishchapter()
{
    static GLfloat t = 0;

    if (t < 25)
        t += 0.005;
    else
    {
        return false;
    }

    glLoadIdentity();

    //	glClearColor(sin(2*t+1)/2+0.5,sin(t)/2+0.5,sin(t/2+1)/2+0.5,1);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);



    glTranslatef(-0.7, 0.05, 0);
    glScalef(1.5, 1.5, 1.5);
//	glColor3f(0.5-sin(2*t+1)/2,0.5-sin(t)/2,0.5-sin(t/2+1)/2);
    glColor4f(1, 1, 1, 1);
    print_straight_text(_("chapter over"));
    glTranslatef(0, -0.15, 0);


    SDL_GL_SwapBuffers();

    return true;
}


static GLfloat titletick = -0.5;

void draw_editmodenochapter()
{
    if (titletick < 1)
        titletick += 0.01;

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();


    {
        glTranslatef(-0.31, 0, 0);

        glColor3f(1, 1, 1);
        glBegin(GL_LINES);
        glVertex2f(0.3, 1);
        glVertex2f(0.3, -2);
        glEnd();


        glPushMatrix();
        glTranslatef(0.4, 0.7, 0);
        glColor4f(1, 1, 1, titletick);


        print_straight_text(_("enter name"));
        glTranslatef(0, -0.15, 0);

        glColor3f(0.3, 0.3, 0.3);

        glBegin(GL_QUADS);
        glVertex2f(-0.05, 0.025);
        glVertex2f(-0.05, -0.065);
        glVertex2f(0.7f, -0.065);
        glVertex2f(0.7f, 0.025);
        glEnd();

        glColor4f(1, 1, 1, titletick);

        print_straight_text(enteringname);



        glPopMatrix();

        glTranslatef(-0.7, 0, 0);
        if (tso == ts_editgameselect)
        {
            glTranslatef(-0.31, 0, 0);

            glColor3f(1, 1, 1);
            glBegin(GL_LINES);
            glVertex2f(0.3, 1);
            glVertex2f(0.3, -2);
            glEnd();


            glPushMatrix();
            glTranslatef(0.4, 0.7, 0);
            glColor3f(0.5, 0.5, 0.5);

            print_straight_text (_("enter name"));

            glPopMatrix();

            glTranslatef(-0.7, 0, 0);
        }

        glTranslatef(-0.31, 0, 0);

        glColor3f(1, 1, 1);
        glBegin(GL_LINES);
        glVertex2f(0.3, 1);
        glVertex2f(0.3, -2);
        glEnd();


        glPushMatrix();
        glTranslatef(0.4, 0.7, 0);
        if (tso == ts_editgameselect)
        {
            glColor4f(1, 1, 1, titletick);
        }
        else
        {
            glColor3f(0.5, 0.5, 0.5);
        }


        print_straight_text(_("new chapter"));


        glPushMatrix();
        glTranslatef(0.05, 0, 0);

        glTranslatef(0, -0.15, 0);
        glColor3f(1, 1, 1);


        print_straight_text(_("--new--"));

        glPopMatrix();

        glPopMatrix();



        glTranslatef(-1, 0, 0);
    }

    SDL_GL_SwapBuffers();
}



void draw_editmode(bool resetright, bool resetleftbounds)
{
    static int currange2_begin = 0;
    static int currange2_end = 9;

    static int currange3_begin = 0;
    static int currange3_end = 10;



    if (resetlistbounds)
    {
        if (episodeList.size() > 0)
            curtitleselection2 = 0;  //erm; what if there are no lists?!?! should set this to -1 then
        else
            curtitleselection2 = -1;

        curtitleselection3 = 0;
        currange2_begin = 0;

        currange2_end = std::min(std::max(curtitleselection2, 0) + 9, (int) customEpisodeList.size());
    }

    if (resetlistbounds || resetright)
    {
        afterselection3 = false;
        beforeselection3 = true;
        curtitleselection3 = 0;
        currange3_begin = 0;

        currange3_end = min(10, customEpisodeList[max(curtitleselection2, 0)].num_episodes);
    }



    {
        if (curtitleselection2 < currange2_begin && curtitleselection2 >= 0)
        {
            currange2_begin = std::max(curtitleselection2, 0);
            currange2_end = std::min(std::max(curtitleselection2, 0) + 9, (int)customEpisodeList.size());
        }
        else if (curtitleselection2 >= currange2_end)
        {
            currange2_begin = std::max(std::max(curtitleselection2, 0) - 8, 0);
            currange2_end = std::min(std::max(std::max(curtitleselection2, 0) - 8, 0) + 9, (int) customEpisodeList.size());
        }


        ///need to be careful about these I think...reset frequently...

        if (curtitleselection2 >= 0)
        {
            //blah ok:
            if ((currange3_end > customEpisodeList[curtitleselection2].num_episodes) || ((currange3_end - currange3_begin < 9) && (currange3_end < customEpisodeList[curtitleselection2].num_episodes)))
            {
                curtitleselection3 = 0;
                currange3_begin = 0;
                currange3_end = min(10, customEpisodeList[curtitleselection2].num_episodes);
            }


            //end blah
            if (curtitleselection3 < currange3_begin)
            {
                currange3_begin = curtitleselection3;
                currange3_end = min(curtitleselection3 + 10, customEpisodeList[curtitleselection2].num_episodes);
            }
            else if (curtitleselection3 >= currange3_end)
            {
                currange3_begin = max(curtitleselection3 - 9, 0);
                currange3_end = min(max(curtitleselection3 - 9, 0) + 10, customEpisodeList[curtitleselection2].num_episodes);
            }
        }
    }

    if (resetleftbounds)
    {
        if (currange2_begin == 0)
        {
            currange2_end = std::min(std::max(std::max(curtitleselection2, 0) - 8, 0) + 9, (int)customEpisodeList.size());
        }
        resetleftbounds = false;
    }



    resetlistbounds = false;



    if (titletick < 1)
        titletick += 0.01;

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();



    switch (tso)
    {
    case ts_editentername:
    case ts_editchapterselect:

        glTranslatef(-0.31, 0, 0);

        glColor3f(1, 1, 1);
        glBegin(GL_LINES);
        glVertex2f(0.3, 1);
        glVertex2f(0.3, -2);
        glEnd();


        glPushMatrix();
        glTranslatef(0.4, 0.7, 0);
        glColor4f(1, 1, 1, titletick);

        if (curtitleselection2 >= 0)
        {
            if (saveasmode)
                print_straight_text(_("save episode as"));
            else
                print_straight_text(_("select episode"));
        }
        else
        {
            print_straight_text(_("enter name"));
            glTranslatef(0, -0.15, 0);

            glColor3f(0.3, 0.3, 0.3);

            glBegin(GL_QUADS);
            glVertex2f(-0.05, 0.025);
            glVertex2f(-0.05, -0.065);
            glVertex2f(0.7f, -0.065);
            glVertex2f(0.7f, 0.025);
            glEnd();

            glColor4f(1, 1, 1, titletick);

            print_straight_text(enteringname);
        }

        if (curtitleselection2 >= 0)
        {
            glPushMatrix();

            glTranslatef(0.05, 0, 0);
            //make the 10 a min (chaptercount,10)

            int loopmax = currange3_end;
            int loopmin = currange3_begin;

            if (afterselection3 && ((curtitleselection3 == customEpisodeList[curtitleselection2].num_episodes - 1) || (curtitleselection3 == currange3_end - 1)))
            {
                if (currange3_end - currange3_begin >= 10)
                {
                    loopmin++;
                }
            }
            else if (afterselection3 || beforeselection3)
            {
                if (currange3_end - currange3_begin >= 10)
                {
                    loopmax--;
                }
            }

            for (int i = loopmin; i < loopmax; i++)
            {
                glTranslatef(0, -0.15, 0);


                if (i == 0 && beforeselection3)
                {
                    glPushMatrix();

                    glTranslatef(0.05f, 0.0f, 0.0f);

                    glColor3f(0.3f, 0.3f, 0.3f);

                    glBegin(GL_QUADS);
                    glVertex2f(-0.05f, 0.025f);
                    glVertex2f(-0.05f, -0.065f);
                    glVertex2f(0.7f, -0.065f);
                    glVertex2f(0.7f, 0.025f);
                    glEnd();

                    glColor3f(1.0f, 1.0f, 1.0f);

                    print_straight_text(_("insert new"));

                    glPopMatrix();
                    glTranslatef(0.0f, -0.15f, 0.0f);
                }

                if (i == curtitleselection3 && (!afterselection3) && (!beforeselection3))
                    glColor3f(0.3f, 0.3f, 0.3f);
                else
                    glColor3f(0.1f, 0.1f, 0.1f);

                glBegin(GL_QUADS);
                glVertex2f(-0.05f, 0.025f);
                glVertex2f(-0.05f, -0.065f);
                glVertex2f(0.7f, -0.065f);
                glVertex2f(0.7f, 0.025f);
                glEnd();
                if (i == curtitleselection3 && (!afterselection3) && (!beforeselection3))
                    glColor3f(1.0f, 1.0f, 1.0f);
                else
                    glColor3f(0.5f, 0.5f, 0.5f);

                //13 CHARACTER LIMIT ON CHAPTER NAMES
                print_straight_text( _("episode ") + stringify(i));

                if (i == curtitleselection3 && afterselection3)
                {
                    glTranslatef(0.0f, -0.15f, 0.0f);

                    glPushMatrix();

                    glTranslatef(0.05f, 0.0f, 0.0f);

                    glColor3f(0.3f, 0.3f, 0.3f);

                    glBegin(GL_QUADS);
                    glVertex2f(-0.05f, 0.025f);
                    glVertex2f(-0.05f, -0.065f);
                    glVertex2f(0.7f, -0.065f);
                    glVertex2f(0.7f, 0.025f);
                    glEnd();

                    glColor3f(1.0f, 1.0f, 1.0f);

                    print_straight_text(_("insert new"));

                    glPopMatrix();
                }
            }
            glPopMatrix();
        }

        glPopMatrix();

        glTranslatef(-0.7f, 0.0f, 0.0f);

    case ts_editgameselect:
        if (tso == ts_editgameselect)
        {
            glTranslatef(-0.31f, 0.0f, 0.0f);

            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_LINES);
            glVertex2f(0.3f, 1.0f);
            glVertex2f(0.3f, -2.0f);
            glEnd();


            glPushMatrix();
            glTranslatef(0.4f, 0.7f, 0.0f);
            glColor3f(0.5f, 0.5f, 0.5f);
            if (curtitleselection2 >= 0)
            {
                if (saveasmode)
                    print_straight_text(_("save episode as"));
                else
                    print_straight_text(_("select episode"));
            }
            else
                print_straight_text(_("enter name"));

            glPushMatrix();

            glTranslatef(0.05f, 0.0f, 0.0f);
            //make the 10 a min (chaptercount,10)
            if (curtitleselection2 >= 0)
            {
                for (int i = 0; i < min(10, customEpisodeList[curtitleselection2].num_episodes); i++)
                {
                    glTranslatef(0.0f, -0.15f, 0.0f);

                    glColor3f(0.1f, 0.1f, 0.1f);

                    glBegin(GL_QUADS);
                    glVertex2f(-0.05f, 0.025f);
                    glVertex2f(-0.05f, -0.065f);
                    glVertex2f(0.7f, -0.065f);
                    glVertex2f(0.7f, 0.025f);
                    glEnd();
                    glColor3f(0.5f, 0.5f, 0.5f);

                    //13 CHARACTER LIMIT ON CHAPTER NAMES
                    print_straight_text (_("episode ") + stringify(i));
                }
            }


            glPopMatrix();



            glPopMatrix();

            glTranslatef(-0.7f, 0.0f, 0.0f);
        }

        glTranslatef(-0.31f, 0.0f, 0.0f);

        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(0.3f, 1.0f);
        glVertex2f(0.3f, -2.0f);
        glEnd();


        glPushMatrix();
        glTranslatef(0.4f, 0.7f, 0.0f);
        if (tso == ts_editgameselect)
        {
            glColor4f(1.0f, 1.0f, 1.0f, titletick);
        }
        else
        {
            glColor3f(0.5f, 0.5f, 0.5f);
        }
        if (curtitleselection2 >= 0)
            print_straight_text(_("edit chapter"));
        else
            print_straight_text(_("new chapter"));


        glPushMatrix();
        glTranslatef(0.05f, 0.0f, 0.0f);

        glTranslatef(0.0f, -0.15f, 0.0f);
        if (curtitleselection2 == -1)
            glColor3f(1, 1, 1);
        else
            glColor3f(0.3, 0.3, 0.3);

        print_straight_text(_("--new--"));

        //make the 10 a min (chaptercount,10)
        for (int i = currange2_begin; i < currange2_end; i++)
        {
            glTranslatef(0, -0.15, 0);

            if (i == curtitleselection2)
                glColor3f(0.3, 0.3, 0.3);
            else
                glColor3f(0.1, 0.1, 0.1);

            glBegin(GL_QUADS);
            glVertex2f(-0.05, 0.025);
            glVertex2f(-0.05, -0.065);
            glVertex2f(0.7f, -0.065);
            glVertex2f(0.7f, 0.025);
            glEnd();
            if (i == curtitleselection2)
                glColor3f(1, 1, 1);
            else
                glColor3f(0.5, 0.5, 0.5);


            string s;


            s = customEpisodeList[i].name;


            s.resize(13);

            //13 CHARACTER LIMIT ON CHAPTER NAMES
            print_straight_text(s);
        }
        glPopMatrix();

        glPopMatrix();



        glTranslatef(-1.0f, 0.0f, 0.0f);
        break;
    default:
        break;
    }

    SDL_GL_SwapBuffers();
}



void draw_title()
{
    static int currange2_begin = 0;
    static int currange2_end = 10;


    static int currange3_begin = 0;
    static int currange3_end = 10;

    if (resetlistbounds)
    {
        if ((tso == ts_newgamechapterselect) || (tso == ts_newgameselect))
        {
            curtitleselection2 = 0;
            curtitleselection3 = 0;
            currange2_begin = 0;
            currange2_end = std::min(curtitleselection2 + 10, (int)episodeList.size());
            currange3_begin = 0;
            currange3_end = std::min(10, episodeList[curtitleselection2].num_episodes);
        }
        else
        {
            curtitleselection2 = 0;
            curtitleselection3 = 0;
            currange2_begin = 0;
            currange2_end = std::min(curtitleselection2 + 10, (int) customEpisodeList.size());
            currange3_begin = 0;
            currange3_end = std::min(10, customEpisodeList[curtitleselection2].num_episodes);
        }
    }
    resetlistbounds = false;


    if ((tso == ts_newgamechapterselect) || (tso == ts_newgameselect))
    {
        if (curtitleselection2 < currange2_begin)
        {
            currange2_begin = curtitleselection2;
            currange2_end = std::min(curtitleselection2 + 10, (int)episodeList.size());
        }
        else if (curtitleselection2 >= currange2_end)
        {
            currange2_begin = max(curtitleselection2 - 9, 0);
            currange2_end = std::min(std::max(curtitleselection2 - 9, 0) + 10,(int) episodeList.size());
        }

        ///need to be careful about these I think...reset frequently...

        //blah ok:
        if ((currange3_end > episodeList[curtitleselection2].num_episodes) || ((currange3_end - currange3_begin < 10) && (currange3_end < episodeList[curtitleselection2].num_episodes)))
        {
            curtitleselection3 = 0;
            currange3_begin = 0;
            currange3_end = min(10, episodeList[curtitleselection2].num_episodes);
        }


        //end blah
        if (curtitleselection3 < currange3_begin)
        {
            currange3_begin = curtitleselection3;
            currange3_end = min(curtitleselection3 + 10, episodeList[curtitleselection2].num_episodes);
        }
        else if (curtitleselection3 >= currange3_end)
        {
            currange3_begin = max(curtitleselection3 - 9, 0);
            currange3_end = min(max(curtitleselection3 - 9, 0) + 10, episodeList[curtitleselection2].num_episodes);
        }
    }
    else if ((tso == ts_newcustomgamechapterselect) || (tso == ts_newcustomgameselect))
    {
        if (curtitleselection2 < currange2_begin)
        {
            currange2_begin = curtitleselection2;
            currange2_end = std::min(curtitleselection2 + 10, (int) customEpisodeList.size());
        }
        else if (curtitleselection2 >= currange2_end)
        {
            currange2_begin = std::max(curtitleselection2 - 9, 0);
            currange2_end = std::min(std::max(curtitleselection2 - 9, 0) + 10, (int)customEpisodeList.size());
        }


        ///need to be careful about these I think...reset frequently...

        //blah ok:
        if ((currange3_end > customEpisodeList[curtitleselection2].num_episodes) || ((currange3_end - currange3_begin < 10) && (currange3_end < customEpisodeList[curtitleselection2].num_episodes)))
        {
            curtitleselection3 = 0;
            currange3_begin = 0;
            currange3_end = min(10, customEpisodeList[curtitleselection2].num_episodes);
        }


        //end blah
        if (curtitleselection3 < currange3_begin)
        {
            currange3_begin = curtitleselection3;
            currange3_end = min(curtitleselection3 + 10, customEpisodeList[curtitleselection2].num_episodes);
        }
        else if (curtitleselection3 >= currange3_end)
        {
            currange3_begin = max(curtitleselection3 - 9, 0);
            currange3_end = min(max(curtitleselection3 - 9, 0) + 10, customEpisodeList[curtitleselection2].num_episodes);
        }
    }


    if (titletick < 1)
        titletick += 0.01;

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();



    switch (tso)
    {
    case ts_newgamechapterselect:
    case ts_newcustomgamechapterselect:
        glTranslatef(-0.31f, 0.0f, 0.0f);

        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(0.3f, 1.0f);
        glVertex2f(0.3f, -2.0f);
        glEnd();


        glPushMatrix();
        glTranslatef(0.4f, 0.7f, 0.0f);
        glColor4f(1.0f, 1.0f, 1.0f, titletick);
        print_straight_text(_("select episode"));

        glPushMatrix();

        glTranslatef(0.05f, 0.0f, 0.0f);
        //make the 10 a min (chaptercount,10)
        for (int i = currange3_begin; i < currange3_end; i++)
        {
            glTranslatef(0.0f, -0.15f, 0.0f);

            if (i == curtitleselection3)
                glColor3f(0.3f, 0.3f, 0.3f);
            else if (tso == ts_newgamechapterselect && !savedata.canplay[curtitleselection2][i])
            {
                glColor3f(0.0f, 0.0f, 0.0f);
            }
            else
            {
                glColor3f(0.1f, 0.1f, 0.1f);
            }

            glBegin(GL_QUADS);
            glVertex2f(-0.05f, 0.025f);
            glVertex2f(-0.05f, -0.065f);
            glVertex2f(0.7f, -0.065f);
            glVertex2f(0.7f, 0.025f);
            glEnd();
            if (i == curtitleselection3)
                glColor3f(1.0f, 1.0f, 1.0f);
            else if (tso == ts_newgamechapterselect && savedata.completed[curtitleselection2][i])
                glColor3f(0.5f, 0.5f, 0.5f);
            else if (tso == ts_newgamechapterselect && savedata.canplay[curtitleselection2][i])
                glColor3f(1.0f, 1.0f, 1.0f);
            else
                glColor3f(0.5f, 0.5f, 0.5f);

            //13 CHARACTER LIMIT ON CHAPTER NAMES
            print_straight_text(_("episode ") + stringify(i));
        }
        glPopMatrix();



        glPopMatrix();

        glTranslatef(-0.7f, 0.0f, 0.0f);

    case ts_newgameselect:
    case ts_newcustomgameselect:
        if ((tso == ts_newgameselect) || (tso == ts_newcustomgameselect))
        {
            glTranslatef(-0.31f, 0.0f, 0.0f);

            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_LINES);
            glVertex2f(0.3f, 1.0f);
            glVertex2f(0.3f, -2.0f);
            glEnd();


            glPushMatrix();
            glTranslatef(0.4f, 0.7f, 0.0f);
            glColor3f(0.5f, 0.5f, 0.5f);
            print_straight_text(_("select episode"));

            glPushMatrix();

            glTranslatef(0.05f, 0.0f, 0.0f);
            //make the 10 a min (chaptercount,10)
            if (tso == ts_newgameselect)
            {
                for (int i = 0; i < min(10, episodeList[curtitleselection2].num_episodes); i++)
                {
                    glTranslatef(0.0f, -0.15f, 0.0f);


                    if (!savedata.canplay[curtitleselection2][i])
                    {
                        glColor3f(0.0f, 0.0f, 0.0f);
                    }
                    else
                    {
                        glColor3f(0.1f, 0.1f, 0.1f);
                    }



                    glBegin(GL_QUADS);
                    glVertex2f(-0.05f, 0.025f);
                    glVertex2f(-0.05f, -0.065f);
                    glVertex2f(0.7f, -0.065f);
                    glVertex2f(0.7f, 0.025f);
                    glEnd();

                    if (savedata.completed[curtitleselection2][i])
                        glColor3f(0.5f, 0.5f, 0.5f);
                    else if (savedata.canplay[curtitleselection2][i])
                        glColor3f(1.0f, 1.0f, 1.0f);
                    else
                        glColor3f(0.5f, 0.5f, 0.5f);

                    //13 CHARACTER LIMIT ON CHAPTER NAMES
                    print_straight_text(_("episode ") + stringify(i));
                }
            }
            else
            {
                for (int i = 0; i < min(10, customEpisodeList[curtitleselection2].num_episodes); i++)
                {
                    glTranslatef(0.0f, -0.15f, 0.0f);

                    glColor3f(0.1f, 0.1f, 0.1f);

                    glBegin(GL_QUADS);
                    glVertex2f(-0.05f, 0.025f);
                    glVertex2f(-0.05f, -0.065f);
                    glVertex2f(0.7f, -0.065f);
                    glVertex2f(0.7f, 0.025f);
                    glEnd();
                    glColor3f(0.5f, 0.5f, 0.5f);

                    //13 CHARACTER LIMIT ON CHAPTER NAMES
                    print_straight_text(_("episode ") + stringify(i));
                }
            }


            glPopMatrix();



            glPopMatrix();

            glTranslatef(-0.7f, 0.0f, 0.0f);
        }

        glTranslatef(-0.31f, 0.0f, 0.0f);

        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(0.3f, 1.0f);
        glVertex2f(0.3f, -2.0f);
        glEnd();


        glPushMatrix();
        glTranslatef(0.4f, 0.7f, 0.0f);
        if ((tso == ts_newgameselect) || (tso == ts_newcustomgameselect))
        {
            glColor4f(1.0f, 1.0f, 1.0f, titletick);
        }
        else
        {
            glColor3f(0.5f, 0.5f, 0.5f);
        }
        print_straight_text(_("select chapter"));


        glPushMatrix();

        glTranslatef(0.05f, 0.0f, 0.0f);
        //make the 10 a min (chaptercount,10)
        for (int i = currange2_begin; i < currange2_end; i++)
        {
            glTranslatef(0.0f, -0.15f, 0.0f);

            if (i == curtitleselection2)
                glColor3f(0.3f, 0.3f, 0.3f);
            else if (tso == ts_newcustomgameselect || tso == ts_newcustomgamechapterselect || savedata.canplay[i][0])
                glColor3f(0.1f, 0.1f, 0.1f);
            else
                glColor3f(0.0f, 0.0f, 0.0f);

            glBegin(GL_QUADS);
            glVertex2f(-0.05f, 0.025f);
            glVertex2f(-0.05f, -0.065f);
            glVertex2f(0.7f, -0.065f);
            glVertex2f(0.7f, 0.025f);
            glEnd();
            if (i == curtitleselection2)
                glColor3f(1.0f, 1.0f, 1.0f);
            else
                glColor3f(0.5f, 0.5f, 0.5f);


            string s;
            if (tso == ts_newgameselect || tso == ts_newgamechapterselect)
            {
                s = episodeList[i].name;
            }
            else
            {
                s = customEpisodeList[i].name;
            }

            s.resize(13);

            //13 CHARACTER LIMIT ON CHAPTER NAMES
            print_straight_text(_(s.c_str()));
        }
        glPopMatrix();

        glPopMatrix();



        glTranslatef(-1.0f, 0.0f, 0.0f);
        break;

    case ts_main:
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(0.3f, 1.0f);
        glVertex2f(0.3f, -2.0f * titletick);
        glEnd();

        glPushMatrix();
        glTranslatef(-0.85f, 0.05f, 0.0f);
        glScalef(1.5f, 1.5f, 1.5f);
        glColor4f(1.0f, 1.0f, 1.0f, titletick);
        print_straight_text(_("mirror stage"));
        glPopMatrix();

        if (titletick >= 1)
        {
            glTranslatef(0.4f, 0.0f, 0.0f);
            glTranslatef(0.0f, -0.15f, 0.0f);
            glColor3f(0.5f, 0.5f, 0.5f);

            glPushMatrix();

            glTranslatef(0.0f, -0.15f * (curtitleselection1 + 1.0f), 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(-0.05f, 0.02f);
            glVertex2f(-0.05f, -0.08f);
            glVertex2f(0.5f, -0.08f);
            glVertex2f(0.5f, 0.02f);
            glEnd();

            glPopMatrix();

            glColor3f(1.0f, 1.0f, 1.0f);

            glTranslatef(0.0f, -0.15f, 0.0f);
            print_straight_text(_("play"));
            glTranslatef(0.0f, -0.15f, 0.0f);
            if (customEpisodeList.size() == 0)
                glColor3f(0.5f, 0.5f, 0.5f);
            print_straight_text(_("custom"));
            if (customEpisodeList.size() == 0)
                glColor3f(1.0f, 1.0f, 1.0f);
            glTranslatef(0.0f, -0.15f, 0.0f);
            print_straight_text(_("editor"));
            glTranslatef(0.0f, -0.15f, 0.0f);
            print_straight_text(_("quit"));
        }
        break;
    default:
        break;
    }

    SDL_GL_SwapBuffers();
}


int main(int argc, char *argv[])
{
    //Localising the text
    #ifdef USE_GETTEXT
    setlocale (LC_MESSAGES, "");
    setlocale (LC_CTYPE, "");
    setlocale (LC_COLLATE, "");
    textdomain ("mirrorstage");
    bindtextdomain ("mirrorstage", NULL);
    #endif

    //The frame rate regulator
    Timer fps;

    tso = ts_main;
    saveasmode = false;

    //Quit flag
    bool quit = false;

    //In	itialize
    if (init() == false)
    {
        return 1;
    }

    //Our square object
    Game game;

    //state chooser
l_choosestate:
    quit = false;
    wantquit = false;
    //curtitleselection1=0;
    switch (gamestate)
    {
    case gs_intro:
        glClearColor(0, 0, 0, 1);
        goto l_intro;
        break;

    case gs_title:
        glClearColor(0, 0, 0, 1);
        goto l_title;
        break;

    case gs_interlude:
        goto l_interlude;
        break;

    case gs_level:
        goto l_level;
        break;

    case gs_quit:
        goto l_quit;
        break;

    case gs_finishchapter:
        goto l_finishchapter;
        break;
    }

    //finish screen
l_finishchapter:
    while (quit == false)
    {
        fps.start();


        if (!draw_finishchapter())
        {
            quit = true;
            gamestate = gs_title;
        }

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYUP)
            {
                //Adjust the velocity
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                case SDLK_SPACE:
                case SDLK_RETURN:
                    quit = true;
                    gamestate = gs_title;
                    fadetoblack();
                    loadtrack(0);
                    break;

                default:
                    break;
                }
            }
            if (event.type == SDL_QUIT)
            {
                quit = true;
                gamestate = gs_quit;
            }
        }




        if (fps.get_ticks() < 1000 / FRAMES_PER_SECOND)
        {
            SDL_Delay((1000 / FRAMES_PER_SECOND) - fps.get_ticks());
        }
    }
    goto l_choosestate;


    //intro
l_intro:
    while (quit == false)
    {
        fps.start();

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYUP)
            {
                //Adjust the velocity
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                case SDLK_SPACE:
                    quit = true;
                    gamestate = gs_title;
                    break;

                default:
                    break;
                }
            }
            if (event.type == SDL_QUIT)
            {
                quit = true;
                gamestate = gs_quit;
            }
        }

        if (!draw_intro())
        {
            quit = true;
            gamestate = gs_title;
        }

        if (fps.get_ticks() < 1000 / FRAMES_PER_SECOND)
        {
            SDL_Delay((1000 / FRAMES_PER_SECOND) - fps.get_ticks());
        }
    }
    goto l_choosestate;

    //title
l_title:
    while (quit == false)
    {
        fps.start();
        bool resetright = false;
        bool resetleftbounds = false;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_KEYUP)
            {
                //Adjust the velocity
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                case SDLK_LEFT:
                {
                    switch (tso)
                    {
                    case ts_main:
                        if (event.key.keysym.sym == SDLK_ESCAPE)
                        {
                            quit = true;
                            gamestate = gs_quit;
                            playsound(0);
                        }
                        break;

                    case ts_newgameselect:
                    case ts_newcustomgameselect:
                        tso = ts_main;
                        resetlistbounds = true;
                        playsound(0);
                        break;

                    case ts_newgamechapterselect:
                        tso = ts_newgameselect;
                        playsound(0);
                        break;

                    case ts_newcustomgamechapterselect:
                        tso = ts_newcustomgameselect;
                        playsound(0);
                        break;

                    case ts_editgameselect:
                    {
                        if (saveasmode)
                        {
                            if (event.key.keysym.sym == SDLK_ESCAPE)
                            {
                                curtitleselection2 = curtitleselection2old;
                                curtitleselection3 = curtitleselection3old;
                                afterselection3 = false;
                                beforeselection3 = false;
                                quit = true;
                                gamestate = gs_level;
                                saveasmode = false;
                                playsound(0);
                            }
                        }
                        else
                        {
                            playsound(0);
                            tso = ts_main;
                        }
                    }
                    break;

                    case ts_editentername:
                        tso = ts_editgameselect;
                        playsound(0);
                        break;

                    case ts_editchapterselect:
                        tso = ts_editgameselect;
                        playsound(0);
                        break;

                    case ts_editnofiles:
                        tso = ts_main;
                        playsound(0);
                        break;
                    case ts_nofiles:
                        //SHOULD NEVER GET HERE
                        break;
                    }
                }
                break;

                case SDLK_SPACE:
                case SDLK_RIGHT:
                case SDLK_RETURN:
                    if (ignorenextspaceup)
                    {
                        ignorenextspaceup = false;
                    }
                    else if (tso == ts_main)
                    {
                        if (titletick < 1)
                        {
                            titletick = 1;
                        }
                        else
                        {
                            switch (curtitleselection1)
                            {
                            case 0:
                                curtitleselection2 = 0;
                                tso = ts_newgameselect;
                                resetlistbounds = true;
                                playsound(0);
                                break;

                            case 1:
                                if (customEpisodeList.size() > 0)
                                {
                                    curtitleselection2 = 0;
                                    tso = ts_newcustomgameselect;
                                    resetlistbounds = true;
                                    playsound(0);
                                }
                                else
                                {
                                    tso = ts_nofiles;
                                    playsound(0);
                                }
                                break;

                            case 2:
                                if (customEpisodeList.size() > 0)
                                {
                                    curtitleselection2 = 0;
                                    tso = ts_editgameselect;
                                    resetlistbounds = true;
                                    playsound(0);
                                }
                                else
                                {
                                    tso = ts_editnofiles;
                                    enteringname = "";
                                    playsound(0);
                                }
                                break;

                            case 3:
                                if (event.key.keysym.sym == SDLK_SPACE)
                                {
                                    gamestate = gs_quit;
                                    quit = true;
                                    playsound(0);
                                }
                                break;

                            default:
                                break;
                            }
                        }
                    }
                    else if (tso == ts_newgameselect)
                    {
                        tso = ts_newgamechapterselect;
                        curtitleselection3 = 0;
                        playsound(0);
                    }
                    else if (tso == ts_newcustomgameselect)
                    {
                        tso = ts_newcustomgamechapterselect;
                        curtitleselection3 = 0;
                        playsound(0);
                    }
                    else if (tso == ts_newgamechapterselect)
                    {
                        if (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN)
                        {
                            curlevel = curtitleselection3;
                            curchapter = episodeList[curtitleselection2].name;
                            customchapter = false;
                            episodecount = episodeList[curtitleselection2].num_episodes;
                            editorenabled = false;
                            editmode = false;
                            fadetoblack();
                            initlevel();
                            quit = true;
                        }
                    }
                    else if (tso == ts_newcustomgamechapterselect)
                    {
                        if (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN)
                        {
                            {
                                curlevel = curtitleselection3;
                                curchapter = customEpisodeList[curtitleselection2].name;
                                customchapter = true;
                                episodecount = customEpisodeList[curtitleselection2].num_episodes;
                                editorenabled = false;
                                editmode = false;
                                editor.page = E_FileStuff;
                                fadetoblack();
                                initlevel();
                                quit = true;
                            }
                        }
                    }
                    else if (tso == ts_editchapterselect)
                    {
                        if (event.key.keysym.sym == SDLK_SPACE || event.key.keysym.sym == SDLK_RETURN)
                        {
                            if (ignorenextspaceup)
                            {
                                ignorenextspaceup = false;
                            }
                            else
                            {
                                //could tidy the following up a little
                                if (!saveasmode)
                                    blankLevel(level);

                                if (beforeselection3)
                                {
                                    curchapter = customEpisodeList[curtitleselection2].name;

                                    //move all the levels up

                                    for (int i = customEpisodeList[curtitleselection2].num_episodes - 1; i >= 0; i--)
                                    {
                                        string s = "custom/" + curchapter + "/" + stringify(i) + ".dat";
                                        string t = "custom/" + curchapter + "/" + stringify(i + 1) + ".dat";
                                        renameFile(s, t);
                                    }

                                    string s = "custom/" + curchapter;
                                    bf::create_directory(s);
                                    s += "/0.dat";
                                    writelevel(s.c_str(), level);
                                    getEpisodeData();
                                    quit = true;


                                    curlevel = 0;
                                    customchapter = true;
                                    episodecount = customEpisodeList[curtitleselection2].num_episodes;
                                    editorenabled = true;
                                    if (!saveasmode)
                                    {
                                        fadetoblack();
                                    }
                                    initlevel();
                                    resetlistbounds = true;
                                    saveasmode = false;
                                    editmode = true;
                                }
                                else if (afterselection3)
                                {
                                    curchapter = customEpisodeList[curtitleselection2].name;

                                    //move all the levels up

                                    for (int i = customEpisodeList[curtitleselection2].num_episodes - 1; i > curtitleselection3; i--)
                                    {
                                        string s = "custom/" + curchapter + "/" + stringify(i) + ".dat";
                                        string t = "custom/" + curchapter + "/" + stringify(i + 1) + ".dat";
                                        renameFile(s, t);
                                    }

                                    string s = "custom/" + curchapter + "/" + stringify(curtitleselection3 + 1) + ".dat";
                                    writelevel(s.c_str(), level);
                                    getEpisodeData();

                                    curlevel = curtitleselection3 + 1;
                                    customchapter = true;
                                    episodecount = customEpisodeList[curtitleselection2].num_episodes;
                                    editmode = true;
                                    editorenabled = true;
                                    if (!saveasmode)
                                    {
                                        fadetoblack();
                                    }
                                    initlevel();
                                    quit = true;
                                    wantquit = true;
                                    gamestate = gs_level;
                                    resetlistbounds = true;
                                    saveasmode = false;
                                }
                                else
                                {
                                    curlevel = curtitleselection3;
                                    curchapter = customEpisodeList[curtitleselection2].name;
                                    customchapter = true;
                                    episodecount = customEpisodeList[curtitleselection2].num_episodes;
                                    editmode = true;
                                    editorenabled = true;

                                    if (saveasmode)
                                    {
                                        string s = "custom/" + curchapter + "/" + stringify(curtitleselection3) + ".dat";
                                        writelevel(s.c_str(), level);
                                        getEpisodeData();
                                    }

                                    if (!saveasmode)
                                    {
                                        fadetoblack();
                                    }
                                    initlevel();
                                    quit = true;
                                    wantquit = true;
                                    gamestate = gs_level;
                                    resetlistbounds = true;
                                    saveasmode = false;
                                }
                            }
                        }
                    }
                    else if (tso == ts_editgameselect)
                    {
                        if (curtitleselection2 >= 0)
                        {
                            tso = ts_editchapterselect;
                            resetright = true;
                            curtitleselection3 = 0;                                    //unnecessary?
                            beforeselection3 = true;
                            afterselection3 = false;
                        }
                        else
                        {
                            enteringname = "";
                            tso = ts_editentername;
                        }
                    }
                    break;

                case SDLK_UP:
                case SDLK_LEFTBRACKET:
                    playsound(0);
                    if (tso == ts_main)
                    {
                        if (customEpisodeList.size() == 0 && curtitleselection1 == 2)
                            curtitleselection1 = 0;
                        else
                            curtitleselection1 = (curtitleselection1 + 4 - 1) % 4;
                    }
                    else if (tso == ts_newgameselect)
                    {
                        if (savedata.canplay[(curtitleselection2 + episodeList.size() - 1) % episodeList.size()][0])
                            curtitleselection2 = (curtitleselection2 + episodeList.size() - 1) % episodeList.size();
                        else
                        {
                            int i;
                            for (i = episodeList.size() - 1; i >= 0; i--)
                            {
                                if (savedata.canplay[i][0])
                                    break;
                            }
                            curtitleselection2 = i;
                        }
                    }
                    else if (tso == ts_newgamechapterselect)
                    {
                        int candidate = (curtitleselection3 + episodeList[curtitleselection2].num_episodes - 1) % episodeList[curtitleselection2].num_episodes;
                        if (savedata.canplay[curtitleselection2][candidate])
                            curtitleselection3 = candidate;
                        else
                        {
                            int i;
                            for (i = episodes_in_chapter[curtitleselection2] - 1; i >= 0; i--)
                            {
                                if (savedata.canplay[curtitleselection2][i])
                                    break;
                            }
                            curtitleselection3 = i;
                        }
                        //curtitleselection3=(curtitleselection3+episodeList[curtitleselection2].num_episodes-1)%episodeList[curtitleselection2].num_episodes;
                    }
                    else if (tso == ts_newcustomgameselect)
                        curtitleselection2 = (curtitleselection2 + customEpisodeList.size() - 1) % customEpisodeList.size();
                    else if (tso == ts_newcustomgamechapterselect)
                        curtitleselection3 = (curtitleselection3 + customEpisodeList[curtitleselection2].num_episodes - 1) % customEpisodeList[curtitleselection2].num_episodes;
                    else if (tso == ts_editgameselect)
                    {
                        //needlessly complex
                        if (curtitleselection2 > 0)
                            curtitleselection2 = (curtitleselection2 + customEpisodeList.size() - 1) % customEpisodeList.size();
                        else if (curtitleselection2 == 0)
                            curtitleselection2 = -1;
                        else
                            curtitleselection2 = customEpisodeList.size() - 1;
                    }
                    else if (tso == ts_editchapterselect)
                    {
                        if (afterselection3)
                        {
                            afterselection3 = false;
                        }
                        else if (beforeselection3)
                        {
                            beforeselection3 = false;
                            curtitleselection3 = (curtitleselection3 + customEpisodeList[curtitleselection2].num_episodes - 1) % customEpisodeList[curtitleselection2].num_episodes;
                            afterselection3 = true;
                        }
                        else if (curtitleselection3 == 0)
                        {
                            beforeselection3 = true;
                        }
                        else
                        {
                            curtitleselection3 = (curtitleselection3 + customEpisodeList[curtitleselection2].num_episodes - 1) % customEpisodeList[curtitleselection2].num_episodes;
                            afterselection3 = true;
                        }
                    }
                    break;

                case SDLK_DOWN:
                case SDLK_RIGHTBRACKET:
                    playsound(0);
                    if (tso == ts_main)
                    {
                        if (customEpisodeList.size() == 0 && curtitleselection1 == 0)
                            curtitleselection1 = 2;
                        else
                            curtitleselection1 = (curtitleselection1 + 1) % 4;
                    }
                    else if (tso == ts_newgameselect)
                    {
                        if (savedata.canplay[(curtitleselection2 + 1) % episodeList.size()][0])
                            curtitleselection2 = (curtitleselection2 + 1) % episodeList.size();
                        else (curtitleselection2 = 0);
                    }
                    else if (tso == ts_newgamechapterselect)
                    {
                        if (savedata.canplay[curtitleselection2][(curtitleselection3 + 1) % episodeList[curtitleselection2].num_episodes])
                            curtitleselection3 = (curtitleselection3 + 1) % episodeList[curtitleselection2].num_episodes;
                        else
                            curtitleselection3 = 0;
                    }
                    else if (tso == ts_newcustomgameselect)
                        curtitleselection2 = (curtitleselection2 + 1) % customEpisodeList.size();
                    else if (tso == ts_newcustomgamechapterselect)
                        curtitleselection3 = (curtitleselection3 + 1) % customEpisodeList[curtitleselection2].num_episodes;
                    else if (tso == ts_editgameselect)
                        curtitleselection2 = (curtitleselection2 + 1) % customEpisodeList.size();
                    else if (tso == ts_editchapterselect)
                    {
                        if (beforeselection3)
                        {
                            beforeselection3 = false;
                        }
                        else if (afterselection3)
                        {
                            curtitleselection3 = (curtitleselection3 + 1) % customEpisodeList[curtitleselection2].num_episodes;
                            afterselection3 = false;
                            if (curtitleselection3 == 0)
                                beforeselection3 = true;
                        }
                        else
                            afterselection3 = true;
                    }
                    break;

                default:
                    break;
                }
            }
            else if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                    if (tso == ts_editentername || tso == ts_editnofiles)
                    {
                        enteringname = enteringname.substr(0, curchapter.length() - 1);
                    }
                }
                else if (event.key.keysym.sym == SDLK_RETURN)
                {
                    if ((tso == ts_editentername || tso == ts_editnofiles) && (enteringname != ""))
                    {
                        string s;
                        curchapter = enteringname;
                        s = "custom/" + curchapter;

                        //need to check if it exists already
                        if (!bf::exists(s))
                        {
                            bf::create_directory(s);
                            if (!saveasmode)
                                blankLevel(level);
                            s += "/0.dat";
                            writelevel(s.c_str(), level);
                            getEpisodeData();
                        }
                        tso = ts_editgameselect;

                        // set curtitleselection2 to point to this
                        for (unsigned int i = 0; i < customEpisodeList.size(); i++)
                        {
                            if (curchapter == customEpisodeList[i].name)
                            {
                                curtitleselection2 = i;
                                break;
                            }
                        }
                        //maybe next should change to ts_editgameepisodeselect and point at first episode?( as opposed to insert new)
//						tso=ts_editchapterselect;

                        resetleftbounds = true;
                        if (saveasmode)
                        {
                            curlevel = 0;
                            curchapter = customEpisodeList[curtitleselection2].name;
                            gamestate = gs_level;
                            editmode = true;
                            editorenabled = true;
                            wantquit = true;
                            quit = true;
                            resetlistbounds = true;
                            initlevel();
                            saveasmode = false;
                        }
                        curtitleselection3 = 0;
                        beforeselection3 = false;
                        afterselection3 = false;
                    }
                }
                else if (event.key.keysym.unicode < 0x80 && event.key.keysym.unicode > 0)
                {
                    if (tso == ts_editentername || (tso == ts_editnofiles))
                    {
                        char ch = (char)event.key.keysym.unicode;
                        enteringname.append(1, tolower(ch));
                        if (enteringname.length() > 13)
                            enteringname.resize(13);
                    }
                }
            }

            if (event.type == SDL_QUIT)
            {
                quit = true;
                gamestate = gs_quit;
            }
        }

        if (!quit)          //don't draw if changing state...get flickering otherwise, because there's a pause to load the level
        {
            if (tso == ts_editgameselect || tso == ts_editentername || tso == ts_editchapterselect)
            {
                draw_editmode(resetright, resetleftbounds);
            }
            else if (tso == ts_editnofiles)
            {
                draw_editmodenochapter();
            }
            else
                draw_title();
        }

        if (fps.get_ticks() < 1000 / FRAMES_PER_SECOND)
        {
            SDL_Delay((1000 / FRAMES_PER_SECOND) - fps.get_ticks());
        }
    }
    goto l_choosestate;

l_level:
l_interlude:
    while (quit == false)
    {
        //Start the frame level.levelparams.timer
        fps.start();



        //While there are events to handle
        while (SDL_PollEvent(&event))
        {
            //Handle key presses
            game.handle_input();

            if (event.type == SDL_QUIT)
            {
                quit = true;
                gamestate = gs_quit;
            }
        }

        //Move the square
        game.move();



        //Clear the screen
        glClear(GL_COLOR_BUFFER_BIT);

        game.show();

        if (editmode)
            editor.overlay();

        SDL_GL_SwapBuffers();

        if (wantquit)
        {
            quit = true;
        }

        //Cap the frame rate
        if (fps.get_ticks() < 1000 / FRAMES_PER_SECOND)
        {
            SDL_Delay((1000 / FRAMES_PER_SECOND) - fps.get_ticks());
        }
    }
    goto l_choosestate;


l_quit:
    //Clean up
    fadetoblack();
    clean_up();

    return 0;
}
