#include <locale.h>
#include <windows.h>
#include <iostream>
#include <string>
#include "argparser.hpp"

#define B 1
#define KB 1024
#define MB 1024 * 1024
#define GB 1024 * 1024 * 1024

bool header = true;
int maxdepth = INT_MAX;
bool directory_printing = false;
std::wstring pathsep = L"\\";
wchar_t* sep = const_cast<wchar_t*>(L"\t");
DWORD size_unit = 1024;
const std::wstring OMIT = L"*\\?";

std::wstring getlinkpath(const wchar_t* shortcutPath) {
    char targetPath[MAX_PATH] = {0};
    wchar_t ret[MAX_PATH] = {0};
    FILE* fp;

    fp = _wfopen(shortcutPath, L"rb");
    if(fp == NULL)
        return std::wstring();

    const char pref[] = "\x00\x00\x00\x11\x00\x00\x00\x03\x00\x00\x00\x6C\x05\x8D\xA0\x10\x00\x00\x00\x00";
    const char* p = pref;
    const char* end = pref + sizeof(pref) / sizeof(pref[0]);
    char c;
    while((c = fgetc(fp)) != EOF) {
        if(*p == c) {
            for(p = pref + 1; p != end; ++p) {
                if((c = fgetc(fp)) == EOF)
                    break;
                if(*p != c)
                    break;
            }
        }
        if(p == end - 1) {
            for(int i = 0; i < MAX_PATH && c && c != EOF; ++i) {
                targetPath[i] = c;
                c = fgetc(fp);
            }
            break;
        }
        p = pref;
    }
    fclose(fp);
    if((mbstowcs(ret, targetPath, MAX_PATH)) != -1)
        return ret;
    return std::wstring();
}

std::wstring abs_path(const std::wstring& partialPath) {
    wchar_t full[_MAX_PATH];
    if(_wfullpath(full, partialPath.c_str(), _MAX_PATH) != NULL)
        return std::wstring(full);
    else
        return nullptr;
}

void print_3devide_number(const DWORD val) {
    int sz = val / size_unit;
    std::string str = std::to_string(sz);
    std::string dst = "";
    while(str.length() > 3) {
        dst = "," + str.substr(str.length() - 3, 3) + dst;
        str = str.substr(0, str.length() - 3);
    }
    std::cout << str << dst;
    if(size_unit > 1024) {
        int f = (int)((((double)val / (double)size_unit) - sz) * 100);
        std::cout << '.' << f;
    }
}

void print_filetime(const SYSTEMTIME& st) {
    printf("%d/%02d/%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

void print_header() {
    std::wcout << L"種類名";
    std::wcout << sep << L"ファイル名";
    std::wcout << sep << L"サイズ";
    switch(size_unit) {
        case B:
            std::wcout << L"(B)";
            break;
        case KB:
            std::wcout << L"(KB)";
            break;
        case MB:
            std::wcout << L"(MB)";
            break;
        case GB:
            std::wcout << L"(GB)";
            break;
        default:
            break;
    }
    std::wcout << sep << L"最終更新日時";
    // std::wcout << sep << L"アクセス日時";
    // std::wcout << sep << L"作成日時";
    std::wcout << sep << L"親ディレクトリパス";
    std::wcout << sep << L"フルパス";
    std::wcout << sep << L"リンク先(ショートカットファイルの場合)";
    std::wcout << std::endl;
}

void print_compute(const WIN32_FIND_DATAW& findData, const std::wstring& path) {
    std::wstring fullpath = path + pathsep + findData.cFileName;

    const auto fileSize = findData.nFileSizeLow;
    const auto fileTime = findData.ftLastWriteTime;
    const auto fileHandle =
        CreateFileW(fullpath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(fileHandle != INVALID_HANDLE_VALUE) {
        FILETIME creationTime;
        FILETIME lastAccessTime;
        GetFileTime(fileHandle, &creationTime, &lastAccessTime, (LPFILETIME)&fileTime);
        CloseHandle(fileHandle);
    }

    SYSTEMTIME st_update;
    SYSTEMTIME st_access;
    SYSTEMTIME st_create;
    FileTimeToSystemTime(&fileTime, &st_update);
    FileTimeToSystemTime(&fileTime, &st_access);
    FileTimeToSystemTime(&fileTime, &st_create);

    if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        std::cout << "DIR";
    else if(fullpath.find(L".lnk", fullpath.size() - 4) != std::string::npos)
        std::cout << "LINK";
    else
        std::cout << "FILE";

    std::wcout << sep;
    wprintf(L"%ls", findData.cFileName);
    std::wcout << sep;
    print_3devide_number(fileSize);

    std::wcout << sep;
    print_filetime(st_update);
    // std::wcout << sep;
    // print_filetime(st_access);
    // std::wcout << sep;
    // print_filetime(st_create);
    std::wcout << sep;
    wprintf(L"%ls", path.c_str());
    std::wcout << sep;
    wprintf(L"%ls", fullpath.c_str());
    if(fullpath.find(L".lnk", fullpath.size() - 4) != std::string::npos) {
        std::wcout << sep;
        std::wcout << getlinkpath(fullpath.c_str());
    }
    std::wcout << std::endl;
}

void ListFiles(const std::wstring& path, int depth = 0) {
    if(depth > maxdepth)
        return;
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(path.c_str(), &findData);

    size_t end = path.find_last_not_of(OMIT);
    std::wstring p = (end == std::string::npos) ? path : path.substr(0, end + 1);

    if(hFind != INVALID_HANDLE_VALUE) {
        do {
            if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if(wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0) {
                    if(directory_printing)
                        print_compute(findData, p);
                    ListFiles(p + pathsep + findData.cFileName + pathsep + L"*", depth + 1);
                }
            } else {
                print_compute(findData, p);
            }
        } while(FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }
}

int wmain(int argc, wchar_t* argv[]) {
    std::wcout.imbue(std::locale("Japanese", std::locale::ctype));
    setlocale(LC_ALL, "");

    auto args = ArgumentParser(
        L"lsdir", L" : 指定したフォルダパス配下にあるファイルやディレクトリの一覧を詳細情報とともに表示します.\n");

    args.append(&header, L'n', L"noheader", L"ヘッダを出力しない\n", 0);
    args.append(&directory_printing, L'd', L"directory", L"ディレクトリの詳細情報も出力する\n", 0);
    args.append(&maxdepth, L'x', L"maxdepth",
                L"再帰的に一覧表示する場合の最大何階層まで出力するかどうか (デフォルト無制限)\n", 0);
    args.append(&sep, L's', L"sep", L"詳細情報の区切り文字、デフォルトはタブ文字\n", 0);
    char u = 'k';
    args.append(&u, L'u', L"sizeunit",
                L"ファイルサイズの表示単位\n"
                L"   b : バイト\n"
                L"   k : キロバイト(デフォルト)\n"
                L"   m : メガバイト\n"
                L"   g : ギガバイト\n",
                0);

    args.parse(argc, argv);
    if(args.parse(argc, argv) == -1)
        return 1;

    unescape(sep);

    switch(u) {
        case 'b':
        case 'B':
            size_unit = B;
            break;
        case 'k':
        case 'K':
            break;
        case 'm':
        case 'M':
            size_unit = MB;
            break;

        case 'g':
        case 'G':
            size_unit = GB;
            break;
        default:
            std::wcerr << L"sizeunitの指定値が不明です。-hでヘルプを参照して正しい値を指定してください" << std::endl;
            return 1;
    }

    if(header)
        print_header();

    std::size_t i = 0;
    for(std::size_t end = args.size(); i < end; ++i) {
        auto a = args[i];
        if(a && ++i) {
            if(a[0] == L'.' && a[1] == 0)
                ListFiles(abs_path(L".\\*"));
            else if(a[0] == L'.' && a[1] == L'.' && a[2] == 0)
                ListFiles(abs_path(L"..\\*"));
            else
                ListFiles(abs_path(a) + L"\\*");
        }
    }
    if(i == 0)
        ListFiles(abs_path(L".\\*"));

    // const auto path = LR"(\\192.168.1.103\download)";
    // ListFiles(path);
    return 0;
}
