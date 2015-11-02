// last modified 

#include "Pass.h"
#include "../helper/Misc.h"
#include "../helper/RichTxt.h"
#include <iostream>
#include <cstdlib>


using std::cin;
using std::cerr;
using std::endl;
using std::cout;
using std::make_pair;



Pass::Pass (const string& username, const string& password)
{
    // 不用可以设置 UA 伪装成手机，否则会导致页面 302 错误
    static const string user_agent("");

    // 获取 BAIDUID 的 cookie 项
    // >>>>>>>>>>>>>>>>>>>>>>>>>

    static const string baidu_wap_url("http://wap.baidu.com/");
    Webpage baidu_webpage( baidu_wap_url,
                           "",
                           "",
                           4,
                           2,
                           2,
                           user_agent );
    if (!baidu_webpage.isLoaded()) {
        cerr << "ERROR! fail to load " << baidu_wap_url << endl;
        exit(EXIT_FAILURE);
    }

    const vector<string>& cookie_items_list = baidu_webpage.getCookies();
    for (const auto& e : cookie_items_list) {
        static const string baiduid_item_key("BAIDUID");
        const string& cookie_item = e;
        const size_t baiduid_pos = cookie_item.find(baiduid_item_key);
        if (string::npos != baiduid_pos) {
            cookies_item_baiduid_ = baiduid_item_key +
                                    "=" +
                                    cookie_item.substr(baiduid_pos + baiduid_item_key.length() + 1);
            break; 
        }
    }

    // <<<<<<<<<<<<<<<<<<<<<<<<<


    // 获取 BDUSS 的 cookie 项。可能出现验证码。
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    vector<string> cookie_items_list_bduss;


    static const string baidu_login_url("http://wappass.baidu.com/passport/login");
    vector<pair<string, string>> post_sections_list = { make_pair("username", username),
                                                        make_pair("password", password),
                                                        make_pair("submit", "%E7%99%BB%E5%BD%95") };
    Webpage baidu_login_webpage( baidu_login_url,
                                 "",
                                 "",
                                 16,
                                 2,
                                 2,
                                 user_agent,
                                 cookies_item_baiduid_,
                                 post_sections_list );
    if (!baidu_login_webpage.isLoaded()) {
        cerr << "ERROR! fail to load " << baidu_login_url << endl;
        exit(EXIT_FAILURE);
    }

    // 是否需要输入验证码
    const string& baidu_login_webpage_txt = baidu_login_webpage.getTxt();
    static const string& verifycode_url_begin_keyword(R"(<img class=verifyCode-img src=")");
    static const string& verifycode_url_end_keyword(R"(")");
    const pair<string, size_t>& verifycode_pair = fetchStringBetweenKeywords( baidu_login_webpage_txt,
                                                                              verifycode_url_begin_keyword,
                                                                              verifycode_url_end_keyword );
    const string& verifycode_url = verifycode_pair.first; 
    string verifycode;
    if (!verifycode_url.empty()) {
        cout << RichTxt::bold_on
             << "WARNING! you have to input the verify code from (DON'T REFRESH)"
             << RichTxt::bold_off
             << " " << verifycode_url << ": "
             << std::flush;
        if (!(cin >> verifycode)) {
            cerr << "ERROR! fail to input verify code. " << endl;
            exit(EXIT_FAILURE);
        }
    }

    // 用验证码重新登录
    if (!verifycode.empty()) {
        const size_t vcodestr_pos = verifycode_url.find("captchaservice");
        const string& vcodestr = verifycode_url.substr(vcodestr_pos);
        
        // 重新构建提交至服务端的数据
        vector<pair<string, string>> post_sections_with_verifycode_list = { make_pair("username", username),
                                                                            make_pair("password", password),
                                                                            make_pair("verifycode", verifycode),
                                                                            make_pair("submit", "%E7%99%BB%E5%BD%95"),
                                                                            make_pair("vcodestr", vcodestr) };
        
        Webpage baidu_login_with_verifycode_webpage( baidu_login_url,
                                                     "",
                                                     "",
                                                     16,
                                                     2,
                                                     2,
                                                     user_agent,
                                                     cookies_item_baiduid_,
                                                     post_sections_with_verifycode_list );
        if (!baidu_login_with_verifycode_webpage.isLoaded()) {
            cerr << "ERROR! fail to load " << baidu_login_url << endl;
            exit(EXIT_FAILURE);
        }
        
        cookie_items_list_bduss = baidu_login_with_verifycode_webpage.getCookies();
    } else {
        cookie_items_list_bduss = baidu_login_webpage.getCookies();
    }

    // 提取 cookies 中的 BDUSS
    for (const auto& e : cookie_items_list_bduss) {
        static const string bduss_item_key("BDUSS");
        const string& cookie_item = e;
        const size_t bduss_pos = cookie_item.find(bduss_item_key);
        if (string::npos != bduss_pos) {
            cookies_item_bduss_ = bduss_item_key +
                                  "=" +
                                  cookie_item.substr(bduss_pos + bduss_item_key.length() + 1);
            break; 
        }
    }
    
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
}

Pass::~Pass ()
{
    ;
}

const string&
Pass::getBaiduid (void)
{
    return(cookies_item_baiduid_);
}

const string&
Pass::getBduss (void)
{
    return(cookies_item_bduss_);
}

