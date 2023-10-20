/* argparser.hpp | MIT License | https://github.com/kirin123kirin/cutil/raw/main/LICENSE */
#ifndef _ARGPARSE_HPP_
#define _ARGPARSE_HPP_

#include <string> /* for char_traits */
#include <vector>
#include <iostream>
#include <ios>     // std::left
#include <iomanip> // std::setw(int)

enum class ITEM { REQUIRED, OPTION, NONE, NULL_t };
enum class TP { INT, DOUBLE, STRING, CHAR, CHARP, BOOL, NONE, NULL_t };


template <typename CharT, typename Traits = std::char_traits<CharT>>
class _ArgParser {

    struct argstruct {
        const CharT* shortarg;
        const CharT* longarg;
        TP type;
        ITEM item;
        char* arg;
        const CharT* helpstr;
        int parsed;
        int has_eq;
        std::size_t arg_len;

        template <typename T>
        argstruct(const CharT* _shortarg,
                  const CharT* _longarg,
                  T* _arg,
                  const CharT* _helpstr,
                  ITEM _item = ITEM::OPTION,
                  TP _type = TP::NONE)
            : shortarg(_shortarg),
              longarg(_longarg),
              item(_item),
              arg((char*)_arg),
              helpstr(_helpstr),
              parsed(0),
              has_eq(-1) {

            type = (_type == TP::NONE) ? getType(*_arg) : _type;
            std::size_t shortlen = _shortarg[0] ? Traits::length(_shortarg) : 0;
            std::size_t longlen = _longarg[0] ? Traits::length(_longarg) : 0;
            arg_len = (shortlen && longlen) ? (shortlen + 2 + longlen) : (shortlen + longlen);

        };

        argstruct(std::nullptr_t) : shortarg(NULL),
              longarg(NULL),
              arg(NULL),
              helpstr(NULL),
              type(TP::NULL_t),
              item(ITEM::NULL_t),
              parsed(0),
              has_eq(-1),
              arg_len(0) {};

        bool operator==(std::nullptr_t) const { return type == TP::NULL_t; }
        bool operator!=(std::nullptr_t) const { return type != TP::NULL_t; }

        bool operator[](const CharT* v) {
            has_eq = 0;
            const CharT* base = v;
            if(v == NULL || *v != '-')
                return false;
            if(*++v != '-') {
                while (*v && *v != '=') {
                    if (*v++ == shortarg[1]){
                        has_eq = (*v == '=') ? v - base : 0;
                        return true;
                    }
                }
            } else  {
                const CharT* la = longarg + 1;
                while(*v && *la) {
                    if(*v++ != *la++)
                        return false;
                }
                if((*v == 0 || *v == '=') && *la == 0) {
                    has_eq = (*v == '=') ? v - base : 0;
                    return true;
                }
            }
            return false;
        }

       private:
        TP getType(char arg) { return TP::CHAR; }
        TP getType(wchar_t arg) { return TP::CHAR; }
        TP getType(char16_t arg) { return TP::CHAR; }
        TP getType(char32_t arg) { return TP::CHAR; }
        TP getType(char* arg) { return TP::CHARP; }
        TP getType(wchar_t* arg) { return TP::CHARP; }
        TP getType(char16_t* arg) { return TP::CHARP; }
        TP getType(char32_t* arg) { return TP::CHARP; }
        TP getType(double arg) { return TP::INT; }
        TP getType(float) { return TP::INT; }
        TP getType(bool) { return TP::BOOL; }
        TP getType(int8_t) { return TP::INT; }
        TP getType(int16_t) { return TP::INT; }
        TP getType(int32_t) { return TP::INT; }
        TP getType(int64_t) { return TP::INT; }
        TP getType(uint8_t) { return TP::INT; }
        TP getType(uint16_t) { return TP::INT; }
        TP getType(uint32_t) { return TP::INT; }
        TP getType(uint64_t) { return TP::INT; }
        template <typename T> TP getType(std::basic_string<T> arg) {return TP::STRING;}

    };

   public:
    int argc;
    CharT** argv;
    std::vector<argstruct> args;
    std::vector<CharT*> positional_argv;
    const CharT* desc;
    unsigned int allow_postional_argc = UINT_MAX;

    _ArgParser(int org_argc, CharT** org_argv) : desc(NULL), argc(org_argc), argv(org_argv) {};
    _ArgParser(const CharT* _desc, int org_argc, CharT** org_argv) : argc(org_argc), argv(org_argv), desc(_desc) {};
    ~_ArgParser(){};

   private:
    std::size_t size_ = 0;
    CharT split_shortarg[3] = {'-', 0, 0};
    char escape_array[128] = {0,    1,   2,    3,   4,    5,   6,    7,    8,   9,   10,  11,  12,   13,  14,   15,
                              16,   17,  18,   19,  20,   21,  22,   23,   24,  25,  26,  27,  28,   29,  30,   31,
                              32,   33,  '\"', 35,  36,   37,  38,   '\'', 40,  41,  42,  43,  44,   45,  46,   47,
                              '\0', 49,  50,   51,  52,   53,  54,   55,   56,  57,  58,  59,  60,   61,  62,   '\?',
                              64,   65,  66,   67,  68,   69,  70,   71,   72,  73,  74,  75,  76,   77,  78,   79,
                              80,   81,  82,   83,  84,   85,  86,   87,   88,  89,  90,  91,  '\\', 93,  94,   95,
                              96,   97,  '\b', 99,  100,  101, '\f', 103,  104, 105, 106, 107, 108,  109, '\n', 111,
                              112,  113, '\r', 115, '\t', 117, '\v', 119,  120, 121, 122, 123, 124,  125, 126,  127};

    CharT* unescape(CharT* buf) {
        std::size_t len = Traits::length(buf);
        if(Traits::find(buf, len, '\\') == nullptr)
            return buf;
        CharT* p = buf;
        for(CharT *end = buf + len; p < end; ++p) {
            if(*p == '\\' && *(char*)++p > -1)
                *p = escape_array[*p];
        }
        *p = 0;
        return buf;
    }

    template <typename T> void _printany_(T s) { if(s) std::cout << s; }
    template <> void _printany_(const char* s) { if(s && *s) std::cout << s; }
    template <> void _printany_(const wchar_t* s) { if(s && *s) std::wcout << s; }
    template <> void _printany_(const wchar_t s) { if(s) std::wcout << s; }
    template <> void _printany_(wchar_t* s) { if(s && *s) std::wcout << s; }
    template <> void _printany_(wchar_t s) { if(s) std::wcout << s; }
    template <> void _printany_(const std::string& s) { if(s.empty()) std::cout << s; }
    template <> void _printany_(const std::wstring& s) { if(s.empty()) std::wcout << s; }
    
    template <typename T> void printany(T s) { _printany_(s); }

    template <typename Char0, typename Char1>
    void printany(const Char0 s0, const Char1 s1) {
        _printany_(s0);
        _printany_(s1);
    }

    template <typename Char0, typename Char1, typename Char2>
    void printany(const Char0 s0, const Char1 s1, const Char2 s2) {
        printany(s0, s1);
        _printany_(s2);
    }

    template <typename Char0, typename Char1, typename Char2, typename Char3>
    void printany(const Char0 s0, const Char1 s1, const Char2 s2, const Char3 s3) {
        printany(s0, s1);
        printany(s2, s3);
    }
    template <typename Char0, typename Char1, typename Char2, typename Char3, typename Char4>
    void printany(const Char0 s0, const Char1 s1, const Char2 s2, const Char3 s3, const Char4 s4) {
        printany(s0, s1, s2);
        printany(s3, s4);
    }

    template <typename T> void _printerr_(T s) { if(s) std::cerr << s; }
    template <> void _printerr_(const char* s) {if(s && *s) std::cerr << s;}
    template <> void _printerr_(char* s) {if(s && *s) std::cerr << s;}
    template <> void _printerr_(const wchar_t* s) {if(s && *s) std::wcerr << s;}
    template <> void _printerr_(wchar_t* s) {if(s && *s) std::wcerr << s;}
    template <> void _printerr_(wchar_t s) {if(s) std::wcerr << s;}
    template <> void _printerr_(const std::string& s) {if(!s.empty()) std::cerr << s;}
    template <> void _printerr_(const std::wstring& s) {if(!s.empty()) std::wcerr << s;}

    template <typename T>
    void printerr(T s) {
        std::cout << "" << std::flush;
        _printerr_(s);
    }
    template <typename Char0, typename Char1>
    void printerr(const Char0 s0, const Char1 s1) {
        _printerr_(s0);
        _printerr_(s1);
    }
    template <typename Char0, typename Char1, typename Char2>
    void printerr(const Char0 s0, const Char1 s1, const Char2 s2) {
        printerr(s0, s1);
        _printerr_(s2);
    }

    template <typename Char0>
    void abort(const Char0 msg) {
        printerr("\nAborting\n\n", msg);
        print_help();
        exit(EXIT_FAILURE);
    }
    template <typename Char0, typename Char1>
    void abort(const Char0 s0, const Char1 s1) {
        printerr(s0);
        abort(s1);
    }
    template <typename Char0, typename Char1, typename Char2>
    void abort(const Char0 s0, const Char1 s1, const Char2 s2) {
        printerr(s0, s1);
        abort(s2);
    }

   public:
    std::size_t size() { return size_; }
    void print_help() {
        std::size_t max_len = 10;  // mean length of "-h, --help";;
        std::size_t rightpad = 4;
        const char* leftpadstr = "    ";

        if(desc)
            printany("Description: \n", leftpadstr, desc, "\n\n");

        printany("Usage: ", argv[0], " [-h,--help] ");
        for(auto& as : args) {
            bool req = as.item == ITEM::REQUIRED;
            printany(req ? " " : " [", as.shortarg);
            if(as.longarg[0])
                printany(',');
            printany(as.longarg, req ? " " : "] ");

            if(as.arg_len > max_len)
                max_len = as.arg_len;
        }
        printany("\n\n");

        printany("Arguments:\n", "    -h, --help\n\n");
        for(auto& as : args) {
            printany("    ", as.shortarg);
            if(as.longarg[0])
                printany(", ", as.longarg);

            std::cout << std::left << std::setw(max_len + rightpad - as.arg_len) << "";
            printany(as.helpstr, "\n");
        }
    }

    template <typename T>
    void add(const CharT* shortarg, const CharT* longarg, std::basic_string<T>* arg, const CharT* helpstr, ITEM item = ITEM::OPTION, TP type = TP::NONE) {
        this->add(shortarg, longarg, &arg.data(), helpstr, item, type);
    }

    template <typename T>
    void add(const CharT* shortarg, const CharT* longarg, T* arg, const CharT* helpstr, ITEM item = ITEM::OPTION, TP type = TP::NONE) {

        if(!shortarg && !longarg)
            abort("Passed NULL pointers to both argstrings");
        if(shortarg && *shortarg == 0)
            abort("Passed empty string to shortarg");
        if(longarg && *longarg == 0)
            abort("Passed empty string to longarg");
        if(shortarg && (Traits::length(shortarg) != 2 || shortarg[0] != '-' || shortarg[1] == '-'))
            abort("Shortarg must be a single dash followed by a single character");
        if(longarg && (Traits::length(longarg) <= 2 || longarg[0] != '-' || longarg[1] != '-'))
            abort("Longarg must be two dashes followed by any number of additional characters");
        if(shortarg && shortarg[0] == '-' && shortarg[1] == 'h' && shortarg[2] == 0)
            abort("-h is reserved short arg");
        if(longarg && longarg[0] == '-' && longarg[1] == '-' && longarg[2] == 'h' &&
           longarg[3] == 'e' && longarg[4] == 'l' && longarg[5] == 'p' && longarg[6] == 0)
                abort("--help is reserved long arg");
        if(!arg)
            abort("Passed NULL arg pointer to argparser_add");
        if(!helpstr)
            abort("Passed NULL help string to argparser_add");

        args.emplace_back(shortarg, longarg, arg, helpstr, item, type);
    }

    argstruct& get(const CharT kw) {
        for(auto& as : args) {
            if(as[kw])
                return as;
        }
        return nullptr;
    }

    void parse(unsigned int max_postional_argc = UINT_MAX) {
        allow_postional_argc = max_postional_argc;

        /* Check if -h, --help was passed as only arg */
        if(argc == 2) {
            CharT* a = argv[1];

            if(a && a[0] && a[0] == '-') {
                if((a[1] == 'h' && a[2] == 0) ||
                   (a[1] == '-' && a[1] == 'h' && a[2] == 'e' && a[3] == 'l' &&
                    a[4] == 'p' && a[5] == 0)) {
                    print_help();
                    exit(EXIT_SUCCESS);
                }
            }
        }

        std::size_t done = 0;
        for(int i = 1; i < argc; i++) {
            for(auto& as : args) {
                if(as[argv[i]] == false)
                    continue;

                as.parsed = 1;
                done = i;

                if(as.type == TP::BOOL) {
                    *(int*)as.arg = *as.arg ? 0 : 1;
                    continue;
                }

                if(i + 1 >= argc) {
                    as.parsed = 0;
                    done = 1;
                    continue;
                }

                CharT* argnext = as.has_eq > 0 ? argv[i] + as.has_eq + 1 : argv[++i];
                if(as.type == TP::INT) {
                    *(int*)as.arg = std::stoi(argnext);
                    if(*(int*)as.arg == 0 && !(argv[i][0] == '0' && argv[i][1] == 0)) {
                        abort(L"Argument Error: 数値型の引数 `", as.shortarg,
                              L"` に対して、数値以外の文字が入力されたことにより失敗しました\n");
                    }
                    break;
                }

                if(as.type == TP::DOUBLE) {
                    *(double*)as.arg = std::stod(argnext);
                    if(*(double*)as.arg == 0 && !(argv[i][0] == '0' && argv[i][1] == 0)) {
                        abort(L"Argument Error: 浮動小数点型の引数 `", as.shortarg,
                              L"` に対して、浮動小数点以外の文字が入力されたことにより失敗しました\n");
                    }
                    break;
                }
                if(as.type == TP::CHARP || as.type == TP::STRING) {
                    unescape(argnext);
                    Traits::copy((CharT*)as.arg, argv[i], Traits::length(argv[i]));
                    break;
                }

                if(as.type == TP::CHAR) {
                    *as.arg = *argnext;
                    break;
                }

                abort(L"Unknown Argument Error: 引数の型を判別出来ませんでした `");
            }
        }

        for(auto as : args) {
            if(as.parsed == 0 && as.item == ITEM::REQUIRED)
                abort(L"Argument Error: 必須の `", as.shortarg, L"` 引数指定がされていません\n");
        }

        for(int i = done + 1; i != argc; ++i) {
            auto v = argv[i];
            if(v[0] == '-')
                printerr(L"Argument Error: 入力された引数 `", v, L"` は定義されてない不明な引数です\n");
            if(allow_postional_argc + done + 1 < i)
                abort(L"Argument Error: 引数の最大値を超えました。\n位置指定引数は ", allow_postional_argc, L" 個までです\n");
            
            positional_argv.emplace_back(v);
        }
        size_ = argc - done;
    }
};

template <typename CharT>
_ArgParser<CharT> ArgParser(int org_argc, CharT** org_argv) {
    return {org_argc, org_argv};
}

template <typename CharT>
_ArgParser<CharT> ArgParser(const CharT* _desc, int org_argc, CharT** org_argv) {
    return {_desc, org_argc, org_argv};
}

#endif /* _ARGPARSE_HPP_ */
