/***************************************************************************
                          StringUtils.h  -
                          Some helping function for string processing
                             -------------------
    begin                : unknown
    copyright            : (C) 2003 by Daniel Krajzewicz
    email                : Daniel.Krajzewicz@dlr.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// $Log$
// Revision 1.4  2003/07/30 09:27:40  dkrajzew
// the double-trimming function debugged
//
// Revision 1.3  2003/05/20 09:49:43  dkrajzew
// further work and debugging
//
// Revision 1.2  2003/04/01 15:28:12  dkrajzew
// some further functions added
//
// Revision 1.1  2003/03/20 17:31:45  dkrajzew
// StringUtils moved from utils/importio to utils/common
//
// Revision 1.2  2003/03/20 16:41:10  dkrajzew
// periodical car emission implemented; windows eol removed
//
//
/* =========================================================================
 * included modules
 * ======================================================================= */
#include <string>
#include <utils/common/UtilExceptions.h>
#include <utils/convert/TplConvert.h>
#include <utils/convert/ToString.h>
#include "StringUtils.h"


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
std::string
StringUtils::prune(std::string str) {
    size_t idx = str.find_first_not_of(" ");
    if(idx!=string::npos) {
        str = str.substr(idx);
    }
    idx = str.find_last_not_of(" ");
    if(idx!=string::npos&&idx!=str.length()-1) {
        str = str.substr(0, idx+1);
    }
    return str;
}


std::string
StringUtils::to_lower_case(std::string str)
{
    for(int i=0; i<str.length(); i++) {
        if(str[i]>='A'&&str[i]<='Z') {
            str[i] = str[i] + 'a' - 'A';
        }
    }
    return str;
}


std::string
StringUtils::version1(std::string str)
{
    size_t idx = str.rfind('_');
    if(idx!=string::npos) {
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
StringUtils::to_html(std::string str)
{
    /*
    str = replace(str, "�", "&auml;");
    str = replace(str, "�", "&Auml;");
    str = replace(str, "�", "&ouml;");
    str = replace(str, "�", "&Ouml;");
    str = replace(str, "�", "&uuml;");
    str = replace(str, "�", "&Uuml;");
    str = replace(str, "�", "&szlig;");
    */
    str = replace(str, "�", "ae");
    str = replace(str, "�", "Ae");
    str = replace(str, "�", "oe");
    str = replace(str, "�", "Oe");
    str = replace(str, "�", "ue");
    str = replace(str, "�", "Ue");
    str = replace(str, "�", "ss");
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
    while(idx!=string::npos) {
        str = str.replace(idx, what_len, by);
        idx = str.find(what, idx+by_len-what_len);
    }
    return str;
}


std::string
StringUtils::trim(double val, size_t to)
{
    string ret = toString<double>(val);
    size_t idx = ret.rfind('.');
    if(idx!=string::npos) {
        return ret.substr(0, ret.find('.') + to);
    } else {
        return ret;
    }
}




/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "StringUtils.icc"
//#endif

// Local Variables:
// mode:C++
// End:

