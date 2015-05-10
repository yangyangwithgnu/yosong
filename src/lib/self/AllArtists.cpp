// last modified 

#include "AllArtists.h"
#include "../helper/Webpage.h"
#include "../helper/Misc.h"
#include <iostream>
#include <cstdlib>

using std::cerr;
using std::endl;




// 解析出所有歌手名及其对应 URL
static bool
parseArtistsId (unordered_map<string, string>& artist_id_map)
{
    artist_id_map.clear();

    static const string artist_list_url("http://music.baidu.com/artist/");
    Webpage artist_list_webpage(artist_list_url);
    if (!artist_list_webpage.isLoaded()) {
        cerr << "ERROR! fail to load " << artist_list_url << endl;
        exit(EXIT_FAILURE);
    }
    const string& artists_webpage_txt = artist_list_webpage.getTxt();

    // 只保留含歌手 ID 列表的关键部分，剔除其他冗余信息
    static const string first_artist(R"(<a href="/artist/4840" title="A Fine Frenzy" >A Fine Frenzy</a>)");
    static const string last_artist(R"(<a href="/artist/69365465" title="阚立文" >阚立文</a>)");
    pair<string, size_t> pair_tmp = fetchStringBetweenKeywords(artists_webpage_txt, first_artist, last_artist);
    if (0 == pair_tmp.second) {
        cerr << "ERROR! fail to find " << first_artist << " and " << last_artist << endl;
        exit(EXIT_FAILURE);
    }
    string artists_id_n_name_info = first_artist + pair_tmp.first + last_artist;


    // 解析歌手 ID 及其姓名
    size_t artist_url_end_pos = 0, artist_name_end_pos = 0;
    while (true) {
        // 解析歌手 ID
        static const string artist_id_begin(R"(<a href="/artist/)"), artist_id_end(R"(")");
        pair<string, size_t> pair_artist_id = fetchStringBetweenKeywords( artists_id_n_name_info,
                                                                          artist_id_begin,
                                                                          artist_id_end,
                                                                          artist_name_end_pos );
        if (0 == pair_artist_id.second) {
            break;
        }
        artist_url_end_pos = pair_artist_id.second;
        const string artist_id = pair_artist_id.first;
        
        // 解析歌手姓名
        static const string artist_name_begin(R"(title=")"), artist_name_end(R"(")");
        pair<string, size_t> pair_artist_name = fetchStringBetweenKeywords( artists_id_n_name_info,
                                                                            artist_name_begin,
                                                                            artist_name_end,
                                                                            artist_url_end_pos );
        if (0 == pair_artist_name.second) {
            break;
        }
        artist_name_end_pos = pair_artist_name.second;
        string artist_name = unescapeHtml(pair_artist_name.first);
        // 为便于大小写查找，统一转换为小写
        for (auto& e : artist_name) {
            e = (char)tolower(e);
        }
        
        artist_id_map[artist_name] = artist_id;
    }


    return(artist_id_map.size());
}


AllArtists::AllArtists ()
{
    if (!parseArtistsId(artist_id_map_)) {
        cerr << "ERROR! fail to parse artists id. " << endl;
        exit(EXIT_FAILURE);
    }
}

AllArtists::~AllArtists ()
{
    ;
}

bool
AllArtists::isThere (const string& artist_name) const
{
    // 为便于大小写查找，统一转换为小写
    string artist_name_lower;
    for (auto& e : artist_name) { 
        artist_name_lower += (char)tolower(e);
    }

    return(artist_id_map_.count(artist_name_lower) > 0);
}

const string&
AllArtists::getArtistId (const string& artist_name)
{
    if (!isThere(artist_name)) {
        static const string null_str("");
        return(null_str);
    }

    // 为便于大小写查找，统一转换为小写
    string artist_name_lower;
    for (auto& e : artist_name) { 
        artist_name_lower += (char)tolower(e);
    }

    return(artist_id_map_[artist_name_lower]);
}

const unordered_map<string, string>&
AllArtists::getAllArtistsNameAndId (void) const
{
    return(artist_id_map_);
}

