// last modified 

#include "Song.h"
#include "../helper/Webpage.h"
#include "../helper/Misc.h"
#include "../3rd/json11/json11.hpp"
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>


using std::cerr;
using std::endl;
using namespace json11;
using std::make_pair;


static bool
parseAllQuality ( unordered_map<string, Song::RateAndFormatAndSize>& quality_rateandformatandsize_map,
                  const string& bduss,
                  const string& song_id )
{
    quality_rateandformatandsize_map.clear();

    // 获取该歌曲不同品质详情的接口为：
    // http://yinyueyun.baidu.com/data/cloud/download?songIds=736491
    const string quality_api = "http://yinyueyun.baidu.com/data/cloud/download?songIds=" + song_id;
    Webpage quality_webpage( quality_api,
                             "",
                             "",
                             16,
                             4,
                             2,
                             "Mozilla/5.0 (X11; Linux i686; rv:30.0) Gecko/20100101 Firefox/30.0",
                             bduss );
    if (!quality_webpage.isLoaded()) {
        cerr << "WARNING! fail to load " << quality_api << endl;
        return(false);
    }

    const string& quality_webpage_txt = quality_webpage.getTxt();


    // 解析返回的 JSON 格式数据
    // >>>>>>>>>>>>>>>>>>>>>>>>
    // 典型格式：
    /*
     * { 
     *     "query":{"songIds":"32144909"},
     *             "errorCode":0,
     *     "data":{
     *  "data" :
     *     {
     *         "flac" : "" ,
     * 
     *         "128":  {
     *                 "rate": 128,
     *                 "size": 4389023,
     *                 "songId": 32144909,
     *                 "link" :  null,
     *                 "format" : "mp3"
     *             } ,
     * 
     *         "192" :  "" ,
     * 
     *         "320" : {
     *                  "rate": 320,
     *                  "size": 10967520,
     *                  "songId": 32144909,
     *                  "link" :  null,
     *                 "format" : "mp3"
     *              } ,
     *              
     *         "original" :  ""     }
     * }
     * 
     * }
     */

    string json_err_msg;
    const auto json_quality = Json::parse(quality_webpage_txt, json_err_msg);
    if (!json_err_msg.empty()) {
        cerr << "WARNING! fail to parse the quality list JSON. because "
             << json_err_msg
             << endl;
        return(false);
    }


    // 分析各种品质的码率信息。若无匹配项时，Json::dump() 返回 "null"，
    // 而 Json::string_value() 返回空字符串。
    static const string null_str("null");
    string rate, size, format;

    // flac 码率
    rate = json_quality["data"]["data"]["flac"]["rate"].dump();
    size = json_quality["data"]["data"]["flac"]["size"].dump(); 
    format = json_quality["data"]["data"]["flac"]["format"].string_value();
    if ( (null_str != rate) &&
         (null_str != size) &&
         !format.empty() ) {
        quality_rateandformatandsize_map["flac"] = Song::RateAndFormatAndSize {rate, size, format};
    }
    
    // 128 码率
    rate = json_quality["data"]["data"]["128"]["rate"].dump();
    size = json_quality["data"]["data"]["128"]["size"].dump(); 
    format = json_quality["data"]["data"]["128"]["format"].string_value();
    if ( (null_str != rate) &&
         (null_str != size) &&
         !format.empty() ) {
        quality_rateandformatandsize_map["128"] = Song::RateAndFormatAndSize {rate, size, format};
    }

    // 192 码率
    rate = json_quality["data"]["data"]["192"]["rate"].dump();
    size = json_quality["data"]["data"]["192"]["size"].dump(); 
    format = json_quality["data"]["data"]["192"]["format"].string_value();
    if ( (null_str != rate) &&
         (null_str != size) &&
         !format.empty() ) {
        quality_rateandformatandsize_map["192"] = Song::RateAndFormatAndSize {rate, size, format};
    }

    // 320 码率
    rate = json_quality["data"]["data"]["320"]["rate"].dump();
    size = json_quality["data"]["data"]["320"]["size"].dump(); 
    format = json_quality["data"]["data"]["320"]["format"].string_value();
    if ( (null_str != rate) &&
         (null_str != size) &&
         !format.empty() ) {
        quality_rateandformatandsize_map["320"] = Song::RateAndFormatAndSize {rate, size, format};
    }

    // <<<<<<<<<<<<<<<<<<<<<<<<


    return(quality_rateandformatandsize_map.size());    
}

Song::Song ( const string& bduss,
             const string& artist_name,
             const string& album_name,
             const string& song_name,
             const string& song_id,
             const string& quality,
             bool b_accept_any_quality )
    : b_init_ok_(false),
      bduss_(bduss),
      artist_name_(artist_name),
      album_name_(album_name),
      song_name_(song_name),
      song_id_(song_id),
      b_added_by_me_(true),
      final_quality_("")
{
    if (!addToFavorite_()) {
        cerr << R"(WARNING! fail to add ")" 
             << artist_name << R"(" - <<)" 
             << album_name << R"(>> - ")"
             << song_name << R"(" to favorite. )" << endl;
        return;
    }

    if (!parseAllQuality(quality_rateandformatandsize_map_, bduss_, song_id_)) {
        //cerr << " WARNING! fail to parse quality list from songid "
             //<< song_id_ << " ";
        return;
    }

    if (b_accept_any_quality) {
        // 一首歌曲不见得四种品质都齐全，如果用户指定的品质不存在，yosong 会按一定优先级查找其他存
        // 在的品质：
        //   0）优先级规则：先解析用户指定品质，若不存在则依次考虑 320、192、128、flac 等品质。为什
        //   么不优先下 flac 的呢？因为劳资的车载音响不支持 flac 格式； (ง •̀_•́)ง
        //   1）同时，保存的最终下载地址可能不再是对应用户指定品质类型的，所以，需要将最终下载地址对
        //   应的品质类型保留下来，以便 get*(quality, ...) 这类提供查询功能的成员函数返回正确信息。
        //   这也是存在 getRateAndFormatAndSize() 和 getFinalRateAndFormatAndSize() 的意义。
        const vector<string> quality_list {quality, "320", "192", "128", "flac"}; // 该初始化元素顺序敏感
        
        // 解析出最终下载地址
        for (const auto& e : quality_list) {
            final_download_url_ = parseFinalDownloadUrl_(e);
            if (!final_download_url_.empty()) {
                final_quality_ = e; // 保存提取最终下载地址对应的品质类型
                break;
            }
        }
    } else {
        final_download_url_ = parseFinalDownloadUrl_(quality);
        final_quality_ = quality;
    }


    b_init_ok_ = true;
}

Song::~Song ()
{
    if (isAddedBefore()) {
        return;
    }

    // 恢复用户歌曲收藏原状态
    if (!delFromFavorite_()) {
        cerr << R"(WARNING! fail to delete ")" << song_name_ << R"(" to favorite. )" << endl;
        return;
    }
}

bool
Song::addToFavorite_ (void)
{
    // 收藏歌曲的接口为：
    // http://tingapi.ting.baidu.com/v1/restserver/ting?method=baidu.ting.favorite.addSongFavorite&format=json&from=bmpc&version=1.0.0&version_d=9.0.4.7&baiduid=&songId=1006104
    // 只需更改 songId 即可。
    const string add_song_favorite_api = "http://tingapi.ting.baidu.com/v1/restserver/ting?method=baidu.ting.favorite.addSongFavorite&format=json&from=bmpc&version=1.0.0&version_d=9.0.4.7&baiduid=&songId=" + 
                                         song_id_;

    // 通过 API 将该歌曲添加至 bduss 的收藏歌曲列表中
    Webpage add_song_favorite_webpage( add_song_favorite_api,
                                       "",
                                       "",
                                       16,
                                       4,
                                       2,
                                       "Mozilla/5.0 (X11; Linux i686; rv:30.0) Gecko/20100101 Firefox/30.0",
                                       bduss_ );
    if (!add_song_favorite_webpage.isLoaded()) {
        cerr << "WARNING! fail to load " << add_song_favorite_api << endl;
        return(false);
    }

    // 检查是否添加成功。若成功则返回 {"error_code":22000}
    const string& webpage_txt = add_song_favorite_webpage.getTxt();
    static const string ok_return_code("22000");
    if (string::npos != webpage_txt.find(ok_return_code)) {
        b_added_by_me_ = true;
        return(true);
    }

    // 检查是否重复添。若重复添加则返回 {"error_code":22322}
    static const string added_return_code("22322");
    if (string::npos != webpage_txt.find(added_return_code)) {
        b_added_by_me_ = false;
        return(true);
    }


    return(false);
}

bool
Song::delFromFavorite_ (void) const
{
    // 删除已收藏歌曲的接口为：
    //  http://tingapi.ting.baidu.com/v1/restserver/ting?method=baidu.ting.favorite.delSongFavorite&format=json&from=bmpc&version=1.0.0&version_d=9.0.4.7&baiduid=&songId=1006104
    // 只需更改 songId 即可。
    const string del_song_favorite_api = "http://tingapi.ting.baidu.com/v1/restserver/ting?method=baidu.ting.favorite.delSongFavorite&format=json&from=bmpc&version=1.0.0&version_d=9.0.4.7&baiduid=&songId=" + 
                                         song_id_;

    // 通过 API 将该歌曲从 bduss 的收藏歌曲列表中删除
    Webpage del_song_favorite_webpage( del_song_favorite_api,
                                       "",
                                       "",
                                       16,
                                       4,
                                       2,
                                       "Mozilla/5.0 (X11; Linux i686; rv:30.0) Gecko/20100101 Firefox/30.0",
                                       bduss_ );
    if (!del_song_favorite_webpage.isLoaded()) {
        cerr << "WARNING! fail to load " << del_song_favorite_api << endl;
        return(false);
    }

    // 检查是否删除成功。若成功则返回 {"error_code":22000}
    static const string ok_return_code("22000");
    const string& webpage_txt = del_song_favorite_webpage.getTxt();
    if (string::npos == webpage_txt.find(ok_return_code)) {
        return(false);
    }


    return(true);
}

bool
Song::isAddedBefore (void) const
{
    return(!b_added_by_me_);
}

bool
Song::isThereQuality (const string& quality) const
{
    string quality_lower;

    // 为便于大小写查找，统一转换为小写
    for (auto& e : quality) {
        quality_lower += (char)tolower(e);
    }


    return(quality_rateandformatandsize_map_.count(quality_lower) > 0);
}

bool
Song::getFinalRateAndFormatAndSize (string& rate, string& size, string& format)
{
    return(getRateAndFormatAndSize(final_quality_, rate, size, format));
}


bool
Song::getRateAndFormatAndSize ( const string& quality,
                                string& rate,
                                string& size,
                                string& format )
{
    if (!isThereQuality(quality)) {
        rate = size = format = "";
        return(false);
    }

    // 为便于大小写查找，统一转换为小写
    string quality_lower;
    for (auto& e : quality) {
        quality_lower += (char)tolower(e);
    }

    rate = quality_rateandformatandsize_map_[quality_lower].rate;
    size = quality_rateandformatandsize_map_[quality_lower].size;
    format = quality_rateandformatandsize_map_[quality_lower].format;


    return(true);
}

string
Song::makeDownloadApi (const string& quality)
{
    // 下载接口：
    // http://yinyueyun.baidu.com/data/cloud/downloadsongfile?songIds=736491&rate=972&format=flac
    // 变更 songIds、rate、format 三个参数即可

    string rate, format, size;
    if (!Song::getRateAndFormatAndSize(quality, rate, size, format)) {
        return("");
    }
    const string download_api = "http://yinyueyun.baidu.com/data/cloud/downloadsongfile?songIds=" + song_id_ +
                                "&rate=" + rate +
                                "&format=" + format;


    return(download_api);
}

string
Song::parseFinalDownloadUrl_ (const string& quality)
{
    const string& download_api = makeDownloadApi(quality);
    if (download_api.empty()) {
        return("");
    }

    Webpage download_webpage ( download_api,
                               "",
                               "",
                               1,
                               1,
                               1,
                               "Mozilla/5.0 (X11; Linux i686; rv:30.0) Gecko/20100101 Firefox/30.0",
                               bduss_ );
    const string& respond_header = download_webpage.getHttpHeader(download_api);
    static const string location_begin_keyword("Location: ");
    static const string location_end_keyword("\r");
    const pair<string, size_t>& location_pair = fetchStringBetweenKeywords( respond_header,
                                                                            location_begin_keyword,
                                                                            location_end_keyword );
    final_download_url_ = location_pair.first;
    if (final_download_url_.empty()) {
        return("");
    }


    return(final_download_url_);
}

bool
Song::download ( const string& path,
                 const string& quality,
                 const unsigned timeout )
{
    const string& final_download_url = getFinalDownloadUrl();
    if (final_download_url.empty()) {
        return(false);
    }

    string rate, format, size;
    if (!getRateAndFormatAndSize(quality, rate, size, format)) {
        return(false);
    }

    // 直接调用 aria2c 进行下载。命令行选项如下：
    // aria2c -x8 -k1M --split=8 --lowest-speed-limit=16K --connect-timeout=8 --dir=/root/ 
    // --out=1.mp3 --timeout=64 --quiet=true --allow-overwrite=true --max-tries=4 --retry-wait=2
    // 'http://file.zip'
    static const string aria2c_name("aria2c");
    const vector<string> argv { aria2c_name,
                                "-x2",
                                "-k1M",
                                //"--split=8",
                                "--lowest-speed-limit=16K",
                                "--connect-timeout=4",
                                string("--dir=" + path).c_str(),
                                string("--out=" + song_name_ + "." + format).c_str(),
                                string("--timeout=" + convNumToStr(timeout)).c_str(),
                                //"--quiet=true",
                                "--allow-overwrite=true",
                                "--max-tries=4",
                                "--retry-wait=2",
                                final_download_url.c_str() };
    wait_cmd(aria2c_name.c_str(), argv);

    // 粗略认为小于 1M 的文件算作下载失败的，删除之
    struct stat st {};
    const string path_with_filename = path + "/" + song_name_ + "." + format;
    stat(path_with_filename.c_str(), &st);
    if (st.st_size < 1024*1024) { // st.st_size 单位为字节
        cerr << "WARNING! downloaded file size anomal, delete it. " << endl;
        remove(path_with_filename.c_str());
        return(false);
    }


    return(true);
}

const string&
Song::getAlbumName (void) const
{
    return(album_name_);
}

const string&
Song::getArtistName (void) const
{
    return(artist_name_);
}

const string&
Song::getSongName (void) const
{
    return(song_name_);
}

bool
Song::isInitOk (void) const
{
    return(b_init_ok_);
}

const string&
Song::getFinalDownloadUrl (void) const
{
    return(final_download_url_);
}

