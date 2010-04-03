#include <boost/filesystem/fstream.hpp>
#include <vector>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

namespace bf = boost::filesystem;
using namespace boost;

struct SaveData
{
    bool completed[7][17];
    bool canplay[7][17];
};

int episodes_in_chapter[7]={9,9,17,9,7,6,9};

SaveData savedata;

namespace boost {
	namespace serialization {

		template<class Archive>
		void serialize(Archive & ar, SaveData & sd, const unsigned int version)
		{

			for (int i=0;i<7;i++)
                for (int j=0;j<17;j++)
                    ar & sd.completed[i][j];
		}
	}
}

void calccanplays()
{
    //basic structure

    //optional levels
    // 5-2
    // 6-4

        for (int i=0;i<7;i++)
        for (int j=0;j<17;j++)
        {
            if (savedata.completed[i][j])
                savedata.canplay[i][j]=true;
            else if ( j>0 && savedata.completed[i][j-1])
                savedata.canplay[i][j]=true;
            else if (j==0 && i>0 && savedata.completed[i-1][episodes_in_chapter[i-1]-1])
            {
                savedata.canplay[i][j]=true;
            }
            else if (i==4 && j==3 && savedata.canplay[4][2])
                savedata.canplay[i][j]=true;
            else if (i==5 && j==5 && savedata.canplay[5][4])
                savedata.canplay[i][j]=true;
            else
                savedata.canplay[i][j]=false;
        }

        savedata.canplay[0][0]=true;
    //can do the sweep here
}

void loadsavedata()
{
    for (int i=0;i<7;i++)
        for (int j=0;j<17;j++)
        {
            savedata.completed[i][j]=false;
        }

    bf::path p("settings/savedata.bin");
    if (bf::exists(p))
    {
        bf::ifstream ifs(p,ios::in|ios::binary);
        boost::archive::binary_iarchive ia(ifs);
        // read class state from archive
        ia >> savedata;
        ifs.close();
    }
    calccanplays();
}


void savesavedata()
{
        bf::path p("settings/savedata.bin");

    bf::ofstream ofs(p, ios::out|ios::binary);
	{
		boost::archive::binary_oarchive oa(ofs);
		oa << savedata;
	}
	ofs.close();

	calccanplays();
}
