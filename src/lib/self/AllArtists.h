// last modified 

#pragma once

#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;


class AllArtists
{
    public:
        AllArtists ();
        virtual ~AllArtists ();
        
        bool isThere (const string& artist_name) const;
        const string& getArtistId (const string& artist_name);
        const unordered_map<string, string>& getAllArtistsNameAndId (void) const;
        
        
    private:
        unordered_map<string, string> artist_id_map_;
};

