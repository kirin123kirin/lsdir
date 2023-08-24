#include <iostream>
#include <string>
#include <windows.h>

void ListFiles(const std::wstring& path)
{
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW((path + L"\\*").c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (wcscmp(findData.cFileName, L".") != 0 && wcscmp(findData.cFileName, L"..") != 0)
                {
                    ListFiles(path + L"\\" + findData.cFileName);
                }
            }
            else
            {
                const auto fileSize = findData.nFileSizeLow;
                const auto fileTime = findData.ftLastWriteTime;
                const auto fileHandle = CreateFileW((path + L"\\" + findData.cFileName).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (fileHandle != INVALID_HANDLE_VALUE)
                {
                    FILETIME creationTime;
                    FILETIME lastAccessTime;
                    GetFileTime(fileHandle, &creationTime, &lastAccessTime, &fileTime);
                    CloseHandle(fileHandle);
                }
                SYSTEMTIME st;
                FileTimeToSystemTime(&fileTime, &st);
                std::wcout << path + L"\\" + findData.cFileName << L" " << fileSize << L" bytes " << st.wYear << L"/" << st.wMonth << L"/" << st.wDay << L" " << st.wHour << L":" << st.wMinute << L":" << st.wSecond << std::endl;
            }
        } while (FindNextFileW(hFind, &findData));
        FindClose(hFind);
    }
}

int main()
{
    const auto path = LR"(\\server\share\path)";
    ListFiles(path);
    return 0;
}
