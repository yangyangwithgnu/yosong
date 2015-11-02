#include <algorithm>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <iomanip>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "lib/self/AllArtists.h"
#include "lib/self/Artist.h"
#include "lib/self/Album.h"
#include "lib/self/Pass.h"
#include "lib/self/Song.h"
#include "lib/helper/CmdlineOption.h"
#include "lib/helper/RichTxt.h"
#include "lib/helper/Time.h"


using namespace std;


static mutex g_mtx;

static const string g_softname(RichTxt::bold_on + "yosong" + RichTxt::bold_off);
static const string g_version("0.1.3");
static const string g_myemail("yangyangwithgnu@yeah.net");
static const string g_myemail_color(RichTxt::bold_on + RichTxt::foreground_green + g_myemail + RichTxt::reset_all);
static const string g_mywebspace("http://yangyangwithgnu.github.io");
static const string g_mywebspace_color(RichTxt::bold_on + RichTxt::foreground_green + g_mywebspace + RichTxt::reset_all);



static void
showHelpInfo()
{
    cout << endl
         << "  yosong 是一个百度音乐下载地址解析工具，具备几个还不错的能力：" << endl
         << "  0）绕开白金收费会员才能下载无损品质歌曲的限制；" << endl
         << "  1）绕开非大陆之外区域无法下载的限制（即便白金收费会员从官方渠道也无该功能）；" << endl
         << "  2）绕开百毒自身存在版权问题的歌曲而无法下载的限制（即便白金收费会员从官方渠道也无该功能）；" << endl
         << "  3）一键式全站歌曲下载（即便白金收费会员从官方渠道也无该功能）；" << endl
         << "  4）绕开高频下载出现验证码的限制；" << endl;

    cout << endl
         << "  yosong 不是下载工具，而是歌曲下载地址解析工具，通过 yosong 获取地址后，你可以用 aria2c、迅雷、旋风等"
            "第三方专业下载工具批量下载。" << endl;

    cout << endl
         << "  yosong 典型用法如下：" << endl
         << "    yosong --user 'yangyangwithgnu' --password 'abcd1234' --artist '伍佰' --album '爱上别人是快乐的事' '浪人情歌' --quality 'flac' --ignore-size-lower '6' --path ~/download/" << endl
         << "  这是让 yosong 解析歌手伍佰的《爱上别人是快乐的事》和《浪人情歌》两张专辑的所有歌曲下载地址，"
            "歌曲品质要 flac 无损的，最终下载地址保存至 HOME 的 download/ 子目录中。" << endl;

    cout << endl
         << "  yosong 是命令行程序，所以，命令行程序的几个常识你得了解：" << endl
         << "  0）--option 'argc'，其中，--option 称之为命令行选项，argc 为命令行参数；" << endl
         << "  1）某些命令行参数中可能含有对 shell 有特殊含义的字符（如，后台运行的 &、用于分割符的空格），为避免 shell 误解，通常，应该用英文单引号包裹命令行参数。如，--user 'yangyangwithgnu'；" << endl
         << "  2）某些命令行选项可以有多个参数，通常，每个参数单独用英文单引号包裹，参数间用空格分割。如，--album '八度空间' '范特西' '我很忙'；" << endl;

    cout << endl
         << "  yosong 支持的命令行选项如下。" << endl;

    cout << endl
         << "  --help" << endl
         << "  显示帮助信息。该选项优先级最高，出现该选项时忽略其他所有选项。" << endl
         << "  可选。" << endl;

    cout << endl
         << "  --version" << endl
         << "  显示当前版本信息。该选项优先级仅次 --help" << endl
         << "  可选。" << endl;

    cout << endl
         << "  --user" << endl
         << "  指定百度帐号。普通免费帐号即可，无需白金付费会员帐号。" << endl
         << "  必填。" << endl
         << "  单参数。" << endl
         << "  无默认值。" << endl;

    cout << endl
         << "  --password" << endl
         << "  指定百度帐号密码。" << endl
         << "  必填。" << endl
         << "  单参数。" << endl
         << "  无默认值。" << endl;

    cout << endl
         << "  --artist" << endl
         << "  指定歌手名。" << endl
         << "  必填。" << endl
         << "  单参数。" << endl
         << "  无默认值。" << endl;

    cout << endl
         << "  --album" << endl
         << "  指定专辑名。该专辑必须归属 --artist 指定歌手的，否则无法下载。若未指定该选项则默认下载 --artist 指定的歌手的所有专辑、所有歌曲。如，--album '八度空间' '范特西' '我很忙'" << endl
         << "  可选。" << endl
         << "  多参数。" << endl
         << "  无默认值。" << endl;

    cout << endl
         << "  --quality" << endl
         << "  指定歌曲品质。百毒音乐上的歌曲有四种品质：标准品质（128kbps）、高品质（192kbps）、超高品质（320kbps）、无损品质（800+kbps），yosong 依次有四种参数与之对应：128、192、320、flac。如果指定品质不存在，那么依次找 320、192、128、flac 等存在的品质，找到即下。" << endl
         << "  必填。" << endl
         << "  单参数。" << endl
         << "  默认值，320。" << endl;

    cout << endl
         << "  --ignore-size-lower" << endl
         << "  有些歌曲尺寸太小相应音质就不高，若想忽略小尺寸的歌曲，可以通过该选项指定一个以 MB 为单位的尺寸下限，凡低于该尺寸的歌曲均不下载。注意，0）该选项的参数可以指定小数；1）指定时不用带单位，如，--ignore-size-lower 6 而非 --ignore-size-lower 6M；2）如果不在乎歌曲尺寸可以将该选项指定为 0。" << endl
         << "  必填。" << endl
         << "  单参数。" << endl
         << "  默认值，6。" << endl;

    cout << endl
         << "  --path" << endl
         << "  指定歌曲最终下载地址保存路径，文件命名规则：artistname@hhmmss.txt。" << endl
         << "  必填。" << endl
         << "  单参数。" << endl
         << "  默认值，~/" << endl;

    cout << endl;
}

static void
showVersionInfo (void)
{
    cout << "yosong version " << g_version << endl
         << "email " << g_myemail << endl
         << "webspace " << g_mywebspace << endl << endl;
}

static void
showLifeStyle (void)
{
    cout << endl
         << "████████████████████████████████████████" << endl
         << "████████████████████ ♫ " << g_softname << " v" << g_version << " ♪ █" << endl
         << "█▀░░░░░▀███████▀▄▄▄▀████████████████████" << endl
         << "█░░███░░███████░███░███████████▀▄▄▀█████" << endl
         << "█░░░░░░▄███████░███░███████████░██░█████" << endl
         << "█░░▄░░▀▀███████░███░███████████░██░█████" << endl
         << "█▄▄██▄▄▄███████░███░███████████░██░█████" << endl
         << "█▀░░░░░▀███████░███░█▀▄▄▄▄▀████░██░█████" << endl
         << "█░░███░░███████░███░▀▄████░▄▄▀█░██░█████" << endl
         << "█░░███░░███████░██▀▄▄▄▀███░██░▄███░█████" << endl
         << "█▄░░░░░▄██████▀░▄▄████░███░██░████░█████" << endl
         << "██▀▀▀▀▀██████░▄███▀▀░▄░███░██░████░█████" << endl
         << "█░░▄▄▄░░████░██▀▀░▄███░██▀░██░████░█████" << endl
         << "█░░█████████░██░▀██████▄▄█▄▀▀▄████░█████" << endl
         << "█░░▀▀▀░░████░███▄▄▄███████████████░█████" << endl
         << "██▄▄▄▄▄██████░▀███████████████████░█████" << endl
         << "█░░█▀░░▄██████▄░▀█████████████████░█████" << endl
         << "█░░░░▄██████████▄░▀████████████▀▄▄██████" << endl
         << "█░░▄░░▀████████████░██████████░█████████" << endl
         << "█▄▄██▄▄▄███████████▄██████████▄█████████" << endl
         << endl;
}

// 存放最终输出信息
struct FinalInfo
{
    string album_name;
    string song_name;
    string final_download_url;
};

static void
parseSongInfo ( const string& bduss,
                const string& artist_name,
                const string& album_name,
                const string& song_name,
                const string& song_id,
                const string& quality,
                const string& ignore_size_lower,
                vector<FinalInfo>& final_info_list )
{
    Song song(bduss, artist_name, album_name, song_name, song_id, quality);
    if (!song.isInitOk()) {
        g_mtx.lock();
        cerr << RichTxt::underline_on << RichTxt::foreground_red
             << song_name
             << RichTxt::reset_all
             << "(no any quality) " << flush;
        g_mtx.unlock();
        return;
    }

    // 歌曲尺寸小于用户指定的则忽略之
    string rate, size, format;
    if (!song.getFinalRateAndFormatAndSize(rate, size, format)) {
        g_mtx.lock();
        cerr << RichTxt::underline_on << RichTxt::foreground_red
             << song_name
             << RichTxt::reset_all
             << "(no this quality) " << flush;
        g_mtx.unlock();
        return;
    }
    long double ignore_size_lower_num = strtold(ignore_size_lower.c_str(), nullptr);
    long double size_num = strtold(size.c_str(), nullptr);
    if (size_num < ignore_size_lower_num * 1024 * 1024) { // 歌曲实际尺寸单位为 byte，用户指定的最小歌曲尺寸单位为 MB
        g_mtx.lock();
        cerr << RichTxt::underline_on << RichTxt::foreground_red
             << song_name
             << RichTxt::reset_all
             << setprecision(1) << setiosflags(ios::fixed)
             << "(" << size_num / 1024 / 1024 << "MB too small) " << flush
             << resetiosflags(ios::fixed);
        g_mtx.unlock();
        return;
    }

    // 显示并保存有效歌曲最终下载地址及其他信息
    g_mtx.lock();
    cout << RichTxt::underline_on << song_name << RichTxt::underline_off << " " << flush;
    final_info_list.push_back(FinalInfo{album_name, song_name, song.getFinalDownloadUrl()});
    g_mtx.unlock();
}

int
main (int argc, char* argv[])
{
    // 解析命令行选项
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    CmdlineOption cmdline_options((unsigned)argc, argv);
    vector<string> cmdline_arguments_list;

    // --help, first high priority, ignore other options
    if (cmdline_options.hasOption("--help")) {
        showHelpInfo();
        exit(EXIT_SUCCESS);
    }

    // --version, second high priority, ignore other options
    if (cmdline_options.hasOption("--version")) {
        showVersionInfo();
        exit(EXIT_SUCCESS);
    }

    // show the life style ASCII art
    showLifeStyle();
    sleep(1);

    // --user
    cout << RichTxt::bold_on
         << "parse commandline options: " << endl
         << "==========================" << endl
         << RichTxt::bold_off;
    string username;
    cmdline_arguments_list = cmdline_options.getArgumentsList("--user");
    if (cmdline_arguments_list.empty()) {
        cerr << "ERROR! there is no --user, more info see --help. " << endl;
        exit(EXIT_FAILURE);
    }
    username = cmdline_arguments_list[0];
    cout << "  user: "
         << RichTxt::bold_on << username << RichTxt::bold_off << endl;

    // --password
    string password;
    cmdline_arguments_list = cmdline_options.getArgumentsList("--password");
    if (cmdline_arguments_list.empty()) {
        cerr << "ERROR! there is no --password, more info see --help. " << endl;
        exit(EXIT_FAILURE);
    }
    password = cmdline_arguments_list[0];
    cout << "  password: "
         << RichTxt::bold_on << password << RichTxt::bold_off << endl;

    // --artist
    string artist_name;
    cmdline_arguments_list = cmdline_options.getArgumentsList("--artist");
    if (cmdline_arguments_list.empty()) {
        cerr << "ERROR! there is no --artist, more info see --help. " << endl;
        exit(EXIT_FAILURE);
    }
    artist_name = cmdline_arguments_list[0];
    cout << "  artist: "
         << RichTxt::bold_on << artist_name << RichTxt::bold_off << endl;

    // --album
    vector<string> albums_name_list;
    cmdline_arguments_list = cmdline_options.getArgumentsList("--album");
    if (!cmdline_arguments_list.empty()) {
        albums_name_list = cmdline_arguments_list;
    }
    cout << "  albums: ";
    if (0 == albums_name_list.size()) {
        cout << "all albums";
    } else {
        for (const auto& e : albums_name_list) {
            cout << "《" << RichTxt::bold_on << e << RichTxt::bold_off << "》";
        }
    }
    cout << endl;

    // --quality
    string quality = "320";
    cmdline_arguments_list = cmdline_options.getArgumentsList("--quality");
    if (!cmdline_arguments_list.empty()) {
        quality = cmdline_arguments_list[0];
    }
    cout << "  quality: "
         << RichTxt::bold_on << quality << RichTxt::bold_off << endl;

    // --ignore-size-lower
    string ignore_size_lower("6"); // 默认小于 6M 的歌曲不下载
    cmdline_arguments_list = cmdline_options.getArgumentsList("--ignore-size-lower");
    if (!cmdline_arguments_list.empty()) {
        ignore_size_lower = cmdline_arguments_list[0];
    }
    cout << "  ignore size lower: "
         << RichTxt::bold_on << ignore_size_lower << "MB" << RichTxt::bold_off << endl;

    // --path
    // >>>>>>>>>>>>>>>>>

    string path;
    cmdline_arguments_list = cmdline_options.getArgumentsList("--path");
    if (cmdline_arguments_list.empty()) {
#ifdef CYGWIN
        const char* p_home = "C:\\";
#else
        const char* p_home = getenv("HOME");
        if (nullptr == p_home) {
            cerr << "ERROR! --path argument setting wrong! " << endl;
            exit(EXIT_FAILURE);
        }
#endif
        path = p_home;
    } else {
        path = cmdline_arguments_list[0];
    }
#ifdef CYGWIN
    // windows style path
    replace(path.begin(), path.end(), '/', '\\');
#endif

#ifndef CYGWIN
    // convert raw path to standard absolute path. To call realpath() success,
    // path must have created.
    char buffer[PATH_MAX];
    realpath(path.c_str(), buffer);
    path = buffer;
#endif

    cout << "  the path: " << RichTxt::bold_on << path << RichTxt::bold_off << endl;

    // <<<<<<<<<<<<<<<<<<

    cout << endl;
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


    cout << RichTxt::bold_on
         << "login baidu.com: " << endl
         << "================" << endl
         << RichTxt::bold_off;
    Pass baidu_pass(username, password);
    const string cookie_bduss = baidu_pass.getBduss();
    if (cookie_bduss.empty()) {
        cerr << "ERROR! fail to login baidu.com, maybe you inputed invalid verify code, or make sure your username ("
             << username << ") and password (" << password << ") is all right. "
             << endl;
        exit(EXIT_FAILURE);
    }
    cout << "done. " << endl << endl;


    cout << RichTxt::bold_on
         << "parse artists: " << endl
         << "==============" << endl
         << RichTxt::bold_off;
    AllArtists all_artists;
    if (!all_artists.isThere(artist_name)) {
        cerr << "ERROR! there is no artist named " << artist_name << endl;
        exit(EXIT_FAILURE);
    }
    const string& artist_id = all_artists.getArtistId(artist_name);
    cout << "done. " << endl << endl;


    cout << RichTxt::bold_on
         << "parse albums: " << endl
         << "=============" << endl
         << RichTxt::bold_off;
    for (auto& e : albums_name_list) { // 大小写敏感，统一为小写
        for (auto& f : e) {
            f = (char)tolower(f);
        }
    }
    unordered_map<string, string> albums_id_map;
    Artist artist(artist_name, artist_id);
    if (!artist.isInitOk()) {
        exit(EXIT_FAILURE);
    }
    if (albums_name_list.empty()) { // 若用户未指定专辑名，则选择所有专辑
        albums_id_map = artist.getAllAlbumsNameAndId();
    } else {                        // 若用户指定了专辑名，则应先确认这些专辑是否归属指定歌手，再选择
        for (const auto& e : albums_name_list) {
            if (!artist.isThere(e)) {
                cerr << "ERROR! there is no album named " << e << endl;
                exit(EXIT_FAILURE);
            }
            
            albums_id_map[e] = artist.getAlbumId(e);
        }
    }
    for (const auto& e : albums_id_map) {
        cout << "《" << e.first << "》" << flush;
    }
    cout << endl << endl;


    cout << RichTxt::bold_on
         << "parse songs (!! maybe very slowly !!): " << endl
         << "======================================" << endl
         << RichTxt::bold_off;
    vector<FinalInfo> final_info_list;
    for (const auto& e : albums_id_map) {
        const string& album_name = e.first;
        const string& album_id = e.second;
        
        Album album(artist_name, album_name, album_id);
        if (!album.isInitOk()) {
            cerr << RichTxt::foreground_red
                 << "《" << album_name << "》" 
                 << RichTxt::reset_all
                 << "(WARNING! fail to parse songs list)" << endl;
            continue;
        }
        cout << "《" << album_name << "》: " << flush;
        
        const unordered_map<string, string>& songs_id_map = album.getAllSongsNameAndId();
        vector<thread> threads_list;
        for (const auto& f : songs_id_map) {
            const string& song_name = f.first;
            const string& song_id = f.second;
            threads_list.push_back(thread( &parseSongInfo,
                                           ref(cookie_bduss),
                                           ref(artist_name),
                                           ref(album_name),
                                           ref(song_name),
                                           ref(song_id),
                                           ref(quality),
                                           ref(ignore_size_lower),
                                           ref(final_info_list) ));
        }
        for (auto& f : threads_list) {
            if (f.joinable()) {
                f.join();
            }
        }
        
        cout << endl;
    }
    cout << endl;


    // 下载歌曲或者输出歌曲下载地址
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>

    cout << RichTxt::bold_on
         << "output final download addr of songs: " << endl
         << "====================================" << endl
         << RichTxt::bold_off;

    // 生成存放最终下载地址的文件名
    Time current_time;
    const string filename = artist_name + "@" +
                            current_time.getHour(2) + current_time.getMinute(2) + current_time.getSecond(2) +
                            ".txt";
    // 创建文件
    const string path_with_filename = path + "/" + filename;
    ofstream ofs(path_with_filename);
    if (!ofs) {
        cerr << "ERROR! fail to create file " << path_with_filename << endl;
        exit(EXIT_FAILURE);
    }
    ofs << "♪♫•*¨*•.¸¸♥ ¸¸.•*¨*•♫•*¨*•.¸¸•♫♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♫•¸¸.•*¨*•♫•*¨*•.¸¸♥ ¸¸.•*¨*•♫♪" << endl;
    ofs << "♪♫•*¨* this file created by _yosong_, more info " << g_mywebspace << " *¨*•♫♪" << endl;
    ofs << "♪♫•*¨*•.¸¸♥ ¸¸.•*¨*•♫•*¨*•.¸¸•♫♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♪♫•¸¸.•*¨*•♫•*¨*•.¸¸♥ ¸¸.•*¨*•♫♪" << endl;
    ofs << endl << endl;

    // 输出最终下载地址
    string last_album_name; 
    for (auto& e : final_info_list) {
        const string& album_name = e.album_name;
        const string& song_name = e.song_name;
        const string& final_download_url = e.final_download_url;
        
        if (last_album_name != album_name) {
            cout << "---------------------" << endl;
            cout << "《" << album_name << "》: " << endl;
            ofs << "---------------------" << endl;
            ofs << "《" << album_name << "》: " << endl;
            last_album_name = album_name;
        }
        
        cout << RichTxt::underline_on << song_name << RichTxt::underline_off << endl;
        ofs << song_name << endl;
        if (final_download_url.empty()) {
            cerr << RichTxt::foreground_red << "WARNING! there is no any quality info for this song. ";
            ofs << "WARNING! there is no any quality info for this song. ";
        } else {
            cout << RichTxt::foreground_green << final_download_url;
            ofs << final_download_url;
        }
        cout << RichTxt::reset_all << endl;
        ofs << endl;
    }
    cout << endl;
    ofs.close();

    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<

    cout << RichTxt::bold_on
         << "your file: " << path_with_filename << endl
         << "♪♫•*¨*•.¸¸♥ ¸¸.•*¨*•♫•*¨*•.¸¸•♫♪ enjoy music! ♪♫•¸¸.•*¨*•♫•*¨*•.¸¸♥ ¸¸.•*¨*•♫♪" << endl
         << RichTxt::bold_off;


    cout << endl;
    return(EXIT_SUCCESS);
}

