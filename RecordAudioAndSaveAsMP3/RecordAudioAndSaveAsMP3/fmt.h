#ifndef FMT_H
#define FMT_H

#include <memory>		//std::unique_ptr
#include <locale>         // std::wstring_convert
#include <codecvt>        // std::codecvt_utf8
#include <iostream>
using namespace std;

std::string UnicodeToUTF8(const std::wstring & wstr);

std::wstring UTF8ToUnicode(const std::string & str);

std::string UnicodeToANSI(const std::wstring & wstr);

std::wstring ANSIToUnicode(const std::string & str);

std::string UTF8ToANSI(const std::string & str);

std::string ANSIToUTF8(const std::string & str);

#endif