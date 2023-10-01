/* ***
MIT License

Copyright (c) 2018 Antoine Beauchamp
Copyright (c) 2023 kirin123kirin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


*** */

#define PRODUCT_VERSION "0.1.0"

// #include <stdint.h>
#include <string>
#include <vector>
// #include <stdio.h>
// #include <iostream>
#include <assert.h>
#include <direct.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#ifdef _WIN32
#include <Windows.h>  //for GetShortPathName()
#define stat _stat
#endif
// #include <memory.h>
// #include <string.h>  //for strlen()
#include <algorithm>  //for std::transform()
#include <sstream>    //for std::stringstream
// #include <sys/stat.h>
// #include <sys/types.h>
// #include <cstdlib>  //for random
// #include <ctime>    //for random
#include <iomanip>  //for std::hex, std::setfill, std::setw

namespace stringfunc {

///< summary>
/// Replace an occurance of a string by another.
///</summary>
///< param name="iString">The given string that need to be searched.</param>
///< param name="iOldValue">The old value to replace.</param>
///< param name="iNewValue">The new value to replace.</param>
///< return>Returns the number of token that was replaced.<return>
int strReplace(std::string& iString, const char* iOldValue, const char* iNewValue) {
    std::string tmpOldValue = iOldValue;
    std::string tmpNewValue = iNewValue;

    int numOccurance = 0;

    if(tmpOldValue.size() > 0) {
        size_t startPos = 0;
        size_t findPos = std::string::npos;
        do {
            findPos = iString.find(tmpOldValue, startPos);
            if(findPos != std::string::npos) {
                iString.replace(findPos, tmpOldValue.length(), tmpNewValue);
                startPos = findPos + tmpNewValue.length();
                numOccurance++;
            }
        } while(findPos != -1);
    }
    return numOccurance;
}

};  // namespace stringfunc

namespace filesystem {

///< summary>
/// Returns the size of the given file path in bytes.
///</summary>
///< param name="f">An valid file path.</param>
///< return>Returns the size of the given file path in bytes.<return>
uint32_t getFileSize(const char* iPath) {
    if(iPath == NULL || iPath[0] == '\0')
        return 0;

    FILE* f;
    errno_t error = fopen_s(&f, iPath, "rb");
    if(error != 0)
        return 0;
    long initPos = ftell(f);
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, initPos, SEEK_SET);
    fclose(f);
    return size;
}

///< summary>
/// Determine if a file exists.
///</summary>
///< param name="iPath">An valid file path.</param>
///< return>Returns true when the file exists. Returns false otherwise.<return>
bool fileExists(const char* iPath) {
    if(iPath == NULL || iPath[0] == '\0')
        return false;

    FILE* f;
    errno_t error = fopen_s(&f, iPath, "rb");
    if(error != 0)
        return false;
    fclose(f);
    return true;
}

///< summary>
/// Determine if a folder exists.
///</summary>
///< param name="iPath">An valid folder path.</param>
///< return>Returns true when the folder exists. Returns false otherwise.<return>
bool folderExists(const char* iPath) {
    if(iPath == NULL || iPath[0] == '\0')
        return false;

    const char* localFolder = _getcwd(NULL, 0);
    bool success = (_chdir(iPath) == 0);
    if(success && localFolder)
        _chdir(localFolder);
    return success;
}

///< summary>
/// Splits a path into a folder and a filename.
///</summary>
///< param name="iPath">The input path to split.</param>
///< param name="oFolder">The output folder of the given path.</param>
///< param name="oFile">The output file of the given path.</param>
void splitPath(const std::string& iPath, std::string& oFolder, std::string& oFilename) {
    oFolder = "";
    oFilename = "";

    std::size_t offset = iPath.find_last_of("/\\");
    if(offset != std::string::npos) {
        // found
        oFolder = iPath.substr(0, offset);
        oFilename = iPath.substr(offset + 1);
    } else {
        oFilename = iPath;
    }
}

///< summary>
/// Splits a path into each element.
///</summary>
///< param name="iPath">The input path to split.</param>
///< param name="oElements">The output list which contains all path elements.</param>
void splitPath(const std::string& iPath, std::vector<std::string>& oElements) {
    oElements.clear();
    std::string s = iPath;
    std::string accumulator;
    unsigned int i = 0;
    unsigned int end = s.size();

    if((s[0] == '\\' && s[1] == '\\') || (s[0] == '/' && s[1] == '/')) {
        accumulator += s[0];
        accumulator += s[1];
        s =- 2;
        i = 2;
    }
    for(; i < end; ++i) {
        const char& c = s[i];
        if((c == '/' || c == '\\')) {
            if(accumulator.size() > 0) {
                oElements.push_back(accumulator);
                accumulator = "";
            }
        } else
            accumulator += c;
    }
    if(accumulator.size() > 0) {
        oElements.push_back(accumulator);
        accumulator = "";
    }
}

///< summary>
/// Returns the extension of a file.
///</summary>
///< param name="iPath">The valid path to a file.</param>
///< return>Returns the extension of a file.<return>
std::string getFileExtention(const std::string& iPath) {
    // extract filename from path to prevent
    // reading a folder's extension
    std::string folder;
    std::string filename;
    splitPath(iPath, folder, filename);

    std::string extension;
    std::size_t offset = filename.find_last_of(".");
    if(offset != std::string::npos) {
        // found
        // name = filename.substr(0,offset);
        extension = filename.substr(offset + 1);
    }

    return extension;
}

///< summary>
/// Convert a long file path to the short path form (8.3 format).
/// If the system does not support automatic conversion, an estimated
/// version is returned.
///</summary>
///< param name="iPath">The input path to convert.</param>
///< return>Returns the short path form of the given path.<return>
std::string getShortPathFormEstimation(const std::string& iPath) {
    std::string shortPath;

    std::vector<std::string> pathElements;
    splitPath(iPath, pathElements);
    for(size_t i = 0; i < pathElements.size(); i++) {
        const std::string& element = pathElements[i];
        if(element.size() > 12 || element.find(' ') != std::string::npos) {
            std::string element83 = element;
            std::string ext = getFileExtention(element);
            stringfunc::strReplace(element83, (std::string(".") + ext).c_str(), "");  // remove extension from filename
            stringfunc::strReplace(ext, " ", "");                                     // remove spaces in extension
            ext = ext.substr(0, 3);                                                   // truncate file extension
            stringfunc::strReplace(element83, " ", "");                               // remove spaces
            element83 = element83.substr(0, 6);                                       // truncate file name
            element83.append("~1");
            if(!ext.empty()) {
                element83.append(".");
                element83.append(ext);
            }

            // uppercase everything
            std::transform(element83.begin(), element83.end(), element83.begin(), ::toupper);

            // add to shortPath
            if(!shortPath.empty())
                shortPath.append("\\");
            shortPath.append(element83);
        } else {
            if(!shortPath.empty())
                shortPath.append("\\");
            shortPath.append(element);
        }
    }

    return shortPath;
}

std::string getShortPathForm(const std::string& iPath) {
#ifdef WIN32
    if(fileExists(iPath.c_str()) || folderExists(iPath.c_str())) {
        // file must exist to use WIN32 api
        std::string shortPath;

        // First obtain the size needed by passing NULL and 0.
        long length = GetShortPathNameA(iPath.c_str(), NULL, 0);
        if(length == 0)
            return "";

        // Dynamically allocate the correct size
        // (terminating null char was included in length)
        char* buffer = new char[length];

        // Now simply call again using same long path.
        length = GetShortPathNameA(iPath.c_str(), buffer, length);
        if(length == 0)
            return "";

        shortPath = buffer;

        delete[] buffer;

        return shortPath;

    } else {
        return getShortPathFormEstimation(iPath);
    }
#elif UNIX
    // no such thing as short path form in unix
    return getShortPathFormEstimation(iPath);
#endif
}

};  // namespace filesystem

//----------------------------------------------------------------------------------------------------------------------------------------
// Defines, Pre-declarations & typedefs
//----------------------------------------------------------------------------------------------------------------------------------------
enum LNK_HOTKEY_CODES {
    LNK_HK_NONE = 0x00,
    LNK_HK_0 = 0x30,
    LNK_HK_1 = 0x31,
    LNK_HK_2 = 0x32,
    LNK_HK_3 = 0x33,
    LNK_HK_4 = 0x34,
    LNK_HK_5 = 0x35,
    LNK_HK_6 = 0x36,
    LNK_HK_7 = 0x37,
    LNK_HK_8 = 0x38,
    LNK_HK_9 = 0x39,
    LNK_HK_A = 0x41,
    LNK_HK_B = 0x42,
    LNK_HK_C = 0x43,
    LNK_HK_D = 0x44,
    LNK_HK_E = 0x45,
    LNK_HK_F = 0x46,
    LNK_HK_G = 0x47,
    LNK_HK_H = 0x48,
    LNK_HK_I = 0x49,
    LNK_HK_J = 0x4a,
    LNK_HK_K = 0x4b,
    LNK_HK_L = 0x4c,
    LNK_HK_M = 0x4d,
    LNK_HK_N = 0x4e,
    LNK_HK_O = 0x4f,
    LNK_HK_P = 0x50,
    LNK_HK_Q = 0x51,
    LNK_HK_R = 0x52,
    LNK_HK_S = 0x53,
    LNK_HK_T = 0x54,
    LNK_HK_U = 0x55,
    LNK_HK_V = 0x56,
    LNK_HK_W = 0x57,
    LNK_HK_X = 0x58,
    LNK_HK_Y = 0x59,
    LNK_HK_Z = 0x5a,
    LNK_HK_F1 = 0x70,
    LNK_HK_F2 = 0x71,
    LNK_HK_F3 = 0x72,
    LNK_HK_F4 = 0x73,
    LNK_HK_F5 = 0x74,
    LNK_HK_F6 = 0x75,
    LNK_HK_F7 = 0x76,
    LNK_HK_F8 = 0x77,
    LNK_HK_F9 = 0x78,
    LNK_HK_F10 = 0x79,
    LNK_HK_F11 = 0x7a,
    LNK_HK_F12 = 0x7b,
    LNK_HK_NUMLOCK = 0x90,
    LNK_HK_SCROLL = 0x91,
};
enum LNK_HOTKEY_MODIFIERS {
    LNK_HK_MOD_NONE = 0x00,
    LNK_HK_MOD_SHIFT = 0x01,
    LNK_HK_MOD_CONTROL = 0x02,
    LNK_HK_MOD_ALT = 0x04,
};
struct LNK_HOTKEY {
    uint8_t keyCode;    // LNK_HOTKEY_CODES
    uint8_t modifiers;  // LNK_HOTKEY_MODIFIERS
};
// extern const LNK_HOTKEY LNK_NO_HOTKEY;

struct LNK_ICON {
    std::string filename;
    unsigned long index;
};

struct LinkInfo {
    std::string target;
    std::string networkPath;
    std::string arguments;
    std::string description;
    std::string workingDirectory;
    LNK_ICON customIcon;
    LNK_HOTKEY hotKey;
};

const char* getVersionString() {
    return PRODUCT_VERSION;
}

typedef std::vector<std::string> StringList;

//----------------------------------------------------------------------------------------------------------------------------------------
// Structures
//----------------------------------------------------------------------------------------------------------------------------------------
// http://www.stdlib.com/art6-Link-File-Format-lnk.html
// http://www.wotsit.org/list.asp?search=lnk

#pragma pack(push)
#pragma pack(1)

// Link flags
struct LinkFlags  // 32 bits
{
    bool HasLinkTargetIDList : 1;
    bool HasLinkInfo : 1;
    bool HasName : 1;
    bool HasRelativePath : 1;
    bool HasWorkingDir : 1;
    bool HasArguments : 1;
    bool HasIconLocation : 1;
    bool reserved1 : 1;
    bool reserved2 : 8;
    bool reserved3 : 8;
    bool reserved4 : 8;
};

// Target flags
struct FileAttributesFlags  // 32 bits
{
    bool isReadOnly : 1;       // FILE_ATTRIBUTE_READONLY
    bool isHidden : 1;         // FILE_ATTRIBUTE_HIDDEN
    bool isSystemFile : 1;     // FILE_ATTRIBUTE_SYSTEM
    bool isVolumeLabel : 1;    // Reserved1, MUST be zero.
    bool isDirectory : 1;      // FILE_ATTRIBUTE_DIRECTORY
    bool isArchive : 1;        // FILE_ATTRIBUTE_ARCHIVE
    bool isEncrypted : 1;      // Reserved2, MUST be zero.
    bool isNormal : 1;         // FILE_ATTRIBUTE_NORMAL
    bool isTemporary : 1;      // FILE_ATTRIBUTE_TEMPORARY
    bool isSparseFile : 1;     // FILE_ATTRIBUTE_SPARSE_FILE
    bool hasReparsePoint : 1;  // FILE_ATTRIBUTE_REPARSE_POINT
    bool isCompressed : 1;     // FILE_ATTRIBUTE_COMPRESSED
    bool isOffline : 1;        // FILE_ATTRIBUTE_OFFLINE
    bool reserved1 : 3;        // FILE_ATTRIBUTE_NOT_CONTENT_INDEXED
    bool reserved2 : 8;        // FILE_ATTRIBUTE_ENCRYPTED
    bool reserved3 : 8;
};

typedef uint8_t LNK_CLSID[16];
static const LNK_CLSID DEFAULT_LINKCLSID = {0x01, 0x14, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46};
struct ShellLinkHeader {
    uint32_t HeaderSize;
    LNK_CLSID LinkCLSID;
    LinkFlags linkFlags;
    FileAttributesFlags FileAttributes;
    unsigned __int64 CreationTime;
    unsigned __int64 AccessTime;
    unsigned __int64 WriteTime;
    unsigned long FileSize;
    unsigned long IconIndex;
    unsigned long ShowCommand;
    LNK_HOTKEY HotKey;
    uint16_t Reserved1;
    uint32_t Reserved2;
    uint32_t Reserved3;
};

static const unsigned long LNK_LOCATION_UNKNOWN = 0;
static const unsigned long LNK_LOCATION_LOCAL = 1;
static const unsigned long LNK_LOCATION_NETWORK = 2;
// File Location Info
// This section is always present, but if bit 1 is not set in the flags value,
// then the length of this section will be zero.
// The header of this section is described below.
// 1 dword This length value includes all the assorted pathnames and other data structures. All offsets are relative to
// the start of this section. 1 dword The offset at which the basic file info structure ends. Should be 1C. 1 dword File
// available on local volume (0) or network share(1) 1 dword Offset to the local volume table. 1 dword Offset to the
// base path on the local volume. 1 dword Offset to the network volume table. 1 dword Offset to the final part of the
// pathname.
struct LNK_FILE_LOCATION_INFO {
    unsigned long length;
    unsigned long endOffset;
    unsigned long location;
    unsigned long localVolumeTableOffset;
    unsigned long basePathOffset;
    unsigned long networkVolumeTableOffset;
    unsigned long finalPathOffset;
};
static const unsigned long LNK_FILE_LOCATION_INFO_SIZE = sizeof(LNK_FILE_LOCATION_INFO);

// Type of volumes
// Code Description
// 0 Unknown
// 1 No root directory
// 2 Removable (Floppy, Zip ...)
// 3 Fixed (Hard disk)
// 4 Remote (Network drive)
// 5 CD-ROM
// 6 Ram drive
static const unsigned long LNK_VOLUME_TYPE_UNKNOWN = 0;
static const unsigned long LNK_VOLUME_TYPE_NO_ROOT_DIRECTORY = 1;
static const unsigned long LNK_VOLUME_TYPE_REMOVABLE = 2;
static const unsigned long LNK_VOLUME_TYPE_FIXED = 3;
static const unsigned long LNK_VOLUME_TYPE_REMOTE = 4;
static const unsigned long LNK_VOLUME_TYPE_CDROM = 5;
static const unsigned long LNK_VOLUME_TYPE_RAMDRIVE = 6;

// The local volume table
// 1 dword Length of this structure including the volume label string.
// 1 dword Type of volume (code below)
// 1 dword Volume serial number
// 1 dword Offset of the volume name (Always 0x10)
// ASCIZ Volume label
struct LNK_LOCAL_VOLUME_TABLE {
    unsigned long length;
    unsigned long volumeType;
    unsigned long volumeSerialNumber;
    unsigned long volumeNameOffset;
    char volumeLabel;
};
static const unsigned long LNK_LOCAL_VOLUME_TABLE_SIZE = sizeof(LNK_LOCAL_VOLUME_TABLE);

// The network volume table
// 1 dword Length of this structure
// 1 dword Always 02
// 1 dword Offset of network share name (Always 0x14)
// 1 dword Reserved 0
// 1 dword Always 0x20000
// ASCIZ Network share name
struct LNK_NETWORK_VOLUME_TABLE {
    unsigned long length;
    unsigned long reserved1;
    unsigned long networkShareNameOffset;
    unsigned long reserved2;
    unsigned long reserved3;
    char networkShareName;
};
static const unsigned long LNK_NETWORK_VOLUME_TABLE_SIZE = sizeof(LNK_NETWORK_VOLUME_TABLE);

// itemId structure used in LinkTargetIDList part of a link
struct LNK_ITEMID {
    uint16_t size;
    uint8_t type;
    uint8_t unknown1[5];
    uint32_t unknown2;
    uint16_t fileAttributes;
    const char* name83;
    unsigned char unknown07;
    unsigned char unknown08;
    unsigned char unknown09[7];
    unsigned char unknown10;
    unsigned char unknown11;
    unsigned char unknown12;
    unsigned char unknown13;
    unsigned char unknown14;
    unsigned char unknown15;
    unsigned char unknown16;
    unsigned char unknown17;
    unsigned char unknown18[4];
    const char* nameUnicode;
    unsigned char unknown19;
    unsigned char unknown20;
};
// extern const LNK_ITEMID LNK_ITEMIDFolderDefault;
// extern const LNK_ITEMID LNK_ITEMIDFileDefault;

#pragma pack(pop)

//----------------------------------------------------------------------------------------------------------------------------------------
// global classes & functions
//----------------------------------------------------------------------------------------------------------------------------------------

// namespace lnk {

class MemoryBuffer {
   public:
    MemoryBuffer(void) : mBuffer(NULL), mSize(0){};
    MemoryBuffer(unsigned long iSize) : mBuffer(NULL), mSize(0) { allocate(iSize); }
    MemoryBuffer(const MemoryBuffer& iValue) : mBuffer(NULL), mSize(0) { (*this) = iValue; }
    virtual ~MemoryBuffer(void) { clear(); }

    //----------------
    // public methods
    //----------------
    void clear() {
        if(mBuffer) {
            delete[] mBuffer;
        }
        mBuffer = NULL;
        mSize = 0;
    }
    unsigned char* getBuffer() { return mBuffer; }
    const unsigned char* getBuffer() const { return mBuffer; }
    bool allocate(unsigned long iSize) {
        clear();
        mBuffer = new unsigned char[iSize];
        if(mBuffer) {
            mSize = iSize;
            return true;
        }
        return false;
    }
    bool reallocate(unsigned long iSize) {
        unsigned char* newBuffer = new unsigned char[iSize];
        if(newBuffer) {
            // if the current memory buffer has data
            if(mBuffer) {
                // copy the content of the existing buffer to newBuffer
                memcpy(newBuffer, mBuffer, (((mSize) < (iSize)) ? (mSize) : (iSize)));
            }

            clear();
            mBuffer = newBuffer;
            mSize = iSize;
            return true;
        }
        return false;
    }
    unsigned long getSize() const { return mSize; }
    bool loadFile(const char* iFilePath) {
        // get size of file
        unsigned long size = filesystem::getFileSize(iFilePath);

        FILE* f;
        errno_t error = fopen_s(&f, iFilePath, "rb");
        if(error == 0) {
            if(allocate(size)) {
                fread(mBuffer, 1, size, f);

                fclose(f);
                return true;
            }
            fclose(f);
        }
        return false;
    }

    const MemoryBuffer& operator=(const MemoryBuffer& iValue) {
        if(allocate(iValue.mSize))
            memcpy(mBuffer, iValue.mBuffer, mSize);
        return (*this);
    }

   private:
    unsigned char* mBuffer;
    unsigned long mSize;
};

std::string toTimeString(const unsigned __int64& iTime) {
    // time stamps
    const FILETIME* utcFileTime = (const FILETIME*)&iTime;
    FILETIME localFileTime = {0};
    FileTimeToLocalFileTime(utcFileTime, &localFileTime);
    SYSTEMTIME localSystemTime = {0};
    FileTimeToSystemTime(&localFileTime, &localSystemTime);

    char buffer[1024];
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d.%03d", localSystemTime.wYear, localSystemTime.wMonth,
            localSystemTime.wDay, localSystemTime.wHour, localSystemTime.wMinute, localSystemTime.wSecond,
            localSystemTime.wMilliseconds);

    std::string value = buffer;
    return value;
}

static const LNK_HOTKEY LNK_NO_HOTKEY = {LNK_HK_NONE, LNK_HK_MOD_NONE};

template <typename T>
inline const T& readData(const unsigned char* iBuffer, unsigned long& ioOffset) {
    const T& value = *((T*)(&iBuffer[ioOffset]));
    ioOffset += sizeof(T);
    return value;
}

void readString(const unsigned char* iBuffer, unsigned long& ioOffset, std::string& oValue) {
    oValue = "";
    const unsigned short& length = readData<unsigned short>(iBuffer, ioOffset);
    size_t sz;
    char target[3] = {0};

    for(unsigned short i = 0; i < length; i++) {
        const unsigned short& tmp = readData<unsigned short>(iBuffer, ioOffset);
        if(tmp < 256) {
            char c = (char)tmp;
            oValue += c;
        } else {
            if((wcstombs_s(&sz, target, (wchar_t*)(&tmp), 2)) == 0)
                oValue += target;
            else
                oValue += (char)tmp;
        }
    }
}

std::string readUnicodeString(FILE* iFile) {
    std::string value;

    unsigned short length = 0;
    fread(&length, 1, sizeof(length), iFile);
    size_t sz;
    char target[3] = {0};

    for(unsigned short i = 0; i < length; i++) {
        unsigned short tmp = 0;
        fread(&tmp, 1, sizeof(tmp), iFile);

        if(tmp < 256) {
            char c = (char)tmp;
            value += c;
        } else {
            if((wcstombs_s(&sz, target, (wchar_t*)(&tmp), 2)) == 0)
                value += target;
            else
                value += (char)tmp;
        }
    }

    return value;
}


void saveStringUnicode(FILE* iFile, const std::string& iValue) {
    unsigned short length = (unsigned short)iValue.size();
    fwrite(&length, 1, sizeof(length), iFile);
    std::size_t sz;
    wchar_t target[MAX_PATH+1] = {0};
    if(mbstowcs_s(&sz, target, iValue.c_str(), MAX_PATH) != 0)
        return;

    for(unsigned short i = 0; i < length; i++) {
        fwrite(&target[i], 1, sizeof(wchar_t), iFile);
    }
}

void saveString(FILE* iFile, const std::string& iValue) {
    size_t length = iValue.size();
    const char* text = iValue.c_str();
    fwrite(text, 1, length, iFile);

    static const char NULLCHARACTER = '\0';
    fwrite(&NULLCHARACTER, 1, sizeof(NULLCHARACTER), iFile);
}

bool deserialize(const MemoryBuffer& iBuffer, LNK_ITEMID& oValue, std::string& oName83, std::string& oNameLong) {
    unsigned long offset = 0;
    const unsigned char* buffer = iBuffer.getBuffer();

    oValue.size = readData<unsigned short>(buffer, offset);
    oValue.type = readData<unsigned char>(buffer, offset);
    oValue.unknown1[0] = readData<unsigned char>(buffer, offset);
    oValue.unknown1[1] = readData<unsigned char>(buffer, offset);
    oValue.unknown1[2] = readData<unsigned char>(buffer, offset);
    oValue.unknown1[3] = readData<unsigned char>(buffer, offset);
    oValue.unknown1[4] = readData<unsigned char>(buffer, offset);
    oValue.unknown2 = readData<unsigned long>(buffer, offset);
    oValue.fileAttributes = readData<unsigned short>(buffer, offset);

    // name83
    oValue.name83 = NULL;
    {
        char c = readData<unsigned char>(buffer, offset);
        while(c != '\0') {
            oName83 += c;
            c = readData<unsigned char>(buffer, offset);
        }
    }

    // search for location of nameUnicode
    // nameUnicode is located at the end of the ItemID
    // following a NULL unicode character.
    const unsigned short* nameUnicodeAddress =
        (const unsigned short*)(&buffer[iBuffer.getSize() - 2]);  // last uint16_t of the ItemID
    nameUnicodeAddress--;                                         // NULL terminating character;
    nameUnicodeAddress--;                                         // last string character;
    while(*nameUnicodeAddress != 0x0000) {
        nameUnicodeAddress--;  // rewind until the beginning of the
    }
    nameUnicodeAddress++;  // move to first string character

    // move buffer up to nameUnicodeAddress
    while(nameUnicodeAddress != (const unsigned short*)(&buffer[offset])) {
        [[maybe_unused]] unsigned char c = readData<unsigned char>(buffer, offset);
    }

    // nameUnicode
    oValue.nameUnicode = NULL;
    {
        unsigned short unicodec = readData<unsigned short>(buffer, offset);
        char c = (char)unicodec;
        while(c != '\0') {
            oNameLong += c;
            unicodec = readData<unsigned short>(buffer, offset);
            c = (char)unicodec;
        }
    }

    oValue.unknown19 = readData<unsigned char>(buffer, offset);
    oValue.unknown20 = readData<unsigned char>(buffer, offset);

    bool success = (offset == iBuffer.getSize());
    assert(success == true);
    return success;
}

template <typename T>
inline bool serialize(const T& iValue, MemoryBuffer& ioBuffer) {
    unsigned long dataSize = sizeof(T);
    unsigned long oldSize = ioBuffer.getSize();
    unsigned long newSize = oldSize + dataSize;
    if(ioBuffer.reallocate(newSize)) {
        // save data
        unsigned char* buffer = ioBuffer.getBuffer();
        T* offset = (T*)&buffer[oldSize];
        *offset = iValue;

        return true;
    }
    return false;
}

inline bool serialize(const unsigned char* iData, const unsigned long& iSize, MemoryBuffer& ioBuffer) {
    unsigned long oldSize = ioBuffer.getSize();
    unsigned long newSize = oldSize + iSize;
    if(ioBuffer.reallocate(newSize)) {
        // save data
        unsigned char* buffer = ioBuffer.getBuffer();
        unsigned char* offset = &buffer[oldSize];
        for(unsigned long i = 0; i < iSize; i++) {
            offset[i] = iData[i];
        }

        return true;
    }
    return false;
}

// };  // namespace lnk

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const void* value) {
    std::stringstream out;
    if(sizeof(void*) == 4)
        out << std::setw(8) << std::setfill('0') << std::hex << value;
    else if(sizeof(void*) == 8)
        out << std::setw(16) << std::setfill('0') << std::hex << value;
    str.append(out.str());
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const std::string& value) {
    str.append(value);
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const char* value) {
    str.append(value);
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const int16_t& value) {
    std::stringstream out;
    out << value;
    str.append(out.str());
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const uint16_t& value) {
    std::stringstream out;
    out << value;
    str.append(out.str());
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const int8_t& value) {
    std::stringstream out;
    out << (int16_t)value;
    str.append(out.str());
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const uint8_t& value) {
    std::stringstream out;
    out << (uint16_t)value;
    str.append(out.str());
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const int32_t& value) {
    std::stringstream out;
    out << value;
    str.append(out.str());
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const uint32_t& value) {
    std::stringstream out;
    out << value;
    str.append(out.str());
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const int64_t& value) {
    std::stringstream out;
    out << value;
    str.append(out.str());
    return str;
}

///< summary>
/// Streams a value to an existing string.
///</summary>
///< param name="value">The value to append to the given string.</param>
///< return>Returns the given string.<return>
std::string& operator<<(std::string& str, const uint64_t& value) {
    std::stringstream out;
    out << value;
    str.append(out.str());
    return str;
}

// namespace lnk {

#pragma pack(push)
#pragma pack(1)

struct ItemIDHeader {
    uint16_t size;
    uint8_t type;
    uint8_t unknown1[9];
    uint16_t fileAttributes;
};

struct ItemIDEx {
    uint16_t size;
    uint32_t type;
};

#pragma pack(pop)

enum FILE_ATTRIBUTES { FA_NORMAL, FA_DIRECTORY };

typedef std::vector<MemoryBuffer> ItemIDList;

MemoryBuffer getLinkTargetIDList(const ItemIDList& iItemIDList) {
    // assert iItemIDList already contains TerminalID
    assert(iItemIDList.size() != 0);
    assert(iItemIDList[iItemIDList.size() - 1].getSize() == 2);

    // build LinkTargetIDList
    MemoryBuffer buffer;

    // compute total size
    uint16_t IDListSize = 0;
    for(size_t i = 0; i < iItemIDList.size(); i++) {
        const MemoryBuffer& ItemID = iItemIDList[i];
        IDListSize += (size_t)ItemID.getSize();
    }
    serialize(IDListSize, buffer);

    // serialize all ItemID
    for(size_t i = 0; i < iItemIDList.size(); i++) {
        const MemoryBuffer& ItemID = iItemIDList[i];
        serialize(ItemID.getBuffer(), ItemID.getSize(), buffer);
    }

    return buffer;
}

MemoryBuffer getTerminalItemId() {
    static const uint16_t terminalId = 0;

    MemoryBuffer buffer;
    serialize(terminalId, buffer);

    return buffer;
}

MemoryBuffer getComputerItemId() {
    static const uint8_t computer[] = {0x14, 0x00, 0x1f, 0x50, 0xe0, 0x4f, 0xd0, 0x20, 0xea, 0x3a,
                                       0x69, 0x10, 0xa2, 0xd8, 0x08, 0x00, 0x2b, 0x30, 0x30, 0x9d};

    MemoryBuffer buffer;
    serialize(computer, sizeof(computer), buffer);

    return buffer;
}

MemoryBuffer getDriveItemId(char iDriveLetter) {
    static const uint8_t drive[] = {0x19, 0x00, 0x2f, 'C',  ':',  '\\', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    MemoryBuffer buffer;
    serialize(drive, sizeof(drive), buffer);

    // fix drive letter
    uint8_t& letter = buffer.getBuffer()[3];
    letter = (uint8_t)iDriveLetter;

    return buffer;
}

MemoryBuffer getDriveItemId() {
    static const uint8_t drive[] = {0x19, 0x00, 0x2f, '\\',  '\\',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    MemoryBuffer buffer;
    serialize(drive, sizeof(drive), buffer);

    return buffer;
}

MemoryBuffer getWinXpItemIdEx(const std::string& iLongName, const FILE_ATTRIBUTES& iAttributes) {
    MemoryBuffer buffer;

    ItemIDEx header;
    header.size = 0;
    header.type = 0x00040003;  // WinXP style

    serialize(header.size, buffer);
    serialize(header.type, buffer);
    serialize((uint8_t)0xEF, buffer);  // always
    serialize((uint8_t)0xBE, buffer);  // always

    serialize((uint8_t)0x3A, buffer);  // 0x77
    serialize((uint8_t)0x3E, buffer);  // 0x35

    if(iAttributes == FA_DIRECTORY) {
        serialize((uint8_t)0x0E, buffer);  // 0x1C
    } else {
        serialize((uint8_t)0x13, buffer);  // 0x1C
    }

    serialize((uint8_t)0x6B, buffer);  // 0x37
    serialize((uint8_t)0x3A, buffer);  // 0x34
    serialize((uint8_t)0x3E, buffer);  // 0x3E

    if(iAttributes == FA_DIRECTORY) {
        serialize((uint8_t)0x0E, buffer);  // 0xE6
    } else {
        serialize((uint8_t)0x13, buffer);  // 0x1C
    }

    serialize((uint8_t)0x6B, buffer);  // 0x60
    serialize((uint8_t)0x14, buffer);
    serialize((uint8_t)0x00, buffer);
    serialize((uint8_t)0x00, buffer);
    serialize((uint8_t)0x00, buffer);

    // long name
    for(size_t i = 0; i <= iLongName.size(); i++)  // include NULL character
    {
        uint16_t c = iLongName[i];
        serialize(c, buffer);
    }

    serialize((uint8_t)0x18, buffer);  // 0x12, 0x14, 0x16, 0x18 or 0x1A
    serialize((uint8_t)0x00, buffer);  // always

    // fix size
    uint16_t* size = (uint16_t*)buffer.getBuffer();
    *size = (uint16_t)buffer.getSize();

    return buffer;
}

MemoryBuffer getFileItemId(const std::string& iShortName,
                           const std::string& iLongName,
                           const FILE_ATTRIBUTES& iAttributes) {
    // validation
    assert(iAttributes == FA_NORMAL || iAttributes == FA_DIRECTORY);

    ItemIDHeader header;
    header.size = 0;
    header.type = ((iAttributes == FA_DIRECTORY) ? 0x31 : 0x32);
    header.fileAttributes = ((iAttributes == FA_DIRECTORY) ? 0x0010 : 0x0020);

    MemoryBuffer buffer;

    serialize(header.size, buffer);
    serialize(header.type, buffer);
    serialize((uint8_t)0x00, buffer);  // unknown1
    serialize((uint8_t)0x00, buffer);  // unknown1
    serialize((uint8_t)0x00, buffer);  // unknown1
    serialize((uint8_t)0x00, buffer);  // unknown1
    serialize((uint8_t)0x00, buffer);  // unknown1
    serialize((uint8_t)0x3A, buffer);  // unknown1  0x34
    serialize((uint8_t)0x3E, buffer);  // unknown1  0x3E
    if(iAttributes == FA_DIRECTORY) {
        serialize((uint8_t)0x0E, buffer);  // unknown1  0xDD
    } else {
        serialize((uint8_t)0x13, buffer);  // unknown1  0xDD
    }
    serialize((uint8_t)0x6B, buffer);  // unknown1  0x60
    serialize(header.fileAttributes, buffer);

    // shortname
    serialize((const uint8_t*)iShortName.c_str(), iShortName.size() + 1, buffer);

    // padding ?
    serialize((uint8_t)0x00, buffer);

    // add ItemIDEx
    MemoryBuffer itemIdExBuffer = getWinXpItemIdEx(iLongName, iAttributes);
    serialize(itemIdExBuffer.getBuffer(), itemIdExBuffer.getSize(), buffer);

    // fix size
    uint16_t* size = (uint16_t*)buffer.getBuffer();
    *size = (uint16_t)buffer.getSize();

    return buffer;
}

// };  // namespace lnk

// namespace lnk {

bool isLink(const unsigned char* iBuffer, const unsigned long& iSize) {
    if(iSize > sizeof(ShellLinkHeader)) {
        unsigned long offset = 0;
        const ShellLinkHeader& header = readData<const ShellLinkHeader>(iBuffer, offset);

        if(header.HeaderSize != sizeof(ShellLinkHeader))
            return false;

        bool guidSuccess = (memcmp(header.LinkCLSID, DEFAULT_LINKCLSID, sizeof(LNK_CLSID)) == 0);
        if(!guidSuccess)
            return false;

        return true;
    }
    return false;
}

bool isLink(const MemoryBuffer& iFileContent) {
    return isLink(iFileContent.getBuffer(), iFileContent.getSize());
}

bool isLink(const char* iFilePath) {
    MemoryBuffer fileContent;
    bool loadSuccess = fileContent.loadFile(iFilePath);
    if(loadSuccess) {
        // validate signature
        bool link = isLink(fileContent);
        if(link) {
            return true;
        }
    }
    return false;
}

bool getLinkInfo(const char* iFilePath, LinkInfo& oLinkInfo) {
    MemoryBuffer fileContent;
    bool loadSuccess = fileContent.loadFile(iFilePath);
    if(loadSuccess) {
        // validate signature
        bool link = isLink(fileContent);
        if(link) {
            unsigned long offset = 0;
            const unsigned char* content = fileContent.getBuffer();

            const ShellLinkHeader& header = readData<const ShellLinkHeader>(content, offset);

            oLinkInfo.customIcon.index = header.IconIndex;
            oLinkInfo.hotKey = header.HotKey;

            if(header.linkFlags.HasLinkTargetIDList) {
                // Shell Item Id List
                // Note: This section exists only if the first bit for link flags is set the header section.
                //       If that bit is not set then this section does not exists.
                //       The first word contains the size of the list in bytes.
                //       Each item (except the last) in the list contains its size in a word fallowed by the content.
                //       The size includes and the space used to store it. The last item has the size 0.
                //       These items are used to store various informations.
                //       For more info read the SHITEMID documentation.
                // const uint16_t& IDListSize = readData<unsigned short>(content, offset); /* TODO bug? */
                uint16_t ItemIDSize = 0xFFFF;
                while(ItemIDSize != 0) {
                    ItemIDSize = readData<unsigned short>(content, offset);
                    bool isTerminalID = (ItemIDSize == 0);
                    if(!isTerminalID) {
                        // item is valid (last item has a size of 0)

                        // read itemId's content
                        MemoryBuffer ItemID;
                        serialize(ItemIDSize, ItemID);
                        while(ItemID.getSize() < ItemIDSize) {
                            const unsigned char& c = readData<unsigned char>(content, offset);
                            serialize(c, ItemID);
                        }

                        // check itemId's content
                        const uint8_t& type = ItemID.getBuffer()[2];
                        switch(type) {
                            case 0x1f:  // computer data. ignore
                                break;
                            case 0x2f:  // drive data.
                                oLinkInfo.target = (char*)&ItemID.getBuffer()[3];
                                break;
                            case 0x31:  // folder data
                            case 0x32:  // file data
                            {
                                std::string name83;
                                std::string nameLong;
                                LNK_ITEMID itemId = {0};
                                bool success = deserialize(ItemID, itemId, name83, nameLong);
                                assert(success == true);

                                if(oLinkInfo.target.size() == 0) {
                                    oLinkInfo.target += ".\\";
                                } else {
                                    // since we are adding a folder of file name,
                                    // make sure the path is ending with a separator
                                    const char& lastCharacter = oLinkInfo.target[oLinkInfo.target.size() - 1];
                                    if(lastCharacter != '\\')
                                        oLinkInfo.target += '\\';
                                }
                                oLinkInfo.target += nameLong;
                            }
                        };
                    }
                }
            }

            {
                // File location info
                const LNK_FILE_LOCATION_INFO& fileInfo = readData<LNK_FILE_LOCATION_INFO>(content, offset);
                offset += (fileInfo.length - fileInfo.endOffset);

                const unsigned char* baseFileLocationAddress = (unsigned char*)(&fileInfo);
                if(fileInfo.length > 0) {
                    std::string basePath = "";
                    if(fileInfo.basePathOffset)
                        basePath = (const char*)&baseFileLocationAddress[fileInfo.basePathOffset];
                    std::string finalPath = "";
                    if(fileInfo.finalPathOffset)
                        finalPath = (const char*)&baseFileLocationAddress[fileInfo.finalPathOffset];

                    // concat paths
                    if(oLinkInfo.target.size() == 0) {
                        // target was not resolved using LinkTargetIDList, resolve using base and final paths
                        if(basePath.size() > 0)
                            oLinkInfo.target = basePath;
                        if(finalPath.size() > 0) {
                            if(oLinkInfo.target.size() == 0)
                                oLinkInfo.target = finalPath;
                            else {
                                oLinkInfo.target += '\\';
                                oLinkInfo.target += finalPath;
                            }
                        }
                    }

                    if(fileInfo.localVolumeTableOffset > 0 && fileInfo.location == LNK_LOCATION_LOCAL) {
                        unsigned long tmpOffset = fileInfo.localVolumeTableOffset;
                        const LNK_LOCAL_VOLUME_TABLE& volumeTable =
                            readData<LNK_LOCAL_VOLUME_TABLE>(baseFileLocationAddress, tmpOffset);
                        // const char* volumeName = &volumeTable.volumeLabel; /* TODO bug? */
                        assert(volumeTable.length >= LNK_LOCAL_VOLUME_TABLE_SIZE);
                    }
                    if(fileInfo.networkVolumeTableOffset > 0 && fileInfo.location == LNK_LOCATION_NETWORK) {
                        unsigned long tmpOffset = fileInfo.networkVolumeTableOffset;
                        const LNK_NETWORK_VOLUME_TABLE& volumeTable =
                            readData<LNK_NETWORK_VOLUME_TABLE>(baseFileLocationAddress, tmpOffset);
                        const char* volumeName = &volumeTable.networkShareName;
                        assert(volumeTable.length >= LNK_NETWORK_VOLUME_TABLE_SIZE);

                        // build network path
                        oLinkInfo.networkPath = volumeName;
                        oLinkInfo.networkPath += '\\';
                        oLinkInfo.networkPath += finalPath;
                    }
                }
            }

            // Description
            // This section is present if bit 2 is set in the flags value in the header.
            // The first word value indicates the length of the string.
            // Following the length value is a string of ASCII characters.
            // It is a description of the item.
            if(header.linkFlags.HasName)
                readString(content, offset, oLinkInfo.description);

            // Relative path string
            // This section is present if bit 3 is set in the flags value in the header.
            // The first word value indicates the length of the string.
            // Following the length value is a string of ASCII characters.
            // It is a relative path to the target.
            std::string relativePath;
            if(header.linkFlags.HasRelativePath)
                readString(content, offset, relativePath);

            // Working directory
            // This section is present if bit 4 is set in the flags value in the header.
            // The first word value indicates the length of the string.
            // Following the length value is a string of ASCII characters.
            // It is the working directory as specified in the link properties.
            if(header.linkFlags.HasWorkingDir)
                readString(content, offset, oLinkInfo.workingDirectory);

            // Command line arguments
            // This section is present if bit 5 is set in the flags value in the header.
            // The first word value indicates the length of the string.
            // Following the length value is a string of ASCII characters.
            // The command line string includes everything except the program name.
            if(header.linkFlags.HasArguments)
                readString(content, offset, oLinkInfo.arguments);

            // Icon filename
            // This section is present if bit 6 is set in the flags value in the header.
            // The first word value indicates the length of the string.
            // Following the length value is a string of ASCII characters.
            // This the name of the file containing the icon.
            if(header.linkFlags.HasIconLocation)
                readString(content, offset, oLinkInfo.customIcon.filename);

            // Additonal Info Usualy consists of a dword with the value 0.
            [[maybe_unused]] const unsigned long& additionnalInfo = readData<unsigned long>(content, offset);

            return true;
        }
    }
    return false;
}

MemoryBuffer createLinkTargetIDList(const char* iFilePath, const LinkInfo& iLinkInfo) {
    MemoryBuffer LinkTargetIDList;

    // convert the long path name to short path name
    std::string shortPath = filesystem::getShortPathForm(iLinkInfo.target.c_str());
    if(shortPath.empty())
        return LinkTargetIDList;

    char driveLetter = shortPath[0];
    driveLetter = toupper(driveLetter);

    ItemIDList itemIDList;
    size_t i = 0;
    itemIDList.push_back(getComputerItemId());
    if(driveLetter == '\\') {
        itemIDList.push_back(getDriveItemId());
    } else {
        itemIDList.push_back(getDriveItemId(driveLetter));
        ++i;
    }

    // split path
    StringList shortPathParts;
    filesystem::splitPath(shortPath.c_str(), shortPathParts);
    if(shortPathParts.size() <= 2)
        return LinkTargetIDList;  // short path needs at least a drive/folder/filename structure
    StringList longPathParts;
    filesystem::splitPath(iLinkInfo.target.c_str(), longPathParts);
    if(longPathParts.size() <= 2)
        return LinkTargetIDList;  // short path needs at least a drive/folder/filename structure
    if(shortPathParts.size() != longPathParts.size())
        return LinkTargetIDList;  // both long and short paths needs to be the same size

    // shortPathParts[0]	C:
    // shortPathParts[1]	PROGRA~1
    // shortPathParts[2]	7-Zip
    // shortPathParts[3]	History.txt

    // longPathParts[0]	C:
    // longPathParts[1]	Program Files
    // longPathParts[2]	7-Zip
    // longPathParts[3]	History.txt

    // setup folder/filename data
    size_t numFileSystemObjects = shortPathParts.size();
    for(; i < numFileSystemObjects; i++) {
        std::string& shortItem = shortPathParts[i];
        std::string& longItem = longPathParts[i];
        FILE_ATTRIBUTES attr = ((i + 1 < numFileSystemObjects) ? FA_DIRECTORY : FA_NORMAL);

        MemoryBuffer ItemID = getFileItemId(shortItem, longItem, attr);
        itemIDList.push_back(ItemID);
    }

    // add TerminalID
    itemIDList.push_back(getTerminalItemId());

    LinkTargetIDList = getLinkTargetIDList(itemIDList);

    return LinkTargetIDList;
}

bool createLink(const char* iFilePath, const LinkInfo& iLinkInfo) {
    // building header
    ShellLinkHeader header = {0};
    header.HeaderSize = sizeof(ShellLinkHeader);
    memcpy(header.LinkCLSID, DEFAULT_LINKCLSID, sizeof(LNK_CLSID));

    // detect target
    bool isTargetFolder = filesystem::folderExists(iLinkInfo.target.c_str());
    bool isTargetFile = filesystem::fileExists(iLinkInfo.target.c_str());

    // building LinkFlags
    {
        LinkFlags& flags = header.linkFlags;
        flags.HasLinkTargetIDList = 1;
        flags.HasLinkInfo = isTargetFile || isTargetFolder;
        flags.HasName = iLinkInfo.description.size() > 0;
        flags.HasRelativePath = 0;
        flags.HasWorkingDir = iLinkInfo.workingDirectory.size() > 0;
        flags.HasArguments = iLinkInfo.arguments.size() > 0;
        flags.HasIconLocation = iLinkInfo.customIcon.filename.size() > 0;
        flags.reserved1 = 1;
        flags.reserved2 = 0;
        flags.reserved3 = 0;
        flags.reserved4 = 0;
    }

    // build FileAttributesFlags
    {
        FileAttributesFlags& flags = header.FileAttributes;
        flags.isReadOnly = 0;
        flags.isHidden = 0;
        flags.isSystemFile = 0;
        flags.isVolumeLabel = 0;
        flags.isDirectory = isTargetFolder;
        flags.isArchive = 1;
        flags.isEncrypted = 0;
        flags.isNormal = 0;
        flags.isTemporary = 0;
        flags.isSparseFile = 0;
        flags.hasReparsePoint = 0;
        flags.isCompressed = 0;
        flags.isOffline = 0;
        flags.reserved1 = 0;
        flags.reserved2 = 1;
        flags.reserved3 = 0;
    }

    header.CreationTime = 0;
    header.AccessTime = 0;
    header.WriteTime = 0;
    header.FileSize = (isTargetFile ? filesystem::getFileSize(iLinkInfo.target.c_str()) : 0);
    header.IconIndex = (iLinkInfo.customIcon.filename.size() > 0 ? iLinkInfo.customIcon.index : 0);
    header.ShowCommand = 1;
    header.HotKey = iLinkInfo.hotKey;
    header.Reserved1 = 0;
    header.Reserved2 = 0;
    header.Reserved3 = 0;

    // LinkTargetIDList
    MemoryBuffer LinkTargetIDList = createLinkTargetIDList(iFilePath, iLinkInfo);
    if(LinkTargetIDList.getSize() == 0)
        return false;  // unable to build LinkTargetIDList

    // File location info
    LNK_FILE_LOCATION_INFO fileInfo = {0};
    fileInfo.length = LNK_FILE_LOCATION_INFO_SIZE + LNK_LOCAL_VOLUME_TABLE_SIZE + iLinkInfo.target.size() + 2;
    fileInfo.endOffset = LNK_FILE_LOCATION_INFO_SIZE;
    fileInfo.location = LNK_LOCATION_LOCAL;
    fileInfo.localVolumeTableOffset = LNK_FILE_LOCATION_INFO_SIZE;
    fileInfo.basePathOffset = LNK_FILE_LOCATION_INFO_SIZE + LNK_LOCAL_VOLUME_TABLE_SIZE;
    fileInfo.networkVolumeTableOffset = 0;
    fileInfo.finalPathOffset = fileInfo.length - 1;

    LNK_LOCAL_VOLUME_TABLE volumeTable = {0};
    volumeTable.length = LNK_LOCAL_VOLUME_TABLE_SIZE;
    volumeTable.volumeType = LNK_VOLUME_TYPE_FIXED;
    volumeTable.volumeSerialNumber = 0;
    volumeTable.volumeNameOffset = LNK_LOCAL_VOLUME_TABLE_SIZE - 1;
    volumeTable.volumeLabel = '\0';

    // Save data to a file
    FILE* f;
    errno_t error = fopen_s(&f, iFilePath, "wb");
    if(error == 0) {
        size_t tmp = 0;

        // Save header
        tmp = sizeof(header);
        fwrite(&header, 1, tmp, f);

        // LinkTargetIDList
        tmp = LinkTargetIDList.getSize();
        fwrite(LinkTargetIDList.getBuffer(), 1, tmp, f);

        // File location info & volume table
        tmp = sizeof(fileInfo);
        fwrite(&fileInfo, 1, tmp, f);
        tmp = sizeof(volumeTable);
        fwrite(&volumeTable, 1, tmp, f);
        saveString(f, iLinkInfo.target);  // basic path
        saveString(f, "");                // final path

        LinkFlags& flags = header.linkFlags;

        // Description
        if(flags.HasName)
            saveStringUnicode(f, iLinkInfo.description);

        // Relative path string
        //(never)

        // Working directory
        if(flags.HasWorkingDir)
            saveStringUnicode(f, iLinkInfo.workingDirectory);

        // Command line arguments
        if(flags.HasArguments)
            saveStringUnicode(f, iLinkInfo.arguments);

        // Icon filename
        if(flags.HasIconLocation)
            saveStringUnicode(f, iLinkInfo.customIcon.filename);

        // Additonal Info Usualy consists of a dword with the value 0.
        const unsigned long additionnalInfo = 0;
        fwrite(&additionnalInfo, 1, sizeof(additionnalInfo), f);

        fclose(f);
        return true;
    }

    return false;
}

std::string toString(const LNK_HOTKEY& iHotKey) {
    std::string value;

    if(iHotKey.modifiers & LNK_HK_MOD_CONTROL) {
        if(value.size() > 0)
            value += ' ';
        value += "CTRL +";
    }
    if(iHotKey.modifiers & LNK_HK_MOD_ALT) {
        if(value.size() > 0)
            value += ' ';
        value += "ALT +";
    }
    if(iHotKey.modifiers & LNK_HK_MOD_SHIFT) {
        if(value.size() > 0)
            value += ' ';
        value += "SHIFT +";
    }
    if(value.size() > 0)
        value += ' ';

    // key
    if(iHotKey.keyCode >= '0' && iHotKey.keyCode <= 'Z')
        value += (char)iHotKey.keyCode;
    if(iHotKey.keyCode >= LNK_HK_F1 && iHotKey.keyCode <= LNK_HK_F12) {
        value += 'F';
        value += (char)(iHotKey.keyCode + 1 - LNK_HK_F1);
    }
    if(iHotKey.keyCode >= LNK_HK_NUMLOCK)
        value += "Numlock";
    if(iHotKey.keyCode >= LNK_HK_SCROLL)
        value += "ScroolLock";

    return value;
}

bool printLinkInfo(const char* iFilePath) {
    FILE* f;
    errno_t error = fopen_s(&f, iFilePath, "rb");
    if(error == 0) {
        printf("Link file: %s\n", iFilePath);

        // read & print header
        ShellLinkHeader header = {0};
        fread(&header, 1, sizeof(header), f);
        // signature
        printf("HeaderSize: %d\n", header.HeaderSize);
        // LinkCLSID
        printf("LinkCLSID: 0x%02x 0x%02x 0x%02x 0x%02x \n", header.LinkCLSID[0], header.LinkCLSID[1],
               header.LinkCLSID[2], header.LinkCLSID[3]);
        printf("      0x%02x 0x%02x 0x%02x 0x%02x \n", header.LinkCLSID[4], header.LinkCLSID[5], header.LinkCLSID[6],
               header.LinkCLSID[7]);
        printf("      0x%02x 0x%02x 0x%02x 0x%02x \n", header.LinkCLSID[8], header.LinkCLSID[9], header.LinkCLSID[10],
               header.LinkCLSID[11]);
        printf("      0x%02x 0x%02x 0x%02x 0x%02x \n", header.LinkCLSID[12], header.LinkCLSID[13], header.LinkCLSID[14],
               header.LinkCLSID[15]);
        // link flags
        printf("link flags: HasLinkTargetIDList   =%c\n", (header.linkFlags.HasLinkTargetIDList ? 'T' : 'F'));
        printf("                HasLinkInfo  =%c\n", (header.linkFlags.HasLinkInfo ? 'T' : 'F'));
        printf("                HasName           =%c\n", (header.linkFlags.HasName ? 'T' : 'F'));
        printf("                HasRelativePath          =%c\n", (header.linkFlags.HasRelativePath ? 'T' : 'F'));
        printf("                HasWorkingDir      =%c\n", (header.linkFlags.HasWorkingDir ? 'T' : 'F'));
        printf("                HasArguments  =%c\n", (header.linkFlags.HasArguments ? 'T' : 'F'));
        printf("                HasIconLocation            =%c\n", (header.linkFlags.HasIconLocation ? 'T' : 'F'));
        printf("                reserved1                =%c\n", (header.linkFlags.reserved1 ? 'T' : 'F'));
        printf("                reserved2                =%02x\n", header.linkFlags.reserved2);
        printf("                reserved3                =%02x\n", header.linkFlags.reserved3);
        printf("                reserved4                =%02x\n", header.linkFlags.reserved4);
        // target flags
        printf("target flags: isReadOnly      =%c\n", (header.FileAttributes.isReadOnly ? 'T' : 'F'));
        printf("              isHidden        =%c\n", (header.FileAttributes.isHidden ? 'T' : 'F'));
        printf("              isSystemFile    =%c\n", (header.FileAttributes.isSystemFile ? 'T' : 'F'));
        printf("              isVolumeLabel   =%c\n", (header.FileAttributes.isVolumeLabel ? 'T' : 'F'));
        printf("              isDirectory     =%c\n", (header.FileAttributes.isDirectory ? 'T' : 'F'));
        printf("              isArchive       =%c\n", (header.FileAttributes.isArchive ? 'T' : 'F'));
        printf("              isEncrypted     =%c\n", (header.FileAttributes.isEncrypted ? 'T' : 'F'));
        printf("              isNormal        =%c\n", (header.FileAttributes.isNormal ? 'T' : 'F'));
        printf("              isTemporary     =%c\n", (header.FileAttributes.isTemporary ? 'T' : 'F'));
        printf("              isSparseFile    =%c\n", (header.FileAttributes.isSparseFile ? 'T' : 'F'));
        printf("              hasReparsePoint =%c\n", (header.FileAttributes.hasReparsePoint ? 'T' : 'F'));
        printf("              isCompressed    =%c\n", (header.FileAttributes.isCompressed ? 'T' : 'F'));
        printf("              isOffline       =%c\n", (header.FileAttributes.isOffline ? 'T' : 'F'));
        printf("              reserved1       =%c\n", (header.FileAttributes.reserved1 ? 'T' : 'F'));
        printf("              reserved2       =%c\n", (header.FileAttributes.reserved2 ? 'T' : 'F'));
        printf("              reserved3       =%c\n", (header.FileAttributes.reserved3 ? 'T' : 'F'));
        std::string creationTimeStr = toTimeString(header.CreationTime);
        std::string modificationTimeStr = toTimeString(header.WriteTime);
        std::string lastAccessTimeStr = toTimeString(header.AccessTime);
        // const char* test = "test";
        // PRINTF BUG. Unable to display using "0x%08x (%s)"
        // printf("time stamps:  CreationTime      = 0x%08x (%s) \n", header.CreationTime, creationTimeStr.c_str());
        // printf("              WriteTime  = 0x%08x (%s) \n", header.WriteTime, modificationTimeStr.c_str());
        // printf("              AccessTime    = 0x%08x (%s) \n", header.AccessTime, lastAccessTimeStr.c_str());
        printf("time stamps:  CreationTime      = 0x%08llx ", header.CreationTime);
        printf("(%s) \n", creationTimeStr.c_str());
        printf("              WriteTime  = 0x%08llx ", header.WriteTime);
        printf("(%s) \n", modificationTimeStr.c_str());
        printf("              AccessTime    = 0x%08llx ", header.AccessTime);
        printf("(%s) \n", lastAccessTimeStr.c_str());
        // remaining properties
        printf("FileSize = 0x%04lx\n", header.FileSize);
        printf("IconIndex = 0x%04lx\n", header.IconIndex);
        printf("ShowCommand = 0x%04lx\n", header.ShowCommand);
        std::string hotKeyDescription = toString(header.HotKey);
        printf("HotKey    = keyCode=0x%04x, modifiers=0x%04x (%s)\n", header.HotKey.keyCode, header.HotKey.modifiers,
               hotKeyDescription.c_str());
        printf("Reserved1 = 0x%04x\n", header.Reserved1);
        printf("Reserved2 = 0x%04x\n", header.Reserved2);
        printf("Reserved3 = 0x%04x\n", header.Reserved3);

        // LinkTargetIDList
        if(header.linkFlags.HasLinkTargetIDList) {
            unsigned short listSize = 0;
            fread(&listSize, 1, sizeof(listSize), f);
            printf("LinkTargetIDList size=0x%02x (%02d)\n", listSize, listSize);

            unsigned short itemIdSize = 0xFFFF;
            unsigned char sequenceNumber = 0;
            while(itemIdSize != 0) {
                fread(&itemIdSize, 1, sizeof(itemIdSize), f);
                if(itemIdSize > 0) {
                    sequenceNumber++;

                    // item is valid (last item has a size of 0)
                    printf("itemId %d size=0x%02x (%02d)\n", sequenceNumber, itemIdSize, itemIdSize);
                    printf("data=");

                    unsigned short remainingDataSize = itemIdSize - sizeof(itemIdSize);
                    while(remainingDataSize) {
                        unsigned char c = 0;
                        fread(&c, 1, sizeof(c), f);
                        printf("%02x", c);
                        remainingDataSize--;
                        if(remainingDataSize)
                            printf(" ");
                    }
                    printf("\n");
                }
            }
        }

        // File location info
        {
            long fileOffset = ftell(f);

            // read file location info size
            unsigned long fileInfoDataSize = 0;
            fread(&fileInfoDataSize, 1, sizeof(fileInfoDataSize), f);

            // rewind back to before the fileInfoDataSize
            fseek(f, fileOffset, SEEK_SET);

            // read the whole LNK_FILE_LOCATION_INFO data
            MemoryBuffer fileInfoData;
            fileInfoData.allocate(fileInfoDataSize);
            fread(fileInfoData.getBuffer(), 1, fileInfoData.getSize(), f);

            const LNK_FILE_LOCATION_INFO* fileInfo = (const LNK_FILE_LOCATION_INFO*)fileInfoData.getBuffer();

            printf("file location info: length                    = 0x%04lx (%lu)\n", fileInfo->length,
                   fileInfo->length);
            printf("                    endOffset                 = 0x%04lx (%lu)\n", fileInfo->endOffset,
                   fileInfo->endOffset);
            printf("                    location                  = 0x%04lx (%lu)\n", fileInfo->location,
                   fileInfo->location);
            printf("                    localVolumeTableOffset    = 0x%04lx (%lu)\n", fileInfo->localVolumeTableOffset,
                   fileInfo->localVolumeTableOffset);
            printf("                    basePathOffset            = 0x%04lx (%lu)\n", fileInfo->basePathOffset,
                   fileInfo->basePathOffset);
            printf("                    networkVolumeTableOffset  = 0x%04lx (%lu)\n",
                   fileInfo->networkVolumeTableOffset, fileInfo->networkVolumeTableOffset);
            printf("                    finalPathOffset           = 0x%04lx (%lu)\n", fileInfo->finalPathOffset,
                   fileInfo->finalPathOffset);

            const unsigned char* baseFileLocationAddress = fileInfoData.getBuffer();
            ;
            if(fileInfo->length > 0) {
                const char* basePath = "";
                if(fileInfo->basePathOffset)
                    basePath = (const char*)&baseFileLocationAddress[fileInfo->basePathOffset];
                const char* finalPath = "";
                if(fileInfo->finalPathOffset)
                    finalPath = (const char*)&baseFileLocationAddress[fileInfo->finalPathOffset];

                printf("                    basePath                  = \"%s\" \n", basePath);
                printf("                    finalPath                 = \"%s\" \n", finalPath);

                if(fileInfo->localVolumeTableOffset > 0 && fileInfo->location == LNK_LOCATION_LOCAL) {
                    const LNK_LOCAL_VOLUME_TABLE* volumeTable =
                        (LNK_LOCAL_VOLUME_TABLE*)&baseFileLocationAddress[fileInfo->localVolumeTableOffset];
                    const char* volumeName = &volumeTable->volumeLabel;
                    assert(volumeTable->length >= LNK_LOCAL_VOLUME_TABLE_SIZE);

                    printf("                    LNK_LOCAL_VOLUME_TABLE:\n");
                    printf("                           length             = 0x%04lx (%lu)\n", volumeTable->length,
                           volumeTable->length);
                    printf("                           volumeType         = 0x%04lx (%lu)\n", volumeTable->volumeType,
                           volumeTable->volumeType);
                    printf("                           volumeSerialNumber = 0x%04lx (%lu)\n",
                           volumeTable->volumeSerialNumber, volumeTable->volumeSerialNumber);
                    printf("                           volumeNameOffset   = 0x%04lx (%lu)\n",
                           volumeTable->volumeNameOffset, volumeTable->volumeNameOffset);
                    printf("                           volumeLabel        = \"%s\" \n", volumeName);
                }
                if(fileInfo->networkVolumeTableOffset > 0 && fileInfo->location == LNK_LOCATION_NETWORK) {
                    const LNK_NETWORK_VOLUME_TABLE* volumeTable =
                        (LNK_NETWORK_VOLUME_TABLE*)&baseFileLocationAddress[fileInfo->networkVolumeTableOffset];
                    const char* volumeName = &volumeTable->networkShareName;
                    assert(volumeTable->length >= LNK_NETWORK_VOLUME_TABLE_SIZE);

                    printf("                    LNK_NETWORK_VOLUME_TABLE:\n");
                    printf("                           length                 = 0x%04lx (%lu)\n", volumeTable->length,
                           volumeTable->length);
                    printf("                           reserved1              = 0x%04lx (%lu)\n",
                           volumeTable->reserved1, volumeTable->reserved1);
                    printf("                           networkShareNameOffset = 0x%04lx (%lu)\n",
                           volumeTable->networkShareNameOffset, volumeTable->networkShareNameOffset);
                    printf("                           reserved2              = 0x%04lx (%lu)\n",
                           volumeTable->reserved2, volumeTable->reserved2);
                    printf("                           reserved3              = 0x%04lx (%lu)\n",
                           volumeTable->reserved3, volumeTable->reserved3);
                    printf("                           networkShareName       = \"%s\" \n", volumeName);
                }
            }
        }

        // Description
        if(header.linkFlags.HasName) {
            std::string value = readUnicodeString(f);
            printf("Description = \"%s\" \n", value.c_str());
        }

        // Relative path string
        if(header.linkFlags.HasRelativePath) {
            std::string value = readUnicodeString(f);
            printf("Relative path string = \"%s\" \n", value.c_str());
        }

        // Working directory
        if(header.linkFlags.HasWorkingDir) {
            std::string value = readUnicodeString(f); /* TODO bug? */
            printf("Working directory = \"%s\" \n", value.c_str());
        }

        // Command line arguments
        if(header.linkFlags.HasArguments) {
            std::string value = readUnicodeString(f);
            printf("Command line arguments = \"%s\" \n", value.c_str());
        }

        // Icon filename
        if(header.linkFlags.HasIconLocation) {
            std::string value = readUnicodeString(f);
            printf("Icon filename = \"%s\" \n", value.c_str());
        }

        // Additonal Info Usualy consists of a dword with the value 0.
        unsigned long additionalInfoBlockSize = 0;
        fread(&additionalInfoBlockSize, 1, sizeof(additionalInfoBlockSize), f);
        unsigned long blockNumber = 0;
        while(additionalInfoBlockSize > 0) {
            blockNumber++;

            // rewind for reading block size
            long seekOffset = sizeof(additionalInfoBlockSize);
            seekOffset *= -1;
            fseek(f, seekOffset, SEEK_CUR);

            // read additionnal info block
            MemoryBuffer block(additionalInfoBlockSize);
            fread(block.getBuffer(), 1, block.getSize(), f);

            printf("Additionnal information block #%ld size=0x%02lx (%02lu) \n", blockNumber, additionalInfoBlockSize,
                   additionalInfoBlockSize);
            printf("data=");

            const unsigned char* content = block.getBuffer();
            for(unsigned long i = sizeof(additionalInfoBlockSize); i < block.getSize(); i++) {
                const unsigned char& c = content[i];
                printf("%02x", c);
                if(i + 1 < block.getSize())
                    printf(" ");
            }
            printf("\n");

            // read next additional block size
            fread(&additionalInfoBlockSize, 1, sizeof(additionalInfoBlockSize), f);
        }

        fclose(f);
        return true;
    }

    return false;
}

bool changeLink(const char* iFilePath, const char* before_pattern, const char* after_pattern) {
    LinkInfo info_bf, info_af;
    if(!getLinkInfo(iFilePath, info_bf))
        return false;

    info_af.arguments = info_bf.arguments;
    info_af.description = info_bf.description;
    info_af.customIcon.filename = info_bf.customIcon.filename;
    info_af.customIcon.index = info_bf.customIcon.index;
    info_af.hotKey.keyCode = info_bf.hotKey.keyCode;
    info_af.hotKey.modifiers = info_bf.hotKey.modifiers;

    info_af.target = stringfunc::strReplace(info_bf.target, before_pattern, after_pattern);
    info_af.networkPath = stringfunc::strReplace(info_bf.networkPath, before_pattern, after_pattern);
    info_af.workingDirectory = stringfunc::strReplace(info_bf.workingDirectory, before_pattern, after_pattern);

    if(remove(iFilePath) != 0)
        return false;

    return createLink(iFilePath, info_af);
}


std::string getLinkpath(const char* iFilePath) {
    FILE* f;
    errno_t error = fopen_s(&f, iFilePath, "rb");
    if(error == 0) {
        ShellLinkHeader header = {0};
        fread(&header, 1, sizeof(header), f);
        std::string hotKeyDescription = toString(header.HotKey);

        if(header.linkFlags.HasLinkTargetIDList) {
            unsigned short listSize = 0;
            fread(&listSize, 1, sizeof(listSize), f);

            unsigned short itemIdSize = 0xFFFF;
            unsigned char sequenceNumber = 0;
            while(itemIdSize != 0) {
                fread(&itemIdSize, 1, sizeof(itemIdSize), f);
                if(itemIdSize > 0) {
                    sequenceNumber++;

                    unsigned short remainingDataSize = itemIdSize - sizeof(itemIdSize);
                    while(remainingDataSize) {
                        unsigned char c = 0;
                        fread(&c, 1, sizeof(c), f);
                        remainingDataSize--;
                    }
                }
            }
        }

        long fileOffset = ftell(f);

        unsigned long fileInfoDataSize = 0;
        fread(&fileInfoDataSize, 1, sizeof(fileInfoDataSize), f);

        fseek(f, fileOffset, SEEK_SET);

        MemoryBuffer fileInfoData;
        fileInfoData.allocate(fileInfoDataSize);
        fread(fileInfoData.getBuffer(), 1, fileInfoData.getSize(), f);

        const LNK_FILE_LOCATION_INFO* fileInfo = (const LNK_FILE_LOCATION_INFO*)fileInfoData.getBuffer();

        const unsigned char* baseFileLocationAddress = fileInfoData.getBuffer();

        if(fileInfo->length > 0) {
            if(fileInfo->networkVolumeTableOffset > 0 && fileInfo->location == LNK_LOCATION_NETWORK) {
                const LNK_NETWORK_VOLUME_TABLE* volumeTable =
                    (LNK_NETWORK_VOLUME_TABLE*)&baseFileLocationAddress[fileInfo->networkVolumeTableOffset];
                const char* volumeName = &volumeTable->networkShareName;
                assert(volumeTable->length >= LNK_NETWORK_VOLUME_TABLE_SIZE);
                fclose(f);
                return volumeName;
            }

            const char* basePath = "";
            if(fileInfo->basePathOffset)
                basePath = (const char*)&baseFileLocationAddress[fileInfo->basePathOffset];

            fclose(f);
            return basePath;
        }

    }

    fclose(f);
    return "";
}
