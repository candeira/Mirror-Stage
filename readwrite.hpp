#include "boost/multi_array.hpp"
#include<boost/tokenizer.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <fstream>

using namespace boost;


namespace boost {
	namespace serialization {

		template<class Archive>
		void serialize(Archive & ar, LevelState & l, const unsigned int version)
		{

			ar & l.introtext;


			ar & l.playerpos;


			ar & l.levelparams;


			ar & l.colourscheme;


			if (version==0)
			{
				l.rooms.resize(DEFAULTROOMCOUNT);
				for (int i=0;i<DEFAULTROOMCOUNT;i++)
				{
					ar & l.rooms[i];
				}
			}
			else if (version>=1)
			{

				ar & l.rooms;
			}


			ar & l.entitydata;


		}

		template<class Archive>
		void serialize(Archive & ar, PlayerPos & pp, const unsigned int version)
		{
			ar & pp.x;
			ar & pp.y;
			ar & pp.dir;
			ar & pp.room;
			ar & pp.par;
			ar & pp.scale;
		}

		template<class Archive>
		void serialize(Archive & ar, LevelParams & lp, const unsigned int version)
		{
			ar & lp.exploreon;
			ar & lp.roomstoexplore;
			ar & lp.paritymatters;
			ar & lp.scalematters;
			ar & lp.enterroom;
			ar & lp.goalroom;
			ar & lp.goalroomscale;
			ar & lp.goalroomparity;
			ar & lp.mirrorexit;
			ar & lp.timeron;
			ar & lp.timer;
			if (version>=1)
			{
				if (version==1)
				{
					bool temp;
					ar & temp;//was 'editable'
				}
				ar & lp.music;
			}
			else if (version==0)
			{
				lp.music=0;
			}

		}

		template<class Archive>
		void serialize(Archive & ar, EntityData & ed, const unsigned int version)
		{
			int i;

			if (version==0)
			{

			ed.curEntityPositions.clear();
			ed.curEntityPositions.resize(DEFAULTROOMCOUNT);

			for (i=0;i<DEFAULTROOMCOUNT;i++)
				ar & ed.curEntityPositions[i];


			}
			else if (version>=1)
			{
				ar & ed.curEntityPositions;

			}
		}

		template<class Archive>
		void serialize(Archive & ar, Entity & ent, const unsigned int version)
		{
			ar & ent.type;
			ar & ent.dat;
			ar & ent.x;
			ar & ent.y;
			ar & ent.x2;
			ar & ent.y2;
			ar & ent.x3;
			ar & ent.y3;
		}

		template<class Archive>
		void serialize(Archive & ar, Room & r, const unsigned int version)
		{
			ar & r.shape;
//oops...there is no version 1 room...
			if (version<2)
			{
				for (int i=0;i<8;i++)
				{
					ar & r.portal[i];
					ar & r.portalparity[i];
					ar & r.portaltoroom[i];
					ar & r.portaltoedge[i];
					ar & r.localportal[i];
				}
				for (int i=8;i<SHAPESIZE;i++)
				{
					r.portal[i]=0;
					r.portalparity[i]=0;
					r.portaltoroom[i]=0;
					r.portaltoedge[i]=0;
					r.localportal[i]=0;
				}

			}
			else
			{
				for (int i=0;i<SHAPESIZE;i++)
				{
					ar & r.portal[i];
					ar & r.portalparity[i];
					ar & r.portaltoroom[i];
					ar & r.portaltoedge[i];
					ar & r.localportal[i];
				}
			}
		}

		template<class Archive>
		void serialize(Archive & ar, ColourScheme & cs, const unsigned int version)
		{
			int i;
			for (i=0;i<4;i++)
			{
				ar & cs.outline[i];
				ar & cs.oddroomunvisited[i];
				ar & cs.evenroomunvisited[i];
				ar & cs.oddroomvisited[i];
				ar & cs.evenroomvisited[i];
				ar & cs.background[i];
			}

			ar & cs.edgetransparency;
			ar & cs.roomtransparency;
			ar & cs.entitytransparency;
			ar & cs.noplayerdup;

		}


	}
}


BOOST_CLASS_VERSION(LevelState, 1)
BOOST_CLASS_VERSION(LevelParams, 2)
BOOST_CLASS_VERSION(EntityData, 1)
BOOST_CLASS_VERSION(Room, 2)


void writelevel(const char* path, const LevelState& level)
{
    std::ofstream ofs(path);
	{
		boost::archive::text_oarchive oa(ofs);
		oa << level;
	}
	ofs.close();

}


void readlevel(const char* path, LevelState& level)
{
    {
        // create and open an archive for input
        std::ifstream ifs(path);
        boost::archive::text_iarchive ia(ifs);
        // read class state from archive
        ia >> level;
        // archive and stream closed when destructors are called
        ifs.close();
    }
}

void blankLevel(LevelState& level)
{

	level.introtext="custom level";
	level.playerpos.x=0;
	level.playerpos.y=0;
	level.playerpos.dir=0;
	level.playerpos.room=0;
	level.playerpos.par=1;
	level.playerpos.scale=0.2;

	level.levelparams.exploreon=false;
	level.levelparams.roomstoexplore=0;
	level.levelparams.paritymatters=false;
	level.levelparams.scalematters=false;
	level.levelparams.scaleandparitymatter=false;
	level.levelparams.enterroom=false;
	level.levelparams.goalroom=-1;
	level.levelparams.goalroomparity=-1;
	level.levelparams.goalroomscale=0;
	level.levelparams.mirrorexit=false;
	level.levelparams.timeron=false;
	level.levelparams.timer=0;
	level.levelparams.music=0;

	level.colourscheme.outline[0]=1;
	level.colourscheme.outline[1]=1;
	level.colourscheme.outline[2]=1;
	level.colourscheme.outline[3]=1;

	level.colourscheme.oddroomunvisited[0]=0.5;
	level.colourscheme.oddroomunvisited[1]=0.5;
	level.colourscheme.oddroomunvisited[2]=0.6;
	level.colourscheme.oddroomunvisited[3]=1;

	level.colourscheme.evenroomunvisited[0]=0.5;
	level.colourscheme.evenroomunvisited[1]=0.6;
	level.colourscheme.evenroomunvisited[2]=0.5;
	level.colourscheme.evenroomunvisited[3]=1;

	level.colourscheme.oddroomvisited[0]=0.6;
	level.colourscheme.oddroomvisited[1]=0.5;
	level.colourscheme.oddroomvisited[2]=0.6;
	level.colourscheme.oddroomvisited[3]=1;

	level.colourscheme.evenroomvisited[0]=0.6;
	level.colourscheme.evenroomvisited[1]=0.6;
	level.colourscheme.evenroomvisited[2]=0.5;
	level.colourscheme.evenroomvisited[3]=1;

	level.colourscheme.background[0]=0;
	level.colourscheme.background[1]=0;
	level.colourscheme.background[2]=0;
	level.colourscheme.background[3]=1;

	level.colourscheme.edgetransparency=true;
	level.colourscheme.roomtransparency=true;
	level.colourscheme.entitytransparency=true;
	level.colourscheme.noplayerdup=false;

	resizethings(DEFAULTROOMCOUNT,true);

	level.entitydata.curEntityPositions.resize(DEFAULTROOMCOUNT);

	level.entitydata.curEntityPositions[0].type=e_EmptyRoom;

	level.rooms[0].shape=0;

	for (int i=0;i<SHAPESIZE;i++)
	{
		level.rooms[0].portal[i]=0;
	}


	buildRoomRatios();
	countPortals();
	buildOutlines();
}
