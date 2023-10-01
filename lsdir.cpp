#include <locale.h>
#include <windows.h>
#include <iostream>
#include <string>
#include "argparser.hpp"

#define B 1
#define KB 1024
#define MB 1024 * 1024
#define GB 1024 * 1024 * 1024

bool noheader = false;
int maxdepth = INT_MAX;
bool directory_printing = false;
std::wstring pathsep = L"\\";
wchar_t sep[] = L"\t";
DWORD size_unit = 1024;
const std::wstring OMIT = L"*\\?";

#define bytes2short(bytes, off) ((bytes[off + 1] << 8) | bytes[off])
#define bytes2int(bytes, off) ((bytes[off + 3] << 24) | (bytes[off + 2] << 16) | (bytes[off + 1] << 8) | bytes[off])

std::wstring getlinkpath(const wchar_t* path) {
    char link[MAX_PATH] = {0};
    wchar_t ret[MAX_PATH] = {0};
    LPBYTE buff = NULL;
    fpos_t fsize = 0;
    FILE* fp = NULL;

    const int shell_offset = 0x4c;
    const byte clsid[] = {0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46};
    const int file_location_info_flag_offset_offset = 0x08;
    const int basename_offset_offset = 0x10;
    const int is_local_mask = 0x01;
    const int is_remote_mask = 0x02;

    if(_wfopen_s(&fp, path, L"rb") == 0) {
        if(fseek(fp, 0L, SEEK_END) == 0) {
            if(fgetpos(fp, &fsize) == 0) {
                if(fseek(fp, 0L, 0) == 0) {
                    buff = new BYTE[fsize + 1];
                    fread_s(buff, fsize + 1, 1, fsize, fp);
                }
            }
        }
        fclose(fp);
    }

    if(buff) {
        if(bytes2int(buff, 0) == shell_offset) {
            if(memcmp(buff + 4, clsid, sizeof(clsid)) == 0) {
                int shell_len = bytes2short(buff, shell_offset) + 2;  // if the shell settings are present, skip them
                int linkinfo_start = shell_offset + shell_len;        // get to the file settings
                int basename_offset = bytes2int(buff, linkinfo_start + basename_offset_offset) +
                                      linkinfo_start;  // get the local volume and local system values
                int file_location_info_flag = bytes2int(buff, linkinfo_start + file_location_info_flag_offset_offset);
                if(file_location_info_flag & (is_local_mask | is_remote_mask)) {
                    if(file_location_info_flag & is_local_mask) {  // XXX - should check first
                        if(strcpy_s(link, (const char*)(buff + basename_offset)) == 0) {
                            std::size_t _ = 0;
                            mbstowcs_s(&_, ret, link, MAX_PATH);
                        }
                    } else {
                        int i = fsize - 5;
                        for(; i > 3; --i) {
                            if(buff[i] == 0x00 && buff[i + 1] == 0x5C && buff[i + 2] == 0x00 && buff[i + 3] == 0x5C &&
                               buff[i + 4] == 0x00)
                                break;
                        }
                        wcscpy_s(ret, (wchar_t*)(buff + i + 1));
                    }
                }
            }
        }
        delete[] buff;
    }
    return ret;
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

    if(hFind == INVALID_HANDLE_VALUE)
        return;

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

int wmain(int argc, wchar_t* argv[]) {
    std::wcout.imbue(std::locale("Japanese", std::locale::ctype));
    setlocale(LC_ALL, "");

    auto ap = ArgParser<wchar_t>(L"指定したフォルダのパスの下にあるファイル・ディレクトリの一覧出力するプログラムです.\n", argc, argv);
    ap.add(L"-n", L"--noheader", &noheader, L"ヘッダを出力しない\n");
    ap.add(L"-d", L"--directory", &directory_printing, L"ディレクトリの詳細情報も出力する\n", ITEM::REQUIRED);
    ap.add(L"-x", L"--maxdepth", &maxdepth, L"再帰的に一覧表示する場合の最大何階層まで出力するかどうか (デフォルト無制限)\n");
    ap.add(L"-s", L"--sep", &sep, L"詳細情報の区切り文字、デフォルトはタブ文字\n");
    wchar_t u = L'k';
    ap.add(L"-u", L"--sizeunit", &u, L"ファイルサイズの表示単位\n"
                  L"                          b : バイト\n"
                  L"                          k : キロバイト(デフォルト)\n"
                  L"                          m : メガバイト\n"
                  L"                          g : ギガバイト\n");

    ap.parse();

    switch(u) {
        case L'b':
        case L'B':
            size_unit = B;
            break;
        case L'k':
        case L'K':
            break;
        case L'm':
        case L'M':
            size_unit = MB;
            break;

        case L'g':
        case L'G':
            size_unit = GB;
            break;
        default:
            std::wcerr << L"sizeunitの指定値が不明です。-hでヘルプを参照して正しい値を指定してください" << std::endl;
            return 1;
    }

    if(noheader == false)
        print_header();

    if(ap.positional_argv.size() == 0) {
        ListFiles(abs_path(L".\\*"));
        return 0;
    }

    for(auto* a : ap.positional_argv) {
        if(a[0] == L'.' && a[1] == 0)
            ListFiles(abs_path(L".\\*"));
        else if(a[0] == L'.' && a[1] == L'.' && a[2] == 0)
            ListFiles(abs_path(L"..\\*"));
        else
            ListFiles(abs_path(a) + L"\\*");
    }

    // const auto path = LR"(\\192.168.1.103\download)";
    // ListFiles(path);
    return 0;
}
