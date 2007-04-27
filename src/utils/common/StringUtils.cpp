/****************************************************************************/
/// @file    StringUtils.cpp
/// @author  Daniel Krajzewicz
/// @date    unknown
/// @version $Id$
///
// Some static methods for string processing
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// copyright : (C) 2001-2007
//  by DLR (http://www.dlr.de/) and ZAIK (http://www.zaik.uni-koeln.de/AFS)
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/
// ===========================================================================
// compiler pragmas
// ===========================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif


// ===========================================================================
// included modules
// ===========================================================================
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>
#include <utils/common/UtilExceptions.h>
#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include "StringUtils.h"

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;


// ===========================================================================
// static member definitions
// ===========================================================================
std::string StringUtils::emptyString;


// ===========================================================================
// method definitions
// ===========================================================================
std::string
StringUtils::prune(std::string str)
{
    size_t idx = str.find_first_not_of(" ");
    if (idx!=string::npos) {
        str = str.substr(idx);
    }
    idx = str.find_last_not_of(" ");
    if (idx!=string::npos&&idx!=str.length()-1) {
        str = str.substr(0, idx+1);
    }
    return str;
}


std::string
StringUtils::to_lower_case(std::string str)
{
    for (size_t i=0; i<str.length(); i++) {
        if (str[i]>='A'&&str[i]<='Z') {
            str[i] = str[i] + 'a' - 'A';
        }
    }
    return str;
}


std::string
StringUtils::version1(std::string str)
{
    size_t idx = str.rfind('_');
    if (idx!=string::npos&&idx<str.length()-1&&(str.substr(idx+1)!="+"&&str.substr(idx+1)!="-")) {
        try {
            int no = TplConvert<char>::_2int(str.substr(idx+1).c_str());
            str = str.substr(0, idx+1) + toString<int>(no+1);
        } catch (NumberFormatException) {
            str = str + "_0";
        }
    } else {
        str = str + "_0";
    }
    return str;
}


std::string
StringUtils::convertUmlaute(std::string str)
{
    str = replace(str, "�", "ae");
    str = replace(str, "�", "Ae");
    str = replace(str, "�", "oe");
    str = replace(str, "�", "Oe");
    str = replace(str, "�", "ue");
    str = replace(str, "�", "Ue");
    str = replace(str, "�", "ss");
    str = replace(str, "�", "E");
    str = replace(str, "�", "e");
    str = replace(str, "�", "E");
    str = replace(str, "�", "e");
    return str;
}



std::string
StringUtils::replace(std::string str, const char *what,
                     const char *by)
{
    string what_tmp(what);
    string by_tmp(by);
    size_t idx = str.find(what);
    size_t what_len = what_tmp.length();
    size_t by_len = by_tmp.length();
    while (idx!=string::npos) {
        str = str.replace(idx, what_len, by);
        idx = str.find(what, idx+by_len-what_len);
    }
    return str;
}


void
StringUtils::upper(std::string &str)
{
    for (size_t i=0; i<str.length(); i++) {
        if (str[i]>='a'&&str[i]<='z') {
            str[i] = str[i] - 'a' + 'A';
        }
    }
}


std::string
StringUtils::toTimeString(int time)
{
    std::ostringstream oss;
    char buffer[4];
    sprintf(buffer, "%02i:",(time/3600));
    oss << buffer;
    time=time%3600;
    sprintf(buffer, "%02i:",(time/60));
    oss << buffer;
    time=time%60;
    sprintf(buffer, "%02i", time);
    oss << buffer;
    return oss.str();
}



/****************************************************************************/

