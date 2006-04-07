/***************************************************************************
                          NITigerLoader.cpp
              A loader tiger-files
                             -------------------
    project              : SUMO
    begin                : Tue, 29 Jun 2004
    copyright            : (C) 2004 by DLR/IVF http://ivf.dlr.de/
    author               : Daniel Krajzewicz
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
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.13  2006/04/07 05:28:24  dkrajzew
// removed some warnings
//
// Revision 1.12  2006/03/27 07:30:20  dkrajzew
// added projection information to the network
//
// Revision 1.11  2006/03/17 11:03:06  dkrajzew
// made access to positions in Position2DVector c++ compliant
//
// Revision 1.10  2006/03/08 13:02:27  dkrajzew
// some further work on converting geo-coordinates
//
// Revision 1.9  2006/02/13 07:20:23  dkrajzew
// code beautifying
//
// Revision 1.8  2005/10/07 11:39:57  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.7  2005/09/23 06:02:42  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.6  2005/09/15 12:03:37  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.5  2005/04/27 12:24:36  dkrajzew
// level3 warnings removed; made netbuild-containers non-static
//
// Revision 1.4  2004/11/23 10:23:52  dkrajzew
// debugging
//
// Revision 1.3  2004/08/02 12:44:13  dkrajzew
// using Position2D instead of two SUMOReals
//
// Revision 1.2  2004/07/05 09:32:22  dkrajzew
// false geometry assignment patched
//
// Revision 1.1  2004/07/02 09:34:38  dkrajzew
// elmar and tiger import added
//
/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include <string>
#include <cassert>
#include <utils/common/MsgHandler.h>
#include <utils/common/StringUtils.h>
#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include <utils/options/OptionsCont.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/nodes/NBNode.h>
#include <netbuild/nodes/NBNodeCont.h>
#include <utils/geom/GeomHelper.h>

#include "NITigerLoader.h"

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG


/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * method definitions
 * ======================================================================= */
NITigerLoader::NITigerLoader(NBEdgeCont &ec, NBNodeCont &nc,
                             const std::string &file, projPJ pj)
    : FileErrorReporter("tiger-network", file),
    myWasSet(false), myInitX(-1), myInitY(-1), myEdgeCont(ec), myNodeCont(nc),
    myProjection(pj)
{
}


NITigerLoader::~NITigerLoader()
{
}


void
NITigerLoader::load(OptionsCont &options)
{
    LineReader tgr1r((_file + ".rt1").c_str());
    LineReader tgr2r((_file + ".rt2").c_str());
    if(!tgr1r.good()) {
        MsgHandler::getErrorInstance()->inform("Could not open '" + _file + ".rt1" + "'.");
        throw ProcessError();
    }
    string line1, line2, tmp;
    std::vector<std::string> values2;
    if(tgr2r.good()&&tgr2r.hasMore()) {
        line2 = StringUtils::prune(tgr2r.readLine());
        StringTokenizer st2(line2, StringTokenizer::WHITECHARS);
        values2 = st2.getVector();
    }
    while(tgr1r.hasMore()) {
        string line1 = StringUtils::prune(tgr1r.readLine());
        StringTokenizer st1(line1, StringTokenizer::WHITECHARS);
        std::vector<std::string> values1 = st1.getVector();
        string eid = values1[1];
        std::vector<std::string> poses;
        poses.push_back(values1[values1.size()-2]);
        // check whether any additional information exists
        if(values2.size()!=0&&values2[1]==eid) {
            copy(values2.begin()+3, values2.end(),
                back_inserter(poses));
            if(tgr2r.good()&&tgr2r.hasMore()) {
                line2 = tgr2r.readLine();
                StringTokenizer st2(line2, StringTokenizer::WHITECHARS);
                values2 = st2.getVector();
            }
        }
        poses.push_back(values1[values1.size()-1]);
        assert(poses.size()>1);
        Position2DVector cposes = convertShape(poses);
        assert(cposes.size()>1);
        NBNode *from = getNode(cposes[0]);
        NBNode *to = getNode(cposes[-1]);
        if(from==0||to==0) {
            if(from==0) {
                MsgHandler::getErrorInstance()->inform("Could not find start node of edge '" + eid + "'.");
                throw ProcessError();
            }
            if(to==0) {
                MsgHandler::getErrorInstance()->inform("Could not find end node of edge '" + eid + "'.");
                throw ProcessError();
            }
        }
        // !!!
        std::string type = getType(values1);
        SUMOReal speed = getSpeed(type);
        int nolanes = getLaneNo(type);
        SUMOReal length = cposes.length();
        if(nolanes!=-1&&length>0) {
            int priority = 1;
            NBEdge *e =
                new NBEdge(eid, eid, from, to, type, speed, nolanes,
                    cposes.length(), priority, cposes);
            if(!myEdgeCont.insert(e)) {
                delete e;
                MsgHandler::getErrorInstance()->inform("Could not insert edge '" + eid + "'.");
                throw ProcessError();
            }
            eid = "-" + eid;
            e =
                new NBEdge(eid, eid, to, from, type, speed, nolanes,
                    cposes.length(), priority, cposes.reverse());
            if(!myEdgeCont.insert(e)) {
                delete e;
                MsgHandler::getErrorInstance()->inform("Could not insert edge '" + eid + "'.");
                throw ProcessError();
            }
        }
    }
}


Position2DVector
NITigerLoader::convertShape(const std::vector<std::string> &sv)
{
    Position2DVector ret;
    std::vector<std::string>::const_iterator i;
    for(i=sv.begin(); i!=sv.end(); ++i) {
        string info = *i;
        size_t b1 = info.find_first_of("+-");
        assert(b1!=string::npos);
        size_t b2 = info.find_first_of("+-", b1+1);
        size_t b3 =
            b2==string::npos
            ? string::npos
            : info.find_first_of("+-", b2+1);
        string p1 = info.substr(0, b2);
        string p2 =
            b3==string::npos
            ? info.substr(b2)
            : info.substr(b2, b3-b2);
        try {
            SUMOReal x = TplConvert<char>::_2SUMOReal(p1.c_str());
            SUMOReal y = TplConvert<char>::_2SUMOReal(p2.c_str());

            myNodeCont.addGeoreference(Position2D((SUMOReal) (x / 100000.0), (SUMOReal) (y / 100000.0)));

            projUV p;
            p.u = x / 100000.0 * DEG_TO_RAD;
            p.v = y / 100000.0 * DEG_TO_RAD;
            if(myProjection!=0) {
                p = pj_fwd(p, myProjection);
            } else {
                x = x / (SUMOReal) 100000.0;
                y = y / (SUMOReal) 100000.0;
                SUMOReal ys = y;
                if(!myWasSet) {
                    myWasSet = true;
                    myInitX = x;
                    myInitY = y;
                }
                x = (x-myInitX);
                y = (y-myInitY);
                p.u = (SUMOReal) (x * 111.320*1000.);
                p.v = (SUMOReal) (y * 111.136*1000.);
                p.u *= (SUMOReal) cos(ys*PI/180.0);
            }
            ret.push_back(Position2D((SUMOReal) p.u, (SUMOReal) p.v));
        } catch(NumberFormatException &) {
            MsgHandler::getErrorInstance()->inform(
                "Could not convert position '" + p1 + "/" + p2 + "'.");
            throw ProcessError();
        }
    }
    return ret;
}

int bla = 0;

NBNode *
NITigerLoader::getNode(const Position2D &p)
{
    NBNode *n = myNodeCont.retrieve(p);
    if(n==0) {
        n = new NBNode(toString<int>(bla++), p);
        if(!myNodeCont.insert(n)) {
            MsgHandler::getErrorInstance()->inform("Could not insert node at position " + toString(p.x()) + "/" + toString(p.y()) + ".");
            throw ProcessError();
        }
    }
    return n;
}


std::string
NITigerLoader::getType(const std::vector<std::string> &sv) const
{
    for(std::vector<std::string>::const_iterator i=sv.begin(); i!=sv.end(); ++i) {
        std::string tc = *i;
        // some checks whether it's the type
        if(tc.length()!=3) {
            continue;
        }
        if(tc.find_first_of("ABCDEFGHPX")!=0) {
            continue;
        }
        if(tc[1]<'0'||tc[1]>'9') {
            continue;
        }
        if(tc[2]<'0'||tc[2]>'9') {
            continue;
        }
        // ok, its the type (let's hope)
        return tc;
    }
    MsgHandler::getErrorInstance()->inform("Could not determine type for an edge..."); // !!! be more verbose
    throw ProcessError();
}


SUMOReal
NITigerLoader::getSpeed(const std::string &type) const
{
    switch(type[0]) {
    case 'A':
        switch(type[1]) {
        case '1':
            return (SUMOReal) 85.0 / (SUMOReal) 3.6 * (SUMOReal) 1.6;
        case '2':
            return (SUMOReal) 85.0 / (SUMOReal) 3.6 * (SUMOReal) 1.6;
        case '3':
            return (SUMOReal) 55.0 / (SUMOReal) 3.6 * (SUMOReal) 1.6;
        case '4':
            return (SUMOReal) 35.0 / (SUMOReal) 3.6 * (SUMOReal) 1.6;
        case '5':
            return (SUMOReal) 10.0 / (SUMOReal) 3.6 * (SUMOReal) 1.6;
        default:
            return -1;
        };
        break;
    case 'B':
        return -1; // rail roads
    case 'C':
        return -1; // Miscellaneous ground transport
    case 'D':
        return -1; // landmark
    case 'E':
        return -1; // physical feature
    case 'F':
        return -1; // nonvisible features
    case 'G':
        return -1; // census bureau usage
    case 'H':
        return -1; // hydrography
    case 'P':
        return -1; // ? !!!
    case 'X':
        return -1; // not yet classified
    default:
        throw 1;
    }
}


int
NITigerLoader::getLaneNo(const std::string &type) const
{
    switch(type[0]) {
    case 'A':
        switch(type[1]) {
        case '1':
            return 5;
        case '2':
            return 4;
        case '3':
            return 1;
        case '4':
            return 1;
        case '5':
            return 1;
        default:
            return -1;
        };
        break;
    case 'B':
        return -1; // rail roads
    case 'C':
        return -1; // Miscellaneous ground transport
    case 'D':
        return -1; // landmark
    case 'E':
        return -1; // physical feature
    case 'F':
        return -1; // nonvisible features
    case 'G':
        return -1; // census bureau usage
    case 'H':
        return -1; // hydrography
    case 'P':
        return -1; // ? !!!
    case 'X':
        return -1; // not yet classified
    default:
        throw 1;
    }
}



/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/

// Local Variables:
// mode:C++
// End:

