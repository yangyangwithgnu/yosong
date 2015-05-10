// last modified 

#include "Artist.h"
#include "../helper/Webpage.h"
#include "../helper/Misc.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>


using std::istringstream;
using std::cerr;
using std::endl;
using std::make_pair;



// 解析出该歌手的所有专辑名及其对应 URL
static bool
parseAlbumsId ( const string& artist_id,
                unordered_map<string, string>& album_id_map )
{
    album_id_map.clear();

    // 获取指定歌手的专辑列表的接口：
    // http://music.baidu.com/data/user/getalbums?start=0&ting_uid=1202&order=time，
    // 这比访问页面 http://music.baidu.com/artist/1202 一是信息更精简、二是可以实现
    // 需要多页显示专辑列表时进行翻页的功能。

    for (unsigned i = 0; true; i += 10) {
        // 构造专辑列表接口地址。
        // 用起始专辑数量替换示例接口中的 0，用歌手 ID 替换示例中的 1202
        const string albums_list_api( "http://music.baidu.com/data/user/getalbums?start=" +
                                      convNumToStr(i) +
                                      "&ting_uid=" +
                                      artist_id +
                                      "&order=time" );
        
        // 获取专辑列表接口返回结果，以 unicode 编码的结果
        Webpage albums_list_webpage(albums_list_api);
        if (!albums_list_webpage.isLoaded()) {
            cerr << "ERROR! fail to load " << albums_list_api << endl;
            exit(EXIT_FAILURE);
        }
        
        // 专辑列表翻页是否已完。
        // 正常专辑列表页含有关键字"data-source”，翻页结束的页面不含该关键字
        if (string::npos == albums_list_webpage.getTxt().find("data-source")) {
            break;
        }
        
        // 对 unicode 转义 
        const string albums_list_info = convertUnicodeTxtToUtf8(albums_list_webpage.getTxt());
        
        // 在转义结果中搜索类似 <h4><a title="生命的现场" href="\/album\/32144906">，
        // 获取专辑名和专辑 ID
        // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
        
        size_t album_name_pos = 0;
        size_t album_id_pos = 0;
        while (true) {
            static const string album_name_begin_keyword(R"(<h4><a title=")");
            static const string album_name_end_keyword(R"(")");
            pair<string, size_t> album_name_pair = fetchStringBetweenKeywords( albums_list_info,
                                                                               album_name_begin_keyword,
                                                                               album_name_end_keyword,
                                                                               album_id_pos );
            if (album_name_pair.first.empty()) {
                break;
            }
            string album_name = unescapeHtml(album_name_pair.first);
            for (auto& e : album_name) { // 为便于大小写查找，统一转换为小写
                e = (char)tolower(e);
            }
            album_name_pos = album_name_pair.second;
            
            static const string album_id_begin_keyword(R"(href="\/album\/)");
            static const string album_id_end_keyword(R"(")");
            pair<string, size_t> album_id_pair = fetchStringBetweenKeywords( albums_list_info,
                                                                             album_id_begin_keyword,
                                                                             album_id_end_keyword,
                                                                             album_name_pos );
            if (album_id_pair.first.empty()) {
                break;
            }
            const string album_id = album_id_pair.first;
            album_id_pos = album_id_pair.second;
            
            album_id_map[album_name] = album_id;
        }
        
        // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    }


    return(album_id_map.size());
}

Artist::Artist (const string& artist_name, const string& artist_id)
    : b_init_ok_(false),
      artist_name_(artist_name),
      artist_id_(artist_id)
{
    if (!parseAlbumsId(artist_id_, album_id_map_)) {
        cerr << "WARNING! fail to parse albums list from artist id "
             << artist_id_ << endl;
        return;
    }

    b_init_ok_ = true;
}

Artist::~Artist ()
{
    ;
}

bool
Artist::isThere (const string& album_name) const
{
    string album_name_lower;

    // 为便于大小写查找，统一转换为小写
    for (auto& e : album_name) {
        album_name_lower += (char)tolower(e);
    }

    return(album_id_map_.count(album_name_lower) > 0);
}

const unordered_map<string, string>& 
Artist::getAllAlbumsNameAndId (void) const
{
    return(album_id_map_);
}

string
Artist::getAlbumId (const string& album_name)
{
    if (!isThere(album_name)) {
        return("");
    }

    // 为便于大小写查找，统一转换为小写
    string album_name_lower;
    for (auto& e : album_name) {
        album_name_lower += (char)tolower(e);
    }


    return(album_id_map_[album_name_lower]);
}

bool
Artist::isInitOk (void) const
{
    return(b_init_ok_);
}


