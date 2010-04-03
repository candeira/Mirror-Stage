/*
 *  filesys.hpp
 *  Mirror Stage
 *
 *  Created by increpare on 21/12/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

/*
maybe leave this as a pure hpp file

 ////
 episodelist
 episodecount
 levellist
 levelcount

 [refresh]
//
 load chapter
 new chapter (with one empty level)
 [delete chapter?...nah...]

 //
 in-editor options
 new episode
 save [need to display file-name somewhere?]
 save as******NEEDS DIALOG******** to display list of current episode numbers, along with option to create new one (directly consequent to the others, I guess...maybe have them all display in a list, with the last/new chapter being a different colour, with a different tooltip?)
 quit to menu (should go to editor menu, though (same with escape))

 //need music settings as well? (maybe one track per chapter? (one (optional) music.ogg file in each chapter folder) or is that too little?)

*/

#include <vector>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/config.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/fstream.hpp>


struct EpisodeDat {
	string name;
	int num_episodes;
};

vector<EpisodeDat> episodeList;
vector<EpisodeDat> customEpisodeList;

// comparison, not case sensitive.
bool compare_nocase (EpisodeDat first, EpisodeDat second)
{
	unsigned int i=0;
	while ( (i<first.name.length()) && (i<second.name.length()) )
	{
		if (tolower(first.name[i])<tolower(second.name[i])) return true;
		else if (tolower(first.name[i])>tolower(second.name[i])) return false;
		++i;
	}
	if (first.name.length()<second.name.length()) return true;
	else return false;
}


namespace bf = boost::filesystem;

void getEpisodeData()
{
	{
		episodeList.clear();
		bf::path p("chapters");
		bf::directory_iterator dir_iter(p), dir_end;

		for(;dir_iter != dir_end; ++dir_iter)
		{

			if( bf::is_directory((*dir_iter)) )
			{
				EpisodeDat ed;
				ed.name=(*dir_iter).leaf();
				ed.num_episodes=0;
				bf::path p("chapters/"+(*dir_iter).leaf());
				bf::directory_iterator sub_dir_iter(p), sub_dir_end;
				for(;sub_dir_iter != sub_dir_end; ++sub_dir_iter)
				{
					if (bf::extension(*sub_dir_iter)==".dat" && (sub_dir_iter->leaf()[0]!='.'))
						ed.num_episodes++;
				}
				episodeList.push_back(ed);
			}

		}
		sort(episodeList.begin(),episodeList.end(),compare_nocase);

	}
	{
		customEpisodeList.clear();
		bf::path p("custom");
		bf::directory_iterator dir_iter(p), dir_end;
		for(;dir_iter != dir_end; ++dir_iter)
		{
			if( bf::is_directory((*dir_iter)) )
			{
				EpisodeDat ed;
				ed.name=(*dir_iter).leaf();
				ed.num_episodes=0;
				bf::path p("custom/"+(*dir_iter).leaf());
				bf::directory_iterator sub_dir_iter(p), sub_dir_end;
				for(;sub_dir_iter != sub_dir_end; ++sub_dir_iter)
				{
					if (bf::extension(*sub_dir_iter)==".dat"&& (sub_dir_iter->leaf()[0]!='.'))
						ed.num_episodes++;
				}
				customEpisodeList.push_back(ed);
			}
		}


		sort(customEpisodeList.begin(),customEpisodeList.end(),compare_nocase);
	}


}

void deleteDir(string s)
{
	boost::filesystem::remove_all(s);
}

void renameFile(string s,string t)
{
	boost::filesystem::rename(s,t);
}
