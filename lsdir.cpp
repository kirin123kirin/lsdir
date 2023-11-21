#include <filesystem>
#include "argparser.hpp"

namespace fs = std::filesystem;

#define B 1
#define KB 1024
#define MB 1024 * 1024
#define GB 1024 * 1024 * 1024
#define MAX_PATH 260

static wchar_t DATETIME_FORMAT[] = L"%Y/%m/%d_%H:%M:%S";
static wchar_t DEFAULT_SEPARATOR[] = L"\t";
static wchar_t DEFAULT_DISPLAYORDER[] = L"psmbdf";

#define b2s(bytes, off) ((bytes[off + 1] << 8) | bytes[off])
#define b2i(bytes, off) ((bytes[off + 3] << 24) | (bytes[off + 2] << 16) | (bytes[off + 1] << 8) | bytes[off])
static const int shof = 0x4c;
static const unsigned char clsid[] = {0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46};
static const int flocflg_ofof = 0x08;
static const int bn_ofof = 0x10;
static const int local_mask = 0x01;
static const int remote_mask = 0x02;

struct FileInfo {
    const fs::path f;
    struct _stat s;
    FileInfo(const fs::path& _f) : f(_f) {
        _wstat(f.c_str(), &s);
    }

    void print_permission() {
        unsigned int m = s.st_mode;
        if (_S_IFDIR & m)
            printf("%s", "DIR");
        else {
            auto ext = f.extension().wstring();
            if(ext.size() == 4 && ext == L".lnk")
                printf("%s", "LINK");
            else
                printf("%s", "FILE");
        }
    }
    void print_username() { printf("%c", '?'); }
    void print_groupname() { printf("%c", '?'); }
    void print_atime(wchar_t* date_format = DATETIME_FORMAT) { datetimestr(s.st_atime, date_format); }
    void print_mtime(wchar_t* date_format = DATETIME_FORMAT) { datetimestr(s.st_mtime, date_format); }
    void print_ctime(wchar_t* date_format = DATETIME_FORMAT) { datetimestr(s.st_ctime, date_format); }
    void print_size(int _unit) { wprintf(L"%lu", s.st_size / _unit); }
    void print_dirname() { wprintf(L"%s", f.parent_path().c_str()); }
    void print_basename() { wprintf(L"%s", f.filename().c_str()); }
    void print_fullpath() { wprintf(L"%s", f.c_str()); }
    void print_symlink_target() { 
        unsigned char* buf = NULL;
        fpos_t flen = 0;
        FILE* fp = NULL;

        if(_wfopen_s(&fp, f.c_str(), L"rb") == 0) {
            if(fseek(fp, 0L, SEEK_END) == 0) {
                if(fgetpos(fp, &flen) == 0) {
                    if(fseek(fp, 0L, 0) == 0) {
                        buf = new unsigned char[flen + 1];
                        fread_s(buf, flen + 1, 1, flen, fp);
                    }
                }
            }
            fclose(fp);
        }

        if(buf == NULL)
            return;

        if(b2i(buf, 0) == shof && memcmp(buf + 4, clsid, sizeof(clsid)) == 0) {
            int slen = b2s(buf, shof) + 2;
            int lnkstart = shof + slen;
            int floc_flg = b2i(buf, lnkstart + flocflg_ofof);
            if(floc_flg & (local_mask | remote_mask)) {
                if(floc_flg & local_mask) {
                    int bn_of = b2i(buf, lnkstart + bn_ofof) + lnkstart;
                    printf("%s", buf + bn_of);
                } else {
                    for(int i = flen - 5; i > 3; --i) {
                        if(buf[i] == 0x00 && buf[i + 1] == 0x5C && buf[i + 2] == 0x00 && buf[i + 3] == 0x5C && buf[i + 4] == 0x00) {
                            printf("%s", buf + i + 1);
                            break;
                        }
                    }
                }
            }
        }
        delete[] buf;
    }
    void print_dirs(wchar_t* sep = DEFAULT_SEPARATOR) {
        const auto& ff = f.parent_path();
        const wchar_t* p = ff.c_str();
        if((p[0] == L'\\' && p[1] == L'\\') || (p[0] == L'/' && p[1] == L'/')) {
            wprintf(L"%c", p[0]);
            wprintf(L"%c", p[1]);
            p += 2;
        } else if(p[1] == ':' && ((p[0] > 64 && p[0] < 91) || (p[0] > 97 && p[0] < 122))) {
            wprintf(L"%c", p[0]);
            wprintf(L"%c", p[1]);
            p += 2;
        }
        for(; *p; ++p) {
            if(*p == L'\\' || *p == L'/')
                wprintf(L"%s", sep);
            else
                wprintf(L"%c", *p);
        }
    }

    private:
    int datetimestr(time_t t, wchar_t* date_format = DATETIME_FORMAT) {
        struct tm tmp;
        if(localtime_s(&tmp, &t))
            return -1;
        wchar_t outstr[128] = {0};
        int ret = -1;

        ret = wcsftime(outstr, sizeof(outstr), date_format, &tmp);
        if(ret == 0)
            return -1;  //@stderr?
        wprintf(L"%s", outstr);
        return 0;
    }

};

int wmain(int argc, wchar_t* argv[]) {
    setlocale(LC_ALL, "");
    std::locale::global(std::locale(""));

    auto ap = ArgParser(L"指定したフォルダのパスの下にあるファイル・ディレクトリの一覧出力するプログラムです.\n", argc, argv);

    bool header = true;
    ap.add(L"-n", L"--noheader", &header, L"ヘッダを出力しない\n");

    bool disp_dirs = false;
    ap.add(L"-D", L"--directory", &disp_dirs, L"ディレクトリの詳細情報を出力する(デフォルトは出力しない)\n");

    bool disp_files = true;
    ap.add(L"-F", L"--file", &disp_files, L"ファイルの詳細情報を出力する(デフォルトは出力する)\n");

    int maxdepth = INT_MAX;
    ap.add(L"-x", L"--maxdepth", &maxdepth, L"再帰的に一覧表示する場合の最大何階層まで出力するかどうか (デフォルト無制限)\n");

    wchar_t* sep = DEFAULT_SEPARATOR;
    ap.add(L"-s", L"--sep", &sep, L"詳細情報の区切り文字、デフォルトはタブ文字\n");

    bool follow_symlink = false;
    ap.add(L"-l", L"--follow-symlink", &follow_symlink, L"リンク先パスを表示する。(注)少し処理が遅くなる");

    int unit = KB;
    wchar_t u = 'k';
    ap.add(L"-u", L"--sizeunit", &u, L"ファイルサイズの表示単位\n"
                  L"                          b : バイト\n"
                  L"                          k : キロバイト(デフォルト)\n"
                  L"                          m : メガバイト\n"
                  L"                          g : ギガバイト\n");

    wchar_t* display_order = DEFAULT_DISPLAYORDER;
    ap.add(L"-d", L"--display", &display_order,
                L"select display target categories. (default : display output all category)\n"
                L"   p : permission       (ex. -rw-------)\n"
                L"   u : owner user name  (ex. root)\n"
                L"   g : owner group name (ex. root)\n"
                L"   s : file size        (ex. 123456)\n"
                L"   a : atime            (ex. 2022/02/05 10:00:00)\n"
                L"   m : mtime            (ex. 2022/02/05 10:00:00)\n"
                L"   c : ctime            (ex. 2022/02/05 10:00:00)\n"
                L"   d : dirname          (ex. /root/.ssh)\n"
                L"   b : filename          (ex. known_hosts)\n"
                L"   f : fullpath         (ex. /root/.ssh/known_hosts)\n"
                L"  Example: --display-category psmbdf\n"
                L"  Output-> -rw-------  root root 123456 2022/02/05 10:00:00 /root/.ssh/known_hosts\n");

    wchar_t* format = DATETIME_FORMAT;
    ap.add(L"-f", L"--format-time", &format,
                L"output timeformat string. (default %Y/%m/%d_%H:%M:%S)\n"
                L"see format definition.\n"
                L"https://www.cplusplus.com/reference/ctime/strftime/\n");
    ap.parse();

    switch(u) {
        case 'b':
        case 'B':
            unit = B;
            break;
        case 'k':
        case 'K':
            break;
        case 'm':
        case 'M':
            unit = MB;
            break;

        case 'g':
        case 'G':
            unit = GB;
            break;
        default:
            std::wcerr << L"sizeunitの指定値が不明です。-hでヘルプを参照して正しい値を指定してください" << std::endl;
            return 1;
    }

    if(header) {
        const wchar_t* p = display_order;
        int i = 0;
        while(*p) {
            if(i != 0)
                wprintf(L"%s", sep);
            if(*p == 'a')
                wprintf(L"%s", L"アクセス日時");
            else if(*p == 'b')
                wprintf(L"%s", L"ファイル名");
            else if(*p == 'c')
                wprintf(L"%s", L"作成日時");
            else if(*p == 'd')
                wprintf(L"%s", L"親フォルダ名");
            else if(*p == 'f')
                wprintf(L"%s", L"フルパス");
            else if(*p == 'g')
                wprintf(L"%s", L"グループ名");
            else if(*p == 'm')
                wprintf(L"%s", L"更新日時");
            else if(*p == 'p')
                wprintf(L"%s", L"種類");
            else if(*p == 's'){
                wprintf(L"%s", L"サイズ");
                if(unit == B)
                    wprintf(L"%s", L"(B)");
                else if(unit == KB)
                    wprintf(L"%s", L"(KB)");
                else if(unit == MB)
                    wprintf(L"%s", L"(MB)");
                else if(unit == GB)
                    wprintf(L"%s", L"(GB)");
            }
            else if(*p == 'u')
                wprintf(L"%s", L"ユーザ名");
            ++p, ++i;
        }
        if (follow_symlink)
            wprintf(L"%s", L"リンク先パス");

        wprintf(L"%s", sep);
        wprintf(L"%s", L"DIRS");
        wprintf(L"%s", L"\n");
    }

    if(ap.positional_argv.size() == 0)
        ap.positional_argv.push_back(L".");

    for(auto a : ap.positional_argv) {
        for(auto entry = fs::recursive_directory_iterator(fs::absolute(a), fs::directory_options::skip_permission_denied), last = fs::recursive_directory_iterator(); entry != last; ++entry) {
            if(entry->is_directory()){
                if(entry.depth() > maxdepth)
                    break;
                if(!disp_dirs)
                    continue;
            }
            auto pth = entry->path();
            auto ep = pth.filename().wstring();

            if(ep[0] == L'~' && ep[1] == L'$')
                continue;

            FileInfo fp(entry->path());
            const wchar_t* p = display_order;
            int i = 0;
            while(*p) {
                if(i != 0)
                    wprintf(L"%s", sep);
                if(*p == 'a')
                    fp.print_atime(format);
                else if(*p == 'b')
                    fp.print_basename();
                else if(*p == 'c')
                    fp.print_ctime(format);
                else if(*p == 'd')
                    fp.print_dirname();
                else if(*p == 'f')
                    fp.print_fullpath();
                else if(*p == 'g')
                    fp.print_groupname();
                else if(*p == 'm')
                    fp.print_mtime(format);
                else if(*p == 'p')
                    fp.print_permission();
                else if(*p == 's')
                    fp.print_size(unit);
                else if(*p == 'u')
                    fp.print_username();
                ++p, ++i;
            }
            if (follow_symlink){
                wprintf(L"%s", sep);
                fp.print_symlink_target();
            }
            wprintf(L"%s", sep);
            fp.print_dirs(sep);
            wprintf(L"\n");
        }
    }
    return 0;
}
