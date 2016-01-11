// last modified 

#include "Album.h"
#include "../helper/Webpage.h"
#include "../helper/Misc.h"
#include <iostream>
#include <string>
#include <cstdlib>


using std::istringstream;
using std::cerr;
using std::endl;
using std::make_pair;



// 解析出该专辑的所有歌曲名及其对应 ID
static bool
parseSongsId ( const string& album_url,
               unordered_map<string, string>& song_id_map )
{
    song_id_map.clear();

    // 获取歌曲列表页面
    Webpage songs_list_webpage(album_url, "", "", 16, 4);
    if (!songs_list_webpage.isLoaded()) {
        cerr << "ERROR! fail to load " << album_url << endl;
        exit(EXIT_FAILURE);
    }

    // 解析所有歌曲名及其 ID
    const string& webpage_txt = songs_list_webpage.getTxt();
    size_t song_id_pos = 0, song_name_pos = 0;
    while (true) {
        // 提取歌曲 ID
        static const string song_id_begin_keyword(R"(<span class="music-icon-hook" data-musicicon='{&quot;id&quot;:&quot;)");
        static const string song_id_end_keyword("&quot");
        pair<string, size_t> song_id_pair = fetchStringBetweenKeywords( webpage_txt,
                                                                        song_id_begin_keyword,
                                                                        song_id_end_keyword,
                                                                        song_name_pos );
        if (song_id_pair.first.empty()) {
            break;
        }
        const string song_id = song_id_pair.first;
        song_id_pos = song_id_pair.second;
        
        // 提取歌曲名
        static const string song_name_begin_keyword(R"(data-film="null">)");
        static const string song_name_end_keyword(R"(</a>)");
        pair<string, size_t> song_name_pair = fetchStringBetweenKeywords( webpage_txt,
                                                                          song_name_begin_keyword,
                                                                          song_name_end_keyword,
                                                                          song_id_pos );
        if (song_name_pair.first.empty()) {
            static const string song_name_begin_keyword2(R"(&quot;}">)");
            pair<string, size_t> song_name_pair2 = fetchStringBetweenKeywords( webpage_txt,
                                                                               song_name_begin_keyword2,
                                                                               song_name_end_keyword,
                                                                               song_id_pos );
            if (song_name_pair2.first.empty()) {
                break;
            }
            song_name_pair = song_name_pair2;
        }
        song_name_pos = song_name_pair.second;
        string song_name = song_name_pair.first;
        // 某些歌曲名中含有 "审批文号" 垃圾信息，剔除之
        static const string spam_keyword("审批文号");
        const size_t spam_pos = song_name.find(spam_keyword);
        if (string::npos != spam_pos) {
            song_name.resize(spam_pos);
        }
        
        // 保存歌曲名及其 ID
        song_id_map[song_name] = song_id;
    }

    return(song_id_map.size());
}

Album::Album (const string& artist_name, const string& album_name, const string& album_id)
    : b_init_ok_(false),
      artist_name_(artist_name),
      album_name_(album_name),
      album_id_(album_id)
{
    const string album_url = "http://music.baidu.com/album/" + album_id;
    if (!parseSongsId(album_url, song_id_map_)) {
        //cerr << "WARNING! fail to parse songs list from "
             //<< album_url << " " << endl;
        return;
    }

    b_init_ok_ = true;
}

Album::~Album ()
{
    ;
}

bool
Album::isThere (const string& song_name) const
{
    string song_name_lower;

    // 为便于大小写查找，统一转换为小写
    for (auto& e : song_name) {
        song_name_lower += (char)tolower(e);
    }

    return(song_id_map_.count(song_name_lower) > 0);
}

const string&
Album::getSongId (const string& song_name)
{
    string song_name_lower;

    // 为便于大小写查找，统一转换为小写
    for (auto& e : song_name) {
        song_name_lower += (char)tolower(e);
    }

    if (!isThere(song_name_lower)) {
        static const string null_str("");
        return(null_str);
    }

    return(song_id_map_[song_name_lower]);
}

const unordered_map<string, string>& 
Album::getAllSongsNameAndId (void) const
{
    return(song_id_map_);
}

bool
Album::isInitOk (void) const
{
    return(b_init_ok_);
}


