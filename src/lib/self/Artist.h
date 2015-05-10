// last modified 

#pragma once

#include <string>
#include <unordered_map>

using std::string;
using std::pair;
using std::unordered_map;


class Artist
{
    public:
        explicit Artist (const string& artist_name, const string& artist_id);
        virtual ~Artist ();
        
        bool isThere (const string& album_name) const;
        string getAlbumId (const string& album_name);
        const unordered_map<string, string>& getAllAlbumsNameAndId (void) const;
        
        bool isInitOk (void) const;
        
    private:
        bool b_init_ok_;
        const string artist_name_;
        const string artist_id_;
        unordered_map<string, string> album_id_map_;
};

