// last modified 

#pragma once

#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;


class Song 
{
    public:
        struct RateAndFormatAndSize
               {
                   string rate;
                   string size; // 字节为单位
                   string format;
               };

    public:
        Song ( const string& bduss,
               const string& artist_name,
               const string& album_name,
               const string& song_name,
               const string& song_id,
               const string& quality,
               bool b_accept_any_quality = true );
        virtual ~Song ();
        
        bool isAddedBefore (void) const;
        
        bool isThereQuality (const string& quality) const;
        
        bool getRateAndFormatAndSize (const string& quality, string& rate, string& size, string& format);
        bool getFinalRateAndFormatAndSize (string& rate, string& size, string& format);
        
        string makeDownloadApi (const string& quality);
        
        bool download (const string& path, const string& quality, const unsigned timeout);
        
        const string& getArtistName (void) const;
        const string& getAlbumName (void) const;
        const string& getSongName (void) const;
        
        const string& getFinalDownloadUrl (void) const;
        
        bool isInitOk (void) const;

    private:
        string parseFinalDownloadUrl_ (const string& quality);
        
        bool addToFavorite_ (void);
        bool delFromFavorite_ (void) const;


    private:
        bool b_init_ok_;
        const string bduss_;
        const string artist_name_;
        const string album_name_;
        const string song_name_;
        const string song_id_;
        bool b_added_by_me_; // 用户已经收藏的歌曲，不要改变其状态。该数据成员表示该歌曲是否由 yosong 添加进收藏的
        string final_download_url_;
        string final_quality_;
        unordered_map<string, RateAndFormatAndSize> quality_rateandformatandsize_map_;

    private:
        Song (const Song& song); // 禁止拷贝构造函数
};

