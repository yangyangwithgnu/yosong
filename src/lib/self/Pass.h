// last modified 

#pragma once

#include "../helper/Webpage.h"
#include "../helper/Misc.h"
#include <string>

using std::string;


class Pass 
{
    public:
        Pass (const string& username, const string& password);
        virtual ~Pass ();
        
        const string& getBaiduid (void);
        const string& getBduss (void);
        
    private:
        string cookies_item_baiduid_;
        string cookies_item_bduss_;
};

