/*
  This file contains the drawing and input routines for the editor.
*/

bool editmode;

void EditorState::traceRooms()
{
    for (int i = 0; i < ROOMCOUNT; i++)
    {
        component[i].connected = 0;
        component[i].indices = -1;
    }

    traceRoom(level.playerpos.room);

    indexFreeRooms();
}

void EditorState::traceRoom(int r)
{
    component[r].connected = 1;

    for (int i = 0; i < shapesize[level.rooms[r].shape]; i++)
    {
        if (level.rooms[r].portal[i])
        {
            if (!component[level.rooms[r].portaltoroom[i]].connected)
                traceRoom(level.rooms[r].portaltoroom[i]);
        }
    }
}

void EditorState::indexFreeRooms()
{
    int curindex = 0;

    for (int i = 0; i < ROOMCOUNT; i++)
    {
        if (!component[i].connected)
        {
            component[curindex].indices = i;
            curindex++;
        }
    }
}

void EditorState::removePortal(Portal p)
{
    if (level.rooms[p.room].portal[p.side] != 0)
    {
        level.rooms[p.room].portal[p.side] = 0;
        level.rooms[level.rooms[p.room].portaltoroom[p.side]].portal[level.rooms[p.room].portaltoedge[p.side]] = 0;

        countPortals();
        buildOutlines();
    }
}


void EditorState::addRoom(Portal pFrom, int shape, int shapeside, bool par, bool loc)
{
    traceRooms();

    if (component[0].indices == -1)
    {
        component[0].indices = ROOMCOUNT;
        resizethings(ROOMCOUNT + 10);
    }

    if (level.rooms[pFrom.room].portal[pFrom.side] == 0 && component[0].indices != -1)
    {
        int newroom = component[0].indices;

        for (int i = 0; i < shapesize[shape]; i++)
        {
            level.rooms[newroom].portal[i] = 0;
        }

        level.rooms[pFrom.room].portal[pFrom.side] = 1;
        level.rooms[pFrom.room].portalparity[pFrom.side] = par;
        level.rooms[pFrom.room].portaltoroom[pFrom.side] = newroom;
        level.rooms[pFrom.room].portaltoedge[pFrom.side] = shapeside;
        level.rooms[pFrom.room].localportal[pFrom.side] = loc;

        level.rooms[newroom].shape = shape;
        level.rooms[newroom].portal[shapeside] = 1;
        level.rooms[newroom].portalparity[shapeside] = par;
        level.rooms[newroom].portaltoroom[shapeside] = pFrom.room;
        level.rooms[newroom].portaltoedge[shapeside] = pFrom.side;
        level.rooms[newroom].localportal[shapeside] = loc;
        level.entitydata.curEntityPositions[newroom].type = e_EmptyRoom;

        buildRoomRatios();
        countPortals();
        buildOutlines();
    }
}

void EditorState::addPortal(Portal pFrom, Portal pTo, bool par, bool loc)
{
    if (
        level.rooms[pFrom.room].portal[pFrom.side] == 0 &&
        level.rooms[pTo.room].portal[pTo.side] == 0)
    {
        level.rooms[pFrom.room].portal[pFrom.side] = 1;
        level.rooms[pFrom.room].portalparity[pFrom.side] = par;
        level.rooms[pFrom.room].portaltoroom[pFrom.side] = pTo.room;
        level.rooms[pFrom.room].portaltoedge[pFrom.side] = pTo.side;
        level.rooms[pFrom.room].localportal[pFrom.side] = loc;

        level.rooms[pTo.room].portal[pTo.side] = 1;
        level.rooms[pTo.room].portalparity[pTo.side] = par;
        level.rooms[pTo.room].portaltoroom[pTo.side] = pFrom.room;
        level.rooms[pTo.room].portaltoedge[pTo.side] = pFrom.side;
        level.rooms[pTo.room].localportal[pTo.side] = loc;

        buildRoomRatios();
        countPortals();
        buildOutlines();
    }
}

//////
//  editor display stff
/////

static const GLfloat colourbox_height = 0.1;
static const GLfloat colourbox_padding = 0.03;

void EditorState::setPage(PageView newpage)
{
    if (page != newpage)
    {
        itemselected = 0;
        page = newpage;
    }
}


void EditorState::overlay()
{
    glPushMatrix();
    glLoadIdentity();

    glTranslatef(-1, 1, 0);

    glBegin(GL_QUADS);
    glColor4f(0.7, 0.7, 0.7, 0.5);
    glVertex2f(0, 0);
    glVertex2f(2, 0);
    glVertex2f(2, -0.13);
    glVertex2f(0, -0.13);
    glColor4f(0.5, 0.5, 0.5, 0.5);
    glVertex2f(0, 0 - 0.13);
    glVertex2f(0.5, -0.13);
    glVertex2f(0.5, -2);
    glVertex2f(0, -2);
    glEnd();

    char c[5];
	sprintf(c,"%d",curlevel);
    glColor4f(1, 1, 1,1);
    glTranslatef(0.04*47, -0.05, 0);
    print_straight_text(c);
    glTranslatef(-0.04*47, 0.05, 0);

    switch (page)
    {
    case (E_FileStuff):
                    drawFileMenu();
        break;

    case (E_ColourScheme):
                    drawColourChart();
        break;

    case (E_LevelGoals):
                    drawLevelObjectives();
        break;

    case (E_EntityChooser):
                    drawEntityChooser();
        break;

    case (E_TextChooser):
                    drawTextChooser();
        break;

    case (E_RoomAdder):
                    drawRoomAdder();
        break;

    case (E_PortalMode):
                    drawPortalMode();
        break;

    case (E_Other):
                    drawOtherMode();
        break;

    default:
        break;
    }

    glPopMatrix();
}

int curtitleselection2old;
int curtitleselection3old;

void EditorState::input(char c)
{
    if (!editmode)
        return;

    switch (page)
    {
    case E_FileStuff:
    {
        if (c == '[')
        {
            itemselected = (itemselected + 5 - 1) % 5;
            if (itemselected==4)
            {
                prompttodelete=false;
            }
        }
        else if (c == ']')
        {
            itemselected = (itemselected + 1) % 5;
            if (itemselected==4)
            {
                prompttodelete=false;
            }
        }
        else if (c == ' ')
        {
            switch (itemselected)
            {
            case 0:
                playsound(0);
                initlevel();
                break;

            case 1:
            {
                playsound(0);
                string s;
                s = "custom/" + curchapter;
                s += "/" + stringify(curlevel) + ".dat";
                writelevel(s.c_str(), level);

                //sprintf(blah,"areas/area%d.dat",curlevel);

                //writelevel(blah, level);
                break;
            }

            case 2:
                //SAVE AS
                wantquit = true;
                playsound(0);
                gamestate = gs_title;
                tso = ts_editchapterselect;
                saveasmode = true;
                curtitleselection2old = curtitleselection2;
                curtitleselection3old = curtitleselection3;
                ignorenextspaceup = true;
                break;

            case 3:
                playsound(0);
                blankLevel(level);
                break;

            case 4:
                if (!prompttodelete)
                {
                    prompttodelete=true;
                }
                else
                {
                    if (episodecount == 1)
                    {
                        playsound(0);
                        string s = "custom/" + curchapter;
                        deleteDir(s);
                        getEpisodeData();
                        resetlistbounds = true;
                        //curtitleselection2=0;
                        if (customEpisodeList.size() == 0)
                        {
                            tso = ts_editnofiles;
                            enteringname = "";
                        }
                        else
                        {
                            tso = ts_editgameselect;
                            curtitleselection2 = 0;
                            enteringname = "";
                            curtitleselection3 = 0;
                        }
                        wantquit = true;
                        gamestate = gs_title;
                        ignorenextspaceup = true;
                    }
                    else
                    {
                        playsound(0);
                        string s = "custom/" + curchapter + "/" + stringify(curlevel) + ".dat";
                        deleteDir(s);

                        for (int i = curlevel + 1; i < episodecount; i++)
                        {
                            string s = "custom/" + curchapter + "/" + stringify(i) + ".dat";
                            string t = "custom/" + curchapter + "/" + stringify(i - 1) + ".dat";
                            renameFile(s, t);
                        }

                        episodecount--;
                        getEpisodeData();
                        resetlistbounds = true;
                        curtitleselection2 = 0;
                        curtitleselection3 = 0;
                        tso = ts_editgameselect;
                        wantquit = true;
                        gamestate = gs_title;
                        ignorenextspaceup = true;
                    }
                    prompttodelete=false;
                }
                break;
                //DELETE LEVEL CODE HERE...if you delete thing with only one level, delete episode also
            }
        }
    }
    break;

    case E_ColourScheme:
    {
        if (c == '[')
        {
            itemselected = (itemselected + 10 - 1) % 10;
        }
        else if (c == ']')
        {
            itemselected = (itemselected + 1) % 10;
        }
        else if (itemselected < 6)
        {
            GLfloat* cs = NULL;
            switch (itemselected)
            {
            case 0:
                cs = &level.colourscheme.outline[0];
                break;

            case 1:
                cs = &level.colourscheme.oddroomunvisited[0];
                break;

            case 2:
                cs = &level.colourscheme.evenroomunvisited[0];
                break;

            case 3:
                cs = &level.colourscheme.oddroomvisited[0];
                break;

            case 4:
                cs = &level.colourscheme.evenroomvisited[0];
                break;

            case 5:
                cs = &level.colourscheme.background[0];
                break;

            default:
                //error!
                break;
            }

            switch (c)
            {
            case 'q':
                cs[0] = min(cs[0] + 0.1, 1);
                if (itemselected == 5)
                    glClearColor(level.colourscheme.background[0], level.colourscheme.background[1], level.colourscheme.background[2], 1);
                break;

            case 'a':
                cs[0] = max(cs[0] - 0.1, 0);
                if (itemselected == 5)
                    glClearColor(level.colourscheme.background[0], level.colourscheme.background[1], level.colourscheme.background[2], 1);
                break;

            case 'w':
                cs[1] = min(cs[1] + 0.1, 1);
                if (itemselected == 5)
                    glClearColor(level.colourscheme.background[0], level.colourscheme.background[1], level.colourscheme.background[2], 1);
                break;

            case 's':
                cs[1] = max(cs[1] - 0.1, 0);
                if (itemselected == 5)
                    glClearColor(level.colourscheme.background[0], level.colourscheme.background[1], level.colourscheme.background[2], 1);
                break;

            case 'e':
                cs[2] = min(cs[2] + 0.1, 1);
                if (itemselected == 5)
                    glClearColor(level.colourscheme.background[0], level.colourscheme.background[1], level.colourscheme.background[2], 1);
                break;

            case 'd':
                cs[2] = max(cs[2] - 0.1, 0);
                if (itemselected == 5)
                    glClearColor(level.colourscheme.background[0], level.colourscheme.background[1], level.colourscheme.background[2], 1);
                break;

            default:
                break;
            }
        }
        else
        {
            if (c == ' ')
            {
                switch (itemselected)
                {
                case 6:
                    level.colourscheme.edgetransparency = !level.colourscheme.edgetransparency;
                    break;

                case 7:
                    level.colourscheme.roomtransparency = !level.colourscheme.roomtransparency;
                    break;

                case 8:
                    level.colourscheme.entitytransparency = !level.colourscheme.entitytransparency;
                    break;

                case 9:
                    level.colourscheme.noplayerdup = !level.colourscheme.noplayerdup;
                    break;
                }
            }
        }
    }
    break;

    case E_LevelGoals:
    {
        int curselectmax;
        if (level.levelparams.exploreon)
            curselectmax = 4;
        else if (level.levelparams.enterroom)
            curselectmax = 5;
        else if (level.levelparams.mirrorexit)
            curselectmax = 2;
        else if (level.levelparams.timeron)
            curselectmax = 2;
        else
            curselectmax = 1;

        if (c == '[')
        {
            itemselected = (itemselected + curselectmax - 1) % curselectmax;
        }
        else if (c == ']')
        {
            itemselected = (itemselected + 1) % curselectmax;
        }
        else if (itemselected == 0)
        {
            if (c == ' ')
            {
                if (level.levelparams.exploreon)
                {
                    level.levelparams.exploreon = false;
                    level.levelparams.goalroomparity = -1;
                    level.levelparams.goalroomscale = -1;
                    level.levelparams.timer=0;
                    level.levelparams.enterroom = true;
                    curselectmax=4;
                }
                else if (level.levelparams.enterroom)
                {
                    level.levelparams.enterroom = false;
                    level.levelparams.mirrorexit = true;
                    level.levelparams.timer = 0;                            //for 'turn of portals' option...
                }
                else if (level.levelparams.mirrorexit)
                {
                    level.levelparams.mirrorexit = false;
                    level.levelparams.timeron = true;
                }
                else if (level.levelparams.timeron)
                {
                    level.levelparams.timeron = false;
                }
                else
                {
                    if (level.levelparams.goalroom == 0)
                        level.levelparams.goalroom = -1;
                    else
                    {
                        level.levelparams.goalroom = 0;
                        level.levelparams.exploreon = true;
                    }
                }
            }
        }
        else
        {
            if (level.levelparams.exploreon)
            {
                switch (itemselected)
                {
                case 1:
                    if (c == 'q')
                        level.levelparams.roomstoexplore = max(level.levelparams.roomstoexplore - 1, 0);
                    else if (c == 'e')
                        level.levelparams.roomstoexplore = level.levelparams.roomstoexplore + 1;
                    break;

                case 2:
                    if (c == ' ')
                    {
                        level.levelparams.paritymatters = !level.levelparams.paritymatters;
                        level.levelparams.scaleandparitymatter = level.levelparams.scalematters && level.levelparams.paritymatters;
                    }
                    break;

                case 3:
                    if (c == ' ')
                    {
                        level.levelparams.scalematters = !level.levelparams.scalematters;
                        level.levelparams.scaleandparitymatter = level.levelparams.scalematters && level.levelparams.paritymatters;
                    }
                    break;

                default:
                    break;
                }
            }
            else if (level.levelparams.enterroom)
            {
                switch (itemselected)
                {
                case 1:
                    if (c == ' ')
                        level.levelparams.goalroom = level.playerpos.room;
                    if (level.levelparams.goalroomparity >= 0)
                        level.levelparams.goalroomparity = level.playerpos.par;
                    if (level.levelparams.goalroomscale >= 0)
                    {
                        level.levelparams.goalroomscale = getscalevisit(level.playerpos.scale);
                    }
                    break;

                case 2:
                    if (c == ' ')
                    {
                        if (level.levelparams.goalroomparity >= 0)
                            level.levelparams.goalroomparity = -1;
                        else
                            level.levelparams.goalroomparity = level.playerpos.par;
                    }
                    break;

                case 3:
                    if (c == ' ')
                    {
                        if (level.levelparams.goalroomscale >= 0)
                        {
                            level.levelparams.goalroomscale = -1;
                            curselectmax=4;
                        }
                        else
                        {
                            curselectmax=5;
                            level.levelparams.goalroomscale = getscalevisit(level.playerpos.scale);
                        }
                    }
                    break;
                case 4:
                    if (c == ' ')
                    {
                        if (level.levelparams.timer==0)
                        {
                            level.levelparams.timer=1;
                        }
                        else if (level.levelparams.timer>0)
                        {
                            level.levelparams.timer =-1;
                        }
                        else
                        {
                            level.levelparams.timer=0;
                        }

                     //HAVE TO FIND SOME WAY TO STORE THINGS HERE
                    }
                break;

                default:
                    break;
                }
            }
            else if (level.levelparams.mirrorexit)
            {
                if (itemselected == 1 && c == ' ')
                {
                    if (level.levelparams.timer == 5)
                        level.levelparams.timer = 0;
                    else level.levelparams.timer = 5;
                }
            }
            else if (level.levelparams.timeron)
            {
                //this if statement isn't strictly necessary
                if (itemselected == 1)
                {
                    if (c == 'q')
                        level.levelparams.timer = max(level.levelparams.timer - 20, 0);
                    else if (c == 'e')
                        level.levelparams.timer = level.levelparams.timer + 20;
                }
            }
            else
            {
            }
        }
    }
    break;

    case E_EntityChooser:
    {
        int curselectmax = 0;

        switch (level.entitydata.curEntityPositions[level.playerpos.room].type)
        {
        case e_EmptyRoom:
            curselectmax = 1;
            break;

        case e_Doctor:
        case e_BigDoctor:
            curselectmax = 2;
            break;

        case e_Mover:
        case e_BigMover:
            curselectmax = 2;
            break;

        case e_Sentry:
            curselectmax = 2;
            break;

        case e_ParityMover:
            curselectmax = 3;
            break;

        case e_Wall:
            curselectmax = 3;
            break;

        case e_Goal_Table:                        //these need scales!  also possibly use parameters to encode objectives
            curselectmax = 1;
            break;

        case e_Goal_Flower:
            curselectmax = 1;
            break;

        case e_Table:
            curselectmax = 1;
            break;

        case e_Bathroom:
            curselectmax = 1;
            break;

        case e_Bedroom:
            curselectmax = 1;
            break;

        case e_Goal_Bedroom:
            curselectmax = 1;
            break;

        case e_TableMover:
            curselectmax = 2;
            break;

        default:
            break;
        }


        if (c == '[')
        {
            itemselected = (itemselected + curselectmax - 1) % curselectmax;
        }
        else if (c == ']')
        {
            itemselected = (itemselected + 1) % curselectmax;
        }
        else if (itemselected == 0)
        {
            if (c == ' ')
            {
                level.entitydata.curEntityPositions[level.playerpos.room].x = 0;
                level.entitydata.curEntityPositions[level.playerpos.room].y = 0;
                level.entitydata.curEntityPositions[level.playerpos.room].x2 = 0;
                level.entitydata.curEntityPositions[level.playerpos.room].y2 = 0;
                level.entitydata.curEntityPositions[level.playerpos.room].x3 = 0;
                level.entitydata.curEntityPositions[level.playerpos.room].y3 = 0;
                switch (level.entitydata.curEntityPositions[level.playerpos.room].type)
                {
                case e_EmptyRoom:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Doctor;
                    level.entitydata.curEntityPositions[level.playerpos.room].dat = 0.02f;
                    break;

                case e_Doctor:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Mover;
                    level.entitydata.curEntityPositions[level.playerpos.room].dat = 0.02f;
                    break;

                case e_Mover:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Sentry;
                    level.entitydata.curEntityPositions[level.playerpos.room].dat = 0.1f;
                    break;

                case e_Sentry:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_ParityMover;
                    level.entitydata.curEntityPositions[level.playerpos.room].dat = 0.02f;
                    break;

                case e_ParityMover:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Wall;
                    level.entitydata.curEntityPositions[level.playerpos.room].dat = 0.02f;
                    level.entitydata.curEntityPositions[level.playerpos.room].x3 = level.playerpos.dir;
                    break;

                case e_Wall:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Goal_Table;
                    break;

                case e_Goal_Table:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Goal_Flower;
                    break;

                case e_Goal_Flower:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Table;
                    break;

                case e_Table:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Bedroom;
                    break;

                case e_Bedroom:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Goal_Bedroom;
                    break;

                case e_Goal_Bedroom:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_Bathroom;
                    break;

                case e_Bathroom:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_TableMover;
                    level.entitydata.curEntityPositions[level.playerpos.room].dat = 0.1f;
                    break;

                case e_TableMover:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_BigMover;
                    level.entitydata.curEntityPositions[level.playerpos.room].dat = 0.02f;
                    break;

                case e_BigMover:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_BigDoctor;
                    level.entitydata.curEntityPositions[level.playerpos.room].dat = 0.02f;
                    break;

                case e_BigDoctor:
                    level.entitydata.curEntityPositions[level.playerpos.room].type = e_EmptyRoom;
                    break;

                default:
                    break;
                }
            }
        }
        else if (itemselected == 1)
        {
            if (c == 'q')
            {
                switch (level.entitydata.curEntityPositions[level.playerpos.room].type)
                {
                case e_Sentry:
                {
                    level.entitydata.curEntityPositions[level.playerpos.room].dat -= 0.05;
                }
                break;

                case e_EmptyRoom:
                    break;

                case e_TableMover:
                case e_Mover:
                case e_Doctor:
                case e_BigDoctor:
                case e_ParityMover:
                case e_Wall:
                case e_BigMover:
                {
                    level.entitydata.curEntityPositions[level.playerpos.room].dat *= 0.9;
                    if (level.entitydata.curEntityPositions[level.playerpos.room].dat < 0.02)
                        level.entitydata.curEntityPositions[level.playerpos.room].dat = 0;
                }
                break;

                case e_Goal_Table:
                case e_Goal_Flower:
                case e_Table:
                default:
                    break;
                }
            }
            else if (c == 'e')
            {
                switch (level.entitydata.curEntityPositions[level.playerpos.room].type)
                {
                case e_Sentry:
                {
                    level.entitydata.curEntityPositions[level.playerpos.room].dat += 0.05;
                }
                break;

                case e_EmptyRoom:
                    break;

                case e_TableMover:
                case e_Doctor:
                case e_BigDoctor:
                case e_Mover:
                case e_ParityMover:
                case e_Wall:
                case e_BigMover:
                {
                    if (level.entitydata.curEntityPositions[level.playerpos.room].dat < 0.01)
                        level.entitydata.curEntityPositions[level.playerpos.room].dat = 0.02;
                    else
                        level.entitydata.curEntityPositions[level.playerpos.room].dat *= 1.1;
                }
                break;

                case e_Goal_Table:
                case e_Goal_Flower:
                case e_Table:
                default:
                    break;
                }
            }
        }
        else if (itemselected == 2)
        {
            if (c == 'q')
            {
                switch (level.entitydata.curEntityPositions[level.playerpos.room].type)
                {
                case e_Wall:
                {
                    level.entitydata.curEntityPositions[level.playerpos.room].x3 += 0.1;
                }
                break;

                case e_ParityMover:
                    if (level.entitydata.curEntityPositions[level.playerpos.room].x2 > 10)
                        level.entitydata.curEntityPositions[level.playerpos.room].x2 = 2.5f;
                    else if (level.entitydata.curEntityPositions[level.playerpos.room].x2 > 5)
                        level.entitydata.curEntityPositions[level.playerpos.room].x2 = 12.5f;
                    else
                        level.entitydata.curEntityPositions[level.playerpos.room].x2 = 7.5f;
                    break;

                default:
                    break;
                }
            }
            else if (c == 'e')
            {
                switch (level.entitydata.curEntityPositions[level.playerpos.room].type)
                {
                case e_Wall:
                {
                    level.entitydata.curEntityPositions[level.playerpos.room].x3 += 0.1;
                }
                break;

                case e_ParityMover:
                    if (level.entitydata.curEntityPositions[level.playerpos.room].x2 > 10)
                        level.entitydata.curEntityPositions[level.playerpos.room].x2 = 7.5f;
                    else if (level.entitydata.curEntityPositions[level.playerpos.room].x2 > 5)
                        level.entitydata.curEntityPositions[level.playerpos.room].x2 = 2.5f;
                    else
                        level.entitydata.curEntityPositions[level.playerpos.room].x2 = 12.5f;
                    break;

                default:
                    break;
                }
            }
            else if (c == ' ')
            {
                if (level.entitydata.curEntityPositions[level.playerpos.room].type == e_ParityMover)
                {
                    if (level.entitydata.curEntityPositions[level.playerpos.room].x2 > 10)
                        level.entitydata.curEntityPositions[level.playerpos.room].x2 = 7.5f;
                    else if (level.entitydata.curEntityPositions[level.playerpos.room].x2 > 5)
                        level.entitydata.curEntityPositions[level.playerpos.room].x2 = 2.5f;
                    else if (level.entitydata.curEntityPositions[level.playerpos.room].x2 > 10)
                        level.entitydata.curEntityPositions[level.playerpos.room].x2 = 12.5f;
                }
            }
        }
    }
    break;

    case E_RoomAdder:
    {
        if (c == 'q')
        {
            shapeselected = (shapeselected + SHAPECOUNT - 1) % SHAPECOUNT;
            wallselected = 0;
        }
        else if (c == 'e')
        {
            shapeselected = (shapeselected + 1) % SHAPECOUNT;
            wallselected = 0;
        }
        else if (c == 'a')
            wallselected = (wallselected + shapesize[shapeselected] - 1) % shapesize[shapeselected];
        else if (c == 'd')
            wallselected = (wallselected + 1) % shapesize[shapeselected];
        else if ((c == 'z' && !level.playerpos.par) || (c == 'c' && level.playerpos.par))
            realwallselected = (realwallselected + 1) % shapesize[level.rooms[level.playerpos.room].shape];
        else if (c == 'c' || c == 'z')
            realwallselected = (realwallselected + shapesize[level.rooms[level.playerpos.room].shape] - 1) % shapesize[level.rooms[level.playerpos.room].shape];
        else if (c == 'r')
        {
            Portal p = { level.playerpos.room, realwallselected };
            removePortal(p);
        }
        else if (c == ' ')
        {
            Portal p = { level.playerpos.room, realwallselected };
            addRoom(p, shapeselected, wallselected, newportalparity, newportalloc);
        }
        else if (c == 'f')
        {
            newportalparity = !newportalparity;
        }
        else if (c == 'v')
        {
            newportalloc = !newportalloc;
        }
        else if (c == 't')
        {
            level.rooms[level.playerpos.room].localportal[realwallselected] = !level.rooms[level.playerpos.room].localportal[realwallselected];
        }
    }
    break;

    case E_PortalMode:
    {
        /*	if (c=='z')
                        realwallselected=(realwallselected+shapesize[level.rooms[level.playerpos.room].shape]-1)%shapesize[level.rooms[level.playerpos.room].shape];
                else if (c=='c')
                        realwallselected=(realwallselected+1)%shapesize[level.rooms[level.playerpos.room].shape];*/
        if ((c == 'z' && !level.playerpos.par) || (c == 'c' && level.playerpos.par))
            realwallselected = (realwallselected + 1) % shapesize[level.rooms[level.playerpos.room].shape];
        else if (c == 'c' || c == 'z')
            realwallselected = (realwallselected + shapesize[level.rooms[level.playerpos.room].shape] - 1) % shapesize[level.rooms[level.playerpos.room].shape];
        else if (c == 'x')
        {
            portalTo.room = level.playerpos.room;
            portalTo.side = realwallselected;
        }
        else if (c == ' ')
        {
            Portal p = { level.playerpos.room, realwallselected };
            addPortal(p, portalTo, newportalparity, newportalloc);
        }
        else if (c == 'r')
        {
            Portal p = { level.playerpos.room, realwallselected };
            removePortal(p);
        }
        else if (c == 'f')
        {
            newportalparity = !newportalparity;
        }
        else if (c == 'v')
        {
            newportalloc = !newportalloc;
        }
        else if (c == 't')
        {
            level.rooms[level.playerpos.room].localportal[realwallselected] = !level.rooms[level.playerpos.room].localportal[realwallselected];
        }
    }
    break;

    case E_Other:
    {
        if (c == '[')
        {
            itemselected = (itemselected + 2 - 1) % 2;
        }
        else if (c == ']')
        {
            itemselected = (itemselected + 1) % 2;
        }
        else switch (itemselected)
            {
            case 0:
                if (c == 'q')
                    level.playerpos.scale *= 1.1;
                else if (c == 'e')
                    level.playerpos.scale /= 1.1;
                break;

            case 1:
                if (c == 'q')
                {
                    level.levelparams.music = (level.levelparams.music + MUSICCOUNT - 1) % MUSICCOUNT;
                    loadtrack(level.levelparams.music);
                }
                else if (c == 'e')
                {
                    level.levelparams.music = (level.levelparams.music + 1) % MUSICCOUNT;
                    loadtrack(level.levelparams.music);
                }
                break;

            default:
                break;
            }
    }
    break;

    default:
        break;
    }
    ;
}

void EditorState::drawOtherMode()
{
    glTranslatef(0.05, -0.05, 0);

    string s;
    switch (itemselected)
    {
    case 0:
        s = "current scale";
        break;

    case 1:
        s = "background music";
        break;

    default:
        break;
    }


    glColor3f(1, 1, 1);
    print_straight_text(s.c_str());

    glTranslatef(0.05, 0, 0);

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);

    glColor4f(0.5, 0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(-0.05, -(colourbox_height + colourbox_padding) * (itemselected));
    glVertex2f(0.35, -(colourbox_height + colourbox_padding) * (itemselected));
    glVertex2f(0.35, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected));
    glVertex2f(-0.05, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected));
    glEnd();

    glColor4f(1, 1, 1, 1);
    glTranslatef(-0.05, -0.03, 0);

    {
        std::stringstream out;
        out << level.playerpos.scale;
        s = out.str();

        print_straight_text(s);
    }

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);

    {
        std::stringstream out;
        out << level.levelparams.music;
        s = out.str();

        print_straight_text(s);
    }
}

void EditorState::drawPortalMode()
{
    glTranslatef(0.05, -0.05, 0);
    glColor3f(1, 1, 1);
    print_straight_text("add new portal");

    glTranslatef(0.05, 0, 0);

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);

    glTranslatef(-0.05, -0.03, 0);

    if (newportalparity)
        print_straight_text("even");
    else
        print_straight_text("odd");

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);

    if (newportalloc)
        print_straight_text("opaque");
    else
        print_straight_text("trans");
}


void EditorState::drawRoomAdder()
{
    glTranslatef(0.05, -0.05, 0);
    \
    glColor3f(1, 1, 1);
    print_straight_text("add new room");
    glPushMatrix();
    glTranslatef(0.2, -0.3, 0);

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    rotamount += 0.5f;

    GLfloat rn = 0.2 / roomNormalizations[shapeselected];

    glColor3fv(&level.colourscheme.background[0]);

    glBegin(GL_QUADS);
    glVertex2f(-0.25, 0.3);
    glVertex2f(-0.25, -0.3);
    glVertex2f(0.25, -0.3);
    glVertex2f(0.25, 0.3);
    glEnd();

    glScalef(rn, rn, rn);

    glRotatef(rotamount, 0, 0, 1);

    glColor3fv(&level.colourscheme.oddroomunvisited[0]);
    glCallList(level.displaylists.shapelist + shapeselected);

    glColor3f(0, 1, 0);
    glBegin(GL_TRIANGLES);
    glVertex2f(0, 0);
    glVertex2fv(shapes[shapeselected][wallselected]);
    glVertex2fv(shapes[shapeselected][wallselected + 1]);
    glEnd();
    glPopMatrix();

    glTranslatef(0.05, -0.64, 0);

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);

    glTranslatef(-0.05, -0.03, 0);
    glColor3f(1, 1, 1);
    if (newportalparity)
        print_straight_text("even");
    else
        print_straight_text("odd");

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);

    if (newportalloc)
        print_straight_text("opaque");
    else
        print_straight_text("trans");
}

void EditorState::drawTextChooser()
{
    glTranslatef(0.05, -0.05, 0);
    glColor3f(1, 1, 1);
    print_straight_text("prelude text");
    glTranslatef(-0.05, 0.05, 0);




    glColor4f(0.5, 0.5, 0.5, 0.9);
    glBegin(GL_QUADS);
    glVertex2f(0.2, -0.4);
    glVertex2f(0.2, -1.75);
    glVertex2f(1.5, -1.75);
    glVertex2f(1.5, -0.4);
    glEnd();

    glColor3f(1.0, 1.0, 1.0);
    {
        glTranslatef(0.3, -0.5, 0);
        glScalef(0.6, 0.6, 0.6);
        glPushMatrix();

        print_text2(level.introtext.c_str());

        glPopMatrix();
    }
}

void EditorState::drawColourBox()
{
    glBegin(GL_QUADS);
    glVertex2f(0.0, 0);
    glVertex2f(0.3, 0);
    glVertex2f(0.3, -colourbox_height);
    glVertex2f(0.0, -colourbox_height);
    glEnd();
}

void EditorState::drawColourChart()
{
    glTranslatef(0.05, -0.05, 0);

    string s;
    switch (itemselected)
    {
    case 0:
        s = "outline colour";
        break;

    case 1:
        s = "unreached odd room colour";
        break;

    case 2:
        s = "unreached even room colour";
        break;

    case 3:
        s = "visited odd room colour";
        break;

    case 4:
        s = "visited even room colour";
        break;

    case 5:
        s = "background colour";
        break;

    case 6:
        s = "edge transparency";
        break;

    case 7:
        s = "room transparency";
        break;

    case 8:
        s = "entity transparency";
        break;

    case 9:
        s = "show player reflections";
        break;

    default:
        break;
    }

    glColor3f(1, 1, 1);
    print_straight_text(s.c_str());

    glTranslatef(0.05, 0, 0);

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);

    glColor4f(0.5, 0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(-0.05, -(colourbox_height + colourbox_padding) * (itemselected + (itemselected > 5 ? 0.8 : 0)));
    glVertex2f(0.35, -(colourbox_height + colourbox_padding) * (itemselected + (itemselected > 5 ? 0.8 : 0)));
    glVertex2f(0.35, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected + (itemselected > 5 ? 0.8 : 0)));
    glVertex2f(-0.05, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected + (itemselected > 5 ? 0.8 : 0)));
    glEnd();

    glDisable(GL_BLEND);

    glColor3fv(level.colourscheme.outline);
    drawColourBox();

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    glColor3fv(level.colourscheme.oddroomunvisited);
    drawColourBox();

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    glColor3fv(level.colourscheme.evenroomunvisited);
    drawColourBox();

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    glColor3fv(level.colourscheme.oddroomvisited);
    drawColourBox();

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    glColor3fv(level.colourscheme.evenroomvisited);
    drawColourBox();

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    glColor3fv(level.colourscheme.background);
    drawColourBox();


    glEnable(GL_BLEND);

    glTranslatef(-0.05, -colourbox_height - colourbox_padding, 0);
    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    glColor4f(1, 1, 1, (level.colourscheme.edgetransparency ? 0.3 : 1));
    print_straight_text("edges");

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    glColor4f(1, 1, 1, (level.colourscheme.roomtransparency ? 0.3 : 1));
    print_straight_text("rooms");

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    glColor4f(1, 1, 1, (level.colourscheme.entitytransparency ? 0.3 : 1));
    print_straight_text("entities");

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    glColor4f(1, 1, 1, (level.colourscheme.noplayerdup ? 0.3 : 1));
    print_straight_text("reflects");
}



void EditorState::drawFileMenu()
{
    glTranslatef(0.05, -0.05, 0);

    string s;
    switch (itemselected)
    {
    case 0:
        s = "revert to previous version";
        break;

    case 1:
        s = "save over previous version";
        break;

    case 2:
        s = "save as";
        break;

    case 3:
        s = "blank level";
        break;

    case 4:
        s = "delete level";
        break;

    default:
        break;
    }

    glColor3f(1, 1, 1);
    print_straight_text(s.c_str());

    glTranslatef(0.05, 0, 0);

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);




    if (itemselected==4 && prompttodelete)
    {
        glColor4f(0.5, 0.5, 0.5, 0.5);
        glBegin(GL_QUADS);
        glVertex2f(-0.05, -(colourbox_height + colourbox_padding) * (itemselected));
        glVertex2f(1.4, -(colourbox_height + colourbox_padding) * (itemselected));
        glVertex2f(1.4, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected));
        glVertex2f(-0.05, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected));
        glEnd();
    }
    else
    {
        glColor4f(0.5, 0.5, 0.5, 0.5);
        glBegin(GL_QUADS);
        glVertex2f(-0.05, -(colourbox_height + colourbox_padding) * (itemselected));
        glVertex2f(0.35, -(colourbox_height + colourbox_padding) * (itemselected));
        glVertex2f(0.35, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected));
        glVertex2f(-0.05, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected));
        glEnd();
    }

    glColor4f(1, 1, 1, 1);
    glTranslatef(-0.05, -0.03, 0);
    print_straight_text("revert");

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    print_straight_text("save");

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    print_straight_text("save as");

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);
    print_straight_text("blank");

    if (itemselected==4 && prompttodelete)
    {
        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        print_straight_text("delete -> are you sure?");
    }
    else
    {
        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        print_straight_text("delete");
    }
}


void EditorState::drawLevelObjectives()
{
    glTranslatef(0.05, -0.05, 0);

    string s;
    if (level.levelparams.exploreon)
    {
        switch (itemselected)
        {
        case 0:
            s = "explore x rooms to progress";
            break;

        case 1:
            s = "number of rooms to explore";
            break;

        case 2:
            s = "does orientation matter?";
            break;

        case 3:
            s = "does scale matter?";
            break;
        }
    }
    else if (level.levelparams.enterroom)
    {
        switch (itemselected)
        {
        case 0:
            s = "enter a room to progress";
            break;

        case 1:
            s = "make current room the goal room";
            break;

        case 2:
            s = "does orientation matter?";
            break;

        case 3:
            s = "does scale matter?";
            break;

        case 4:
        {

            if (level.levelparams.timer==0)
            {
                s = "this size";
            }
            else if (level.levelparams.timer>0)
            {
                s = "at least this size";
            }
            else
            {
                s = "at most this size";
            }
        }
            break;
        }
    }
    else if (level.levelparams.mirrorexit)
    {
        switch (itemselected)
        {
        case 0:
            s = "explore a mirror to progress";
            break;

        case 1:
            s = "enable/disable portal movement";
            break;
        }
    }
    else if (level.levelparams.timeron)
    {
        switch (itemselected)
        {
        case 0:
            s = "survive for a time x to progress";
            break;

        case 1:
            s = "waiting time in seconds";
            break;
        }
    }
    else
    {
        if (level.levelparams.goalroom == -1)
            s = "other goal";
        else
            s = "text only area";
    }

    glColor3f(1, 1, 1);
    print_straight_text(s.c_str());

    glTranslatef(0.05, 0, 0);

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);

    glColor4f(0.5, 0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(-0.05, -(colourbox_height + colourbox_padding) * (itemselected + (itemselected > 0 ? 1 : 0)));
    glVertex2f(0.35, -(colourbox_height + colourbox_padding) * (itemselected + (itemselected > 0 ? 1 : 0)));
    glVertex2f(0.35, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected + (itemselected > 0 ? 1 : 0)));
    glVertex2f(-0.05, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected + (itemselected > 0 ? 1 : 0)));
    glEnd();

    glColor4f(1, 1, 1, 1);
    glTranslatef(-0.05, -0.03, 0);

    if (level.levelparams.exploreon)
    {
        print_straight_text("explore");

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);

        std::string s;
        std::stringstream out;
        out << level.levelparams.roomstoexplore;
        s = out.str();

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        print_straight_text(s);

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        glColor4f(1, 1, 1, (!level.levelparams.paritymatters ? 0.3 : 1));
        print_straight_text("orient");

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        glColor4f(1, 1, 1, (!level.levelparams.scalematters ? 0.3 : 1));
        print_straight_text("scale");
    }
    else if (level.levelparams.enterroom)
    {
        print_straight_text("visit");
        glTranslatef(0, -colourbox_height - colourbox_padding, 0);

        std::string s;
        std::stringstream out;
        out << level.levelparams.goalroom;
        s = out.str();

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        print_straight_text(s);

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        glColor4f(1, 1, 1, ((level.levelparams.goalroomparity < 0) ? 0.3 : 1));
        print_straight_text("orient");

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        glColor4f(1, 1, 1, ((level.levelparams.goalroomscale < 0) ? 0.3 : 1));
        print_straight_text("scale");

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        glColor4f(1, 1, 1, ((level.levelparams.goalroomscale < 0) ? 0.3 : 1));

        if (level.levelparams.goalroomscale>=0)
        {
            if (level.levelparams.timer==0)
            {
                print_straight_text("at");
            }
            else if (level.levelparams.timer>0)
            {
                print_straight_text("larger");
            }
            else
            {
                print_straight_text("smaller");
            }
        }

    }
    else if (level.levelparams.mirrorexit)
    {
        print_straight_text("mirror");

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        if (level.levelparams.timer == 5)
            print_straight_text("nomove");
        else
            print_straight_text("move");
    }
    else if (level.levelparams.timeron)
    {
        print_straight_text("timer");
        glTranslatef(0, -colourbox_height - colourbox_padding, 0);

        std::string s;
        std::stringstream out;
        out << (float)level.levelparams.timer / 30.0f;
        s = out.str();

        glTranslatef(0, -colourbox_height - colourbox_padding, 0);
        print_straight_text(s);
    }
    else
    {
        if (level.levelparams.goalroom == -1)
            print_straight_text("other");
        else
            print_straight_text("text");
    }
}

void EditorState::drawEntityChooser()
{
    glTranslatef(0.05, -0.05, 0);
    glColor3f(1, 1, 1);
    print_straight_text("entity in current room");


    glTranslatef(0.05, 0, 0);

    glTranslatef(0, -colourbox_height - colourbox_padding, 0);

    glColor4f(0.5, 0.5, 0.5, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(-0.05, -(colourbox_height + colourbox_padding) * (itemselected + (itemselected > 0 ? 1 : 0)));
    glVertex2f(0.35, -(colourbox_height + colourbox_padding) * (itemselected + (itemselected > 0 ? 1 : 0)));
    glVertex2f(0.35, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected + (itemselected > 0 ? 1 : 0)));
    glVertex2f(-0.05, -colourbox_height - (colourbox_height + colourbox_padding) * (itemselected + (itemselected > 0 ? 1 : 0)));
    glEnd();

    glColor4f(1, 1, 1, 1);
    glTranslatef(-0.05, -0.03, 0);



    switch (level.entitydata.curEntityPositions[level.playerpos.room].type)
    {
    case e_EmptyRoom:
        print_straight_text("empty");
        break;

    case e_Doctor:
        print_straight_text("doctor");
        break;

    case e_Mover:
        print_straight_text("mover");
        break;

    case e_Sentry:
        print_straight_text("sentry");
        break;

    case e_ParityMover:
        print_straight_text("parity");
        break;

    case e_Wall:
        print_straight_text("laser");
        break;

    case e_Goal_Table:
        print_straight_text("endtable");
        break;

    case e_Goal_Flower:
        print_straight_text("endflower");
        break;

    case e_Table:
        print_straight_text("table");
        break;

    case e_Bedroom:
        print_straight_text("bedroom");
        break;

    case e_Goal_Bedroom:
        print_straight_text("bedroom2");
        break;

    case e_Bathroom:
        print_straight_text("bathroom");
        break;

    case e_TableMover:
        print_straight_text("table sen");
        break;

    case e_BigMover:
        print_straight_text("bigmover");
        break;

    case e_BigDoctor:
        print_straight_text("bigdoctor");
        break;

    default:
        break;
    }


    switch (level.entitydata.curEntityPositions[level.playerpos.room].type)
    {
    case e_Mover:
    case e_Sentry:
    case e_ParityMover:
    case e_Doctor:
    case e_TableMover:
    case e_Wall:
    case e_BigMover:
    case e_BigDoctor:
    {
        glTranslatef(0, -colourbox_height - colourbox_padding, 0);

        {
            std::string s;
            std::stringstream out;
            out << setprecision(3) << level.entitydata.curEntityPositions[level.playerpos.room].dat;
            s = out.str();

            glTranslatef(0, -colourbox_height - colourbox_padding, 0);
            print_straight_text(s);
        }

        if (level.entitydata.curEntityPositions[level.playerpos.room].type == e_Wall)
        {
            std::string s;
            std::stringstream out;
            out << setprecision(3) << level.entitydata.curEntityPositions[level.playerpos.room].x3;
            s = out.str();

            glTranslatef(0, -colourbox_height - colourbox_padding, 0);
            print_straight_text(s);
        }
        else if (level.entitydata.curEntityPositions[level.playerpos.room].type == e_ParityMover)
        {
            glTranslatef(0, -colourbox_height - colourbox_padding, 0);
            if (level.entitydata.curEntityPositions[level.playerpos.room].x2 < 5)
                print_straight_text("win");
            else if (level.entitydata.curEntityPositions[level.playerpos.room].x2 < 10)
                print_straight_text("vis");
            else
                print_straight_text("invis");
        }
    }

    default:
        break;
    }
}

EditorState editor;
