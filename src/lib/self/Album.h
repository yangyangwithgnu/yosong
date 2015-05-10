// last modified 

#pragma once

#include <string>
#include <unordered_map>

using std::string;
using std::pair;
using std::unordered_map;


class Album
{
    public:
        Album (const string& artist_name, const string& album_name, const string& album_id);
        virtual ~Album ();
        
        bool isThere (const string& song_name) const;
        const string& getSongId (const string& song_name);
        const unordered_map<string, string>& getAllSongsNameAndId (void) const;
        
        bool isInitOk (void) const;

    private:
        bool b_init_ok_;
        const string artist_name_;
        const string album_name_;
        //const string album_url_;
        const string album_id_;
        unordered_map<string, string> song_id_map_;
};

