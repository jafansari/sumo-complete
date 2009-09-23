/****************************************************************************/
/// @file    NIImporter_OpenDrive.cpp
/// @author  Daniel Krajzewicz
/// @date    Mon, 14.04.2008
/// @version $Id: NIImporter_OpenDrive.cpp 7615 2009-08-06 23:59:51Z behrisch $
///
// Importer for networks stored in openDrive format
/****************************************************************************/
// SUMO, Simulation of Urban MObility; see http://sumo.sourceforge.net/
// Copyright 2001-2009 DLR (http://www.dlr.de/) and contributors
/****************************************************************************/
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif
#include <string>
#include <utils/xml/SUMOSAXHandler.h>
#include <utils/common/UtilExceptions.h>
#include <utils/common/TplConvert.h>
#include <utils/common/ToString.h>
#include <utils/common/MsgHandler.h>
#include <netbuild/NBEdge.h>
#include <netbuild/NBEdgeCont.h>
#include <netbuild/NBNode.h>
#include <netbuild/NBNodeCont.h>
#include <netbuild/NBNetBuilder.h>
#include <utils/xml/SUMOXMLDefinitions.h>
#include "NIImporter_OpenDrive.h"
#include <utils/geom/GeoConvHelper.h>
#include <utils/geom/GeomConvHelper.h>
#include <utils/options/OptionsCont.h>
#include <utils/common/FileHelpers.h>
#include <utils/xml/XMLSubSys.h>

#ifdef CHECK_MEMORY_LEAKS
#include <foreign/nvwa/debug_new.h>
#endif // CHECK_MEMORY_LEAKS


// ===========================================================================
// used namespaces
// ===========================================================================
using namespace std;



#define C_LENGTH 10.

// ===========================================================================
// method definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// static methods (interface in this case)
// ---------------------------------------------------------------------------
void
NIImporter_OpenDrive::loadNetwork(const OptionsCont &oc, NBNetBuilder &nb) {
    // check whether the option is set (properly)
    if (!oc.isUsableFileList("opendrive")) {
        return;
    }
    // build the handler
    std::vector<OpenDriveEdge> edges;
    NIImporter_OpenDrive handler(nb.getNodeCont(), edges);
    // parse file(s)
    vector<string> files = oc.getStringVector("opendrive");
    for (vector<string>::const_iterator file=files.begin(); file!=files.end(); ++file) {
        if (!FileHelpers::exists(*file)) {
            MsgHandler::getErrorInstance()->inform("Could not open opendrive file '" + *file + "'.");
            // !!! clean up
            return;
        }
        handler.setFileName(*file);
        MsgHandler::getMessageInstance()->beginProcessMsg("Parsing opendrive from '" + *file + "'...");
        XMLSubSys::runParser(handler, *file);
        MsgHandler::getMessageInstance()->endProcessMsg("done.");
    }
    // convert geometries into a disretised representation
    for (std::vector<OpenDriveEdge>::iterator i=edges.begin(); i!=edges.end(); ++i) {
        OpenDriveEdge &e = *i;
        for (std::vector<OpenDriveGeometry>::iterator j=e.geometries.begin(); j!=e.geometries.end(); ++j) {
            OpenDriveGeometry &g = *j;
            std::vector<Position2D> geom;
            switch(g.type) {
            case OPENDRIVE_GT_UNKNOWN:
                break;
            case OPENDRIVE_GT_LINE:
                geom = geomFromLine(e, g);
                break;
            case OPENDRIVE_GT_SPIRAL:
                geom = geomFromSpiral(e, g);
                break;
            case OPENDRIVE_GT_ARC:
                geom = geomFromArc(e, g);
                break;
            case OPENDRIVE_GT_POLY3:
                geom = geomFromPoly(e, g);
                break;
            default:
                break;
            }
            for(std::vector<Position2D>::iterator k=geom.begin(); k!=geom.end(); ++k) {
                e.geom.push_back_noDoublePos(*k);
            }
        }
        for(unsigned int j=0; j<e.geom.size(); ++j) {
            if(!GeoConvHelper::x2cartesian(e.geom[j])) {
                MsgHandler::getErrorInstance()->inform("Unable to project coordinates for.");
            }
        }
    }
    // build start/end nodes
    for (std::vector<OpenDriveEdge>::iterator i=edges.begin(); i!=edges.end(); ++i) {
        OpenDriveEdge &e = *i;
        for (std::vector<OpenDriveLink>::iterator j=e.links.begin(); j!=e.links.end(); ++j) {
            OpenDriveLink &l = *j;
            std::string nid;
            Position2D pos;
            // !!! check other cases
            if (l.elementType==OPENDRIVE_ET_ROAD) {
                if (l.linkType==OPENDRIVE_LT_SUCCESSOR) {
                    nid = e.id + "_to_" + l.elementID;
                    pos = e.geom[e.geom.size()-1];//Position2D(e.geometries[e.geometries.size()-1].x, e.geometries[e.geometries.size()-1].y);
                } else {
                    nid = l.elementID + "_to_" + e.id;
                    pos = e.geom[0];//Position2D(e.geometries[0].x, e.geometries[0].y);
                }
            } else {
                nid = l.elementID;
                if (l.linkType==OPENDRIVE_LT_SUCCESSOR) {
                    pos = e.geom[e.geom.size()-1];//Position2D(e.geometries[e.geometries.size()-1].x, e.geometries[e.geometries.size()-1].y);
                } else {
                    pos = e.geom[0];//Position2D(e.geometries[0].x, e.geometries[0].y);
                }
            }
            NBNode *n = getOrBuildNode(nid, pos, nb.getNodeCont());
            if (l.linkType==OPENDRIVE_LT_SUCCESSOR) {
                if (e.to!=0&&e.to!=n) {
                    throw ProcessError("Edge '" + e.id + "' has two ending nodes.");
                }
                e.to = n;
            } else {
                if (e.from!=0&&e.from!=n) {
                    throw ProcessError("Edge '" + e.id + "' has two starting nodes.");
                }
                e.from = n;
            }
        }
        if(e.from==0) {
            std::string nid = e.id + ".begin";
            Position2D pos(e.geometries[0].x, e.geometries[0].y);
            e.from = getOrBuildNode(nid, e.geom[0], nb.getNodeCont());
        }
        if(e.to==0) {
            std::string nid = e.id + ".end";
            Position2D pos(e.geometries[e.geometries.size()-1].x, e.geometries[e.geometries.size()-1].y);
            e.to = getOrBuildNode(nid, e.geom[e.geom.size()-1], nb.getNodeCont());
        }
    }
    // build edges
    for (std::vector<OpenDriveEdge>::iterator i=edges.begin(); i!=edges.end(); ++i) {
        OpenDriveEdge &e = *i;
        SUMOReal speed = nb.getTypeCont().getDefaultSpeed();
        int priority = nb.getTypeCont().getDefaultPriority();
        unsigned int nolanes = e.getMaxLaneNumber(SUMO_TAG_OPENDRIVE_RIGHT);
        if(nolanes>0) {
            NBEdge *nbe = new NBEdge(e.id, e.from, e.to, "", speed, nolanes, priority, e.geom, NBEdge::LANESPREAD_RIGHT);
            if (!nb.getEdgeCont().insert(nbe)) {
                throw ProcessError("Could not add edge '" + e.id + "'.");
            }
        }
        nolanes = e.getMaxLaneNumber(SUMO_TAG_OPENDRIVE_LEFT);
        if(nolanes>0) {
            NBEdge *nbe = new NBEdge("-" + e.id, e.to, e.from, "", speed, nolanes, priority, e.geom.reverse(), NBEdge::LANESPREAD_RIGHT);
            if (!nb.getEdgeCont().insert(nbe)) {
                throw ProcessError("Could not add edge '" + std::string("-") + e.id + "'.");
            }
        }
    }
    // build connections
    for (std::vector<OpenDriveEdge>::iterator i=edges.begin(); i!=edges.end(); ++i) {
        OpenDriveEdge &e = *i;
        for (std::vector<OpenDriveLink>::iterator j=e.links.begin(); j!=e.links.end(); ++j) {
            OpenDriveLink &l = *j;
        }
    }
}


NBNode *
NIImporter_OpenDrive::getOrBuildNode(const std::string &id, Position2D &pos, 
                                     NBNodeCont &nc) throw(ProcessError)
{
    if (nc.retrieve(id)==0) {
        // not yet built; build now
        if (!nc.insert(id, pos)) {
            // !!! clean up
            throw ProcessError("Could not add node '" + id + "'.");
        }
    }
    return nc.retrieve(id);
}


std::vector<Position2D> 
NIImporter_OpenDrive::geomFromLine(const OpenDriveEdge &e, const OpenDriveGeometry &g) throw()
{
    std::vector<Position2D> ret;
    ret.push_back(Position2D(g.x, g.y));
    ret.push_back(calculateStraightEndPoint(g.hdg, g.length, Position2D(g.x, g.y)));
    return ret;
}


std::vector<Position2D> 
NIImporter_OpenDrive::geomFromSpiral(const OpenDriveEdge &e, const OpenDriveGeometry &g) throw()
{
    std::vector<Position2D> ret;
    SUMOReal curveStart = g.params[0];
    SUMOReal curveEnd = g.params[1];
    SUMOReal start_x = g.x;
    SUMOReal start_y = g.y;
    SUMOReal start_Angle = g.hdg;
    SUMOReal length = g.length;

    SUMOReal curve;
    bool left;
    bool first_spiral;
    SUMOReal dist, tmpx, tmpy, tmphdg;
	if(fabs(curveEnd) > fabs(curveStart)) {
		curve = curveEnd;
		//Bestimmen der Richtung
		// wird bei der Berechnung der Endkoordinate des neuen Kurven-Segmentes ben�tigt
        if(curveEnd > 0.0) {
			left = true;
        } else {
			left = false;
        }
		first_spiral = true;
		dist = 0.0;
		tmpx = start_x;
		tmpy = start_y;
		tmphdg = start_Angle;
		//Berechnung des Startpunkts der Klothoide, bei der Start- und Endkr�mmung != 0 sind
		if(fabs(curveStart) != 0.0) {
			//Berechnung der L�nge bis Kr�mmung 0
			SUMOReal r1 = 1. / fabs(curveStart);
			SUMOReal r2 = 1. / fabs(curveEnd);
			dist = (r2 * length) / (r1 - r2);
			//Berechnung des ersten Punkts
			start_Angle += PI;
			calculateFirstClothoidPoint(&start_x, &start_y, &start_Angle, curveStart*-1. , dist);
			start_Angle -= PI;
			length += dist;
		}
	} else {
		if(curveEnd == 0.0) {
			curve = curveStart;
			calculateFirstClothoidPoint(&start_x, &start_y, &start_Angle, curve, length);
			start_Angle += PI;
			//Bestimmen der Richtung
			// wird bei der Berechnung der Endkoordinate des neuen Kurven-Segmentes ben�tigt
            if(curveStart > 0.0) {
				left = false;
            } else {
				left = true;
            }
			first_spiral = false;
			dist = length;
			tmpx = g.x;
			tmpy = g.y;
            tmphdg = g.hdg;
        } else {
			return ret;
        }
	}

	SUMOReal end_x, end_y, endZ, end_Angle;
	SUMOReal geo_posS = g.s;
	SUMOReal geo_posE = g.s;
	bool end = false;
	do {
		//setzen der Startkoordinate der original Klothoide
		//bei Berechnung der Endkoordinate wird immer vom original Startpunkt der Klothoide ausgegangen
		end_x = start_x;
		end_y = start_y;
		end_Angle = start_Angle;	
		geo_posE += C_LENGTH;
        if(geo_posE - g.s > g.length) {
			geo_posE = g.s + g.length;
		}
		//Bestimmen der Entfernung zum zu berechnenden Punkts
        if(first_spiral) {
			dist += (geo_posE - geo_posS);
        } else {
			dist -= (geo_posE - geo_posS);
        }
		//Berechnen der XY-Koordinate
		if(dist > 0.0) {
			calculateClothoidProperties(&end_x, &end_y, &end_Angle, curve, length, dist, left);
		} else {
			//Bei der Klothoide Start != 0 entspricht der Startwert dem zu berechnenden Endwert
			end_x = start_x;
			end_y = start_y;
			end_Angle = end_Angle - PI - PI;	
		}
        if(!first_spiral) {
			end_Angle += PI;
        }
        //
        ret.push_back(Position2D(tmpx, tmpy));
        //
		geo_posS = geo_posE;
		tmpx = end_x;
		tmpy = end_y;
		tmphdg = end_Angle;
		if (geo_posE  - (g.s + g.length) < 0.001 && geo_posE  - (g.s + g.length) > -0.001) {
			end = true;
		}
	} while(!end);
    return ret;
}


std::vector<Position2D> 
NIImporter_OpenDrive::geomFromArc(const OpenDriveEdge &e, const OpenDriveGeometry &g) throw()
{
    std::vector<Position2D> ret;
	SUMOReal dist = 0.0;
	SUMOReal centerX = g.x;
	SUMOReal centerY = g.y;
	//Positiv Linkskurve; Negativ Rechtskurve
    SUMOReal curvature = g.params[0];
    SUMOReal radius = 1. / curvature;
	// Berechnung des Mittelpunktes
    calculateCurveCenter(&centerX, &centerY, radius, g.hdg);
	SUMOReal endX = g.x;
	SUMOReal endY = g.y;
	SUMOReal startX = g.x;
	SUMOReal startY = g.y;
    SUMOReal hdgS = g.hdg;
	SUMOReal hdgE;
	SUMOReal geo_posS = g.s;
	SUMOReal geo_posE = g.s;
	int index1 = 0;
	bool end = false;
	do {
		SUMOReal startds, endds;
		geo_posE += C_LENGTH;
		// Berechnung eines Punktes auf der Kurve abh�ngig von der L�nge (Konstante)
        if(geo_posE - g.s > g.length) {
			geo_posE = g.s + g.length;
		}
        calcPointOnCurve(&endX, &endY, centerX, centerY, radius, geo_posE - geo_posS);
		//Berechnen des Richtungswinkels des Berechneten Punktes
		dist += (geo_posE - geo_posS);
		if(curvature > 0.0) {
            hdgE = g.hdg + dist/fabs(radius);
        } else {
			hdgE = g.hdg - dist/fabs(radius);
        }
        //
        ret.push_back(Position2D(startX, startY));		
        //
		startX = endX;
        startY = endY;
		geo_posS = geo_posE;
		hdgS = hdgE;
		
        if (geo_posE  - (g.s + g.length) < 0.001 && geo_posE  - (g.s + g.length) > -0.001) {
			end = true;
		}
	}while(!end);
    return ret;
}


std::vector<Position2D> 
NIImporter_OpenDrive::geomFromPoly(const OpenDriveEdge &e, const OpenDriveGeometry &g) throw()
{
    std::vector<Position2D> ret;
    return ret;
}


Position2D
NIImporter_OpenDrive::calculateStraightEndPoint(double hdg, double length, const Position2D &start) throw() {
	double normx = 1.0f;
	double normy = 0.0f;
	double x2 = normx * cos(hdg) - normy * sin(hdg);
	double y2 = normx * sin(hdg) + normy * cos(hdg); 
	normx = x2 * length;
	normy = y2 * length;
	return Position2D(start.x() + normx, start.y() + normy);
}


void
NIImporter_OpenDrive::calculateClothoidProperties(SUMOReal *x, SUMOReal *y, SUMOReal *hdg, SUMOReal curve, 
									   SUMOReal length, SUMOReal dist, bool direction) throw() {
	
	double xtmp, ytmp;
	//Radius am Ende der Klothoide (Kreisradius)
	double r = abs(1 / curve);
	// 'A' Klothoiden-Parameter,immer Konstant
	double a = sqrt(length * r);
	// Radius am zum Berechneneden Endpunkt der Klothoide
	r = pow(a, 2) / dist;
	double l = a / r;
	// Tangenten Winkel = Richtungswinkel des Endpunktes
	double new_hdg = dist/(2 * (pow(a, 2)/dist));
	
	double omega_hdg;
	double lengthS;
	//Berechnung aus RAS-L (Richtlinien f�r die Anlage von Stra�en Teil Linienf�hrung)
	xtmp = a *( l - powl(l, 5.0)/40.0 + powl(l, 9.0)/3456.0 - powl(l, 13.0)/599040.0 + powl(l, 17.0)/175472640.0);
	ytmp = a * ( powl(l, 3.0)/6.0 - powl(l, 7.0)/366.0 + powl(l, 11.0)/42240.0 - powl(l, 15.0)/9767800.0);
	// Falls Rechtskurve -> Spiegeln der Klothoide an der X achse (X bleibt / Y * (-1))
	if(!direction)
		ytmp *= (-1);
	//Richtungswibnkel der Sehne
	omega_hdg = atan(ytmp/xtmp);
	//L�nge der Sehne
	lengthS = sqrt(pow(xtmp,2) + pow(ytmp,2));

	long double tmp;
	tmp = xtmp;
	//Rotieren umd den Richtungswinkel des Startpunkts
	xtmp = xtmp * cos(*hdg) - ytmp * sin(*hdg);
	ytmp = tmp * sin(*hdg) + ytmp * cos(*hdg); 
 
	// R�ckgabe der End-Koordinaten
	*x = *x + xtmp;
	*y = *y + ytmp;
	// R�ckgabe des Richtungswinkels abh�ngig von der Richtung der Kurve
	if(direction)
		*hdg = new_hdg +*hdg;
	else
		*hdg = *hdg - new_hdg;
}
//Berechnung des Startpunktes der Klothoide
//Dabei wird zun�chst die normale Klothoide Berechnet
//Dannach wird der Endpunkt auf den Koordinatenursprung verschoben
//Und zum Schluss der Startpunkt anhand der Sehne (Start- <-> Endpunkt) berechnet
void 
NIImporter_OpenDrive::calculateFirstClothoidPoint(SUMOReal* ad_X, SUMOReal* ad_Y, SUMOReal* ad_hdg, 
						SUMOReal ad_curvature, SUMOReal ad_lengthE) throw()
{
	//Entscheidung Links oder Rechtskurve
	//notwendig, da bei einer Linkskurve die Normale-Klothoide gespiegelt werden muss
	//BERECHNUNG R�CKW�RTS
	long double reflect;
	//Umgekehrt da Klothoide vom Ende betrachtet wird
	if(ad_curvature > 0.0)
		reflect = -1.0;
	else
	{
		reflect = 1.0;
	}
	//Klothoiden Parameter
	long double A = sqrt(ad_lengthE * 1/abs(ad_curvature)); 	
	long double xE, yE;
	long double rE = pow(A,2)/ad_lengthE;
	long double lE = A / rE;
	//Berechnung der Endpunkte der Normalen-Klothoide anhand von A
	xE = A *( lE - powl(lE, 5.0)/40.0 + powl(lE, 9.0)/3456.0 - powl(lE, 13.0)/599040.0 + powl(lE, 17.0)/175472640.0);
	yE = reflect* A * ( powl(lE, 3.0)/6.0 - powl(lE, 7.0)/366.0 + powl(lE, 11.0)/42240.0 - powl(lE, 15.0)/9767800.0);
	//Berechnung des Tangentenwinkels der Normalen-Klotoide
	long double lok_hdg = pow(A, 2)/ (2 * pow(rE,2));
	//Richtungswinkel des Startpunktes
	long double new_hdg;
	if(reflect > 0.0)
		new_hdg = *ad_hdg - lok_hdg;
	else
		new_hdg = *ad_hdg + lok_hdg;

	//Abfangen der negativen Winkel-Werte
	if(new_hdg <0.0)
		new_hdg = 2 * PI + new_hdg;
	// Berechnung der Sehhne zw. Startpunkt und Endpunkt
	long double S = sqrt(pow(yE, 2)+ pow(xE, 2));
	//Richtungswinkel der Sehne
	long double omega = atan(yE/xE);
	
	//******* VEKTORRECHNUNG ************

	long double normxS = 1.0;
	long double normyS = 0.0;

	long double tmp;
	
	tmp = normxS;
	normxS = normxS * cos(PI + new_hdg + omega) - normyS * sin(PI + new_hdg + omega);
	normyS = tmp * sin(PI + new_hdg + omega) + normyS * cos(PI + new_hdg + omega);
	//Startpunkt der normalen Klothoide versetzt vom Koordinatenurprung
	//anhand der Sehne S
	normxS *= S;
	normyS *= S;
	
	//R�ckgabe
	*ad_X -= normxS;
	*ad_Y -= normyS;
	*ad_hdg = new_hdg;
}


//Berechnung des Mittelpunkts einer Kurve
void
NIImporter_OpenDrive::calculateCurveCenter(SUMOReal *ad_x, SUMOReal *ad_y, SUMOReal ad_radius, SUMOReal ad_hdg) throw()
{
	SUMOReal normX = 1.0;
	SUMOReal normY = 0.0;
	SUMOReal tmpX;
	SUMOReal turn;
	if(ad_radius > 0)
		turn = -1.0;
	else
		turn = 1.0;
	//Rotation um den Richtungswinkel (KEINE BEACHTUNG DER RICHTUNG !!!!!)
	tmpX = normX;
	normX = normX * cos(ad_hdg) + normY * sin(ad_hdg);
	normY = tmpX * sin(ad_hdg) + normY * cos(ad_hdg);
	
	//Rotation um 90� (Beachtung der Richtung)
	tmpX = normX;
	normX = normX * cos(90 * PI / 180) + turn * normY * sin(90 * PI / 180);
	normY = -1 * turn * tmpX * sin(90 * PI / 180) + normY * cos(90 * PI / 180);

	//Verl�ngern um den Radius
	normX = abs(ad_radius) * normX;
	normY = abs(ad_radius) * normY;
	
	//Verschieben der Punkte
	*ad_x += normX;
	*ad_y += normY;
}

//Berechnung eines Punktes auf der Kurve in Abh�ngigkeit von ad_length
void
NIImporter_OpenDrive::calcPointOnCurve(SUMOReal *ad_x, SUMOReal *ad_y, SUMOReal ad_centerX, SUMOReal ad_centerY, 
						   SUMOReal ad_r, SUMOReal ad_length) throw()
{
	//Mittelpunktswinkel
	double rotAngle = ad_length/abs(ad_r);
	//Vektor vom Mittelpunkt zum Startpunkt
	double vx = *ad_x - ad_centerX;
	double vy = *ad_y - ad_centerY;
	double tmpx;

	double turn;
	if(ad_r > 0)
		turn = -1; //Links
	else
		turn = 1; //Rechts
	tmpx = vx;
	//Rotation (Beachtung der Richtung)
	vx = vx * cos(rotAngle) + turn * vy * sin(rotAngle);
	vy = -1 * turn * tmpx * sin(rotAngle) + vy * cos(rotAngle);
	//Verschiebung
	*ad_x = vx + ad_centerX;
	*ad_y = vy + ad_centerY;
}

// ---------------------------------------------------------------------------
// loader methods
// ---------------------------------------------------------------------------
NIImporter_OpenDrive::NIImporter_OpenDrive(NBNodeCont &nc, std::vector<OpenDriveEdge> &edgeCont)
        : SUMOSAXHandler("opendrive"), myCurrentEdge("", "", -1),
        myEdges(edgeCont) {}


NIImporter_OpenDrive::~NIImporter_OpenDrive() throw() {
}


void
NIImporter_OpenDrive::myStartElement(SumoXMLTag element,
                                     const SUMOSAXAttributes &attrs) throw(ProcessError) {
    bool ok = true;
    switch (element) {
    case SUMO_TAG_OPENDRIVE_HEADER: {
        int majorVersion = attrs.getIntReporting(SUMO_ATTR_OPENDRIVE_REVMAJOR, "opendrive header", 0, ok);
        int minorVersion = attrs.getIntReporting(SUMO_ATTR_OPENDRIVE_REVMINOR, "opendrive header", 0, ok);
        if (majorVersion!=1||minorVersion!=2) {
            MsgHandler::getWarningInstance()->inform("Given openDrive file '" + getFileName() + "' uses version " + toString(majorVersion) + "." + toString(minorVersion) + ";\n Version 1.2 is supported.");
        }
    }
    break;
    case SUMO_TAG_OPENDRIVE_ROAD: {
        std::string id =
            attrs.hasAttribute(SUMO_ATTR_OPENDRIVE_ID)
            ? attrs.getStringReporting(SUMO_ATTR_OPENDRIVE_ID, "road", 0, ok)
            : attrs.getStringReporting(SUMO_ATTR_ID, "road", 0, ok);
        cout << "found edge '" << id << "'" << endl;
        std::string junction = attrs.getStringReporting(SUMO_ATTR_OPENDRIVE_JUNCTION, "road", id.c_str(), ok);
        SUMOReal length = attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_LENGTH, "road", id.c_str(), ok);
        myCurrentEdge = OpenDriveEdge(id, junction, length);
    }
    break;
    case SUMO_TAG_OPENDRIVE_PREDECESSOR: {
        if(myElementStack[myElementStack.size()-2]==SUMO_TAG_OPENDRIVE_ROAD) {
            std::string elementType = attrs.getStringReporting(SUMO_ATTR_OPENDRIVE_ELEMENTTYPE, "predecessor", myCurrentEdge.id.c_str(), ok);
            std::string elementID = attrs.getStringReporting(SUMO_ATTR_OPENDRIVE_ELEMENTID, "predecessor", myCurrentEdge.id.c_str(), ok);
            std::string contactPoint = attrs.hasAttribute(SUMO_ATTR_OPENDRIVE_CONTACTPOINT)
                ? attrs.getStringReporting(SUMO_ATTR_OPENDRIVE_CONTACTPOINT, "predecessor", myCurrentEdge.id.c_str(), ok)
                : "end";
            addLink(OPENDRIVE_LT_PREDECESSOR, elementType, elementID, contactPoint);
        }
    }
    break;
    case SUMO_TAG_OPENDRIVE_SUCCESSOR: {
        if(myElementStack[myElementStack.size()-2]==SUMO_TAG_OPENDRIVE_ROAD) {
            std::string elementType = attrs.getStringReporting(SUMO_ATTR_OPENDRIVE_ELEMENTTYPE, "successor", myCurrentEdge.id.c_str(), ok);
            std::string elementID = attrs.getStringReporting(SUMO_ATTR_OPENDRIVE_ELEMENTID, "successor", myCurrentEdge.id.c_str(), ok);
            std::string contactPoint = attrs.hasAttribute(SUMO_ATTR_OPENDRIVE_CONTACTPOINT)
                ? attrs.getStringReporting(SUMO_ATTR_OPENDRIVE_CONTACTPOINT, "successor", myCurrentEdge.id.c_str(), ok)
                : "start";
            addLink(OPENDRIVE_LT_SUCCESSOR, elementType, elementID, contactPoint);
        }
    }
    break;
    case SUMO_TAG_OPENDRIVE_GEOMETRY: {
        SUMOReal length = attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_LENGTH, "geometry", myCurrentEdge.id.c_str(), ok);
        SUMOReal s = attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_S, "geometry", myCurrentEdge.id.c_str(), ok);
        SUMOReal x = attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_X, "geometry", myCurrentEdge.id.c_str(), ok);
        SUMOReal y = attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_Y, "geometry", myCurrentEdge.id.c_str(), ok);
        SUMOReal hdg = attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_HDG, "geometry", myCurrentEdge.id.c_str(), ok);
        myCurrentEdge.geometries.push_back(OpenDriveGeometry(length, s, x, y, hdg));
    }
    break;
    case SUMO_TAG_OPENDRIVE_LINE: {
        std::vector<SUMOReal> vals;
        addGeometryShape(OPENDRIVE_GT_LINE, vals);
    }
    break;
    case SUMO_TAG_OPENDRIVE_SPIRAL: {
        std::vector<SUMOReal> vals;
        vals.push_back(attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_CURVSTART, "spiral", myCurrentEdge.id.c_str(), ok));
        vals.push_back(attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_CURVEND, "spiral", myCurrentEdge.id.c_str(), ok));
        addGeometryShape(OPENDRIVE_GT_SPIRAL, vals);
    }
    break;
    case SUMO_TAG_OPENDRIVE_ARC: {
        std::vector<SUMOReal> vals;
        vals.push_back(attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_CURVATURE, "arc", myCurrentEdge.id.c_str(), ok));
        addGeometryShape(OPENDRIVE_GT_ARC, vals);
    }
    break;
    case SUMO_TAG_OPENDRIVE_POLY3: {
        std::vector<SUMOReal> vals;
        vals.push_back(attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_A, "poly3", myCurrentEdge.id.c_str(), ok));
        vals.push_back(attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_B, "poly3", myCurrentEdge.id.c_str(), ok));
        vals.push_back(attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_C, "poly3", myCurrentEdge.id.c_str(), ok));
        vals.push_back(attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_D, "poly3", myCurrentEdge.id.c_str(), ok));
        addGeometryShape(OPENDRIVE_GT_POLY3, vals);
    }
    break;
    case SUMO_TAG_OPENDRIVE_LANESECTION: {
        SUMOReal s = attrs.getSUMORealReporting(SUMO_ATTR_OPENDRIVE_S, "geometry", myCurrentEdge.id.c_str(), ok);
        myCurrentEdge.laneSections.push_back(OpenDriveLaneSection(s));
    }
    break;
    case SUMO_TAG_OPENDRIVE_LEFT:
    case SUMO_TAG_OPENDRIVE_CENTER:
    case SUMO_TAG_OPENDRIVE_RIGHT:
        myCurrentLaneDirection = element;
        break;
    case SUMO_TAG_LANE: // !!!
    case SUMO_TAG_OPENDRIVE_LANE: {
        std::string type = attrs.getStringReporting(SUMO_ATTR_OPENDRIVE_TYPE, "lane", myCurrentEdge.id.c_str(), ok);
        int id = attrs.hasAttribute(SUMO_ATTR_OPENDRIVE_ID)
            ? attrs.getIntReporting(SUMO_ATTR_OPENDRIVE_ID, "lane", myCurrentEdge.id.c_str(), ok)
            : attrs.getIntReporting(SUMO_ATTR_ID, "lane", myCurrentEdge.id.c_str(), ok);
        int level = attrs.getIntReporting(SUMO_ATTR_OPENDRIVE_LEVEL, "lane", myCurrentEdge.id.c_str(), ok);
        OpenDriveLaneSection &ls = myCurrentEdge.laneSections[myCurrentEdge.laneSections.size()-1];
        switch(myCurrentLaneDirection) {
        case SUMO_TAG_OPENDRIVE_LEFT:
            ls.lanesByDir[SUMO_TAG_OPENDRIVE_LEFT].push_back(OpenDriveLane(id, level, type));
            break;
        case SUMO_TAG_OPENDRIVE_CENTER:
            ls.lanesByDir[SUMO_TAG_OPENDRIVE_CENTER].push_back(OpenDriveLane(id, level, type));
            break;
        case SUMO_TAG_OPENDRIVE_RIGHT:
            ls.lanesByDir[SUMO_TAG_OPENDRIVE_RIGHT].push_back(OpenDriveLane(id, level, type));
            break;
        }
    }
    default:
        break;
    }
    myElementStack.push_back(element);
}


void
NIImporter_OpenDrive::myCharacters(SumoXMLTag element,
                                   const std::string &chars) throw(ProcessError) {
}



void
NIImporter_OpenDrive::myEndElement(SumoXMLTag element) throw(ProcessError) {
    myElementStack.pop_back();
    switch (element) {
    case SUMO_TAG_OPENDRIVE_ROAD:
        myEdges.push_back(myCurrentEdge);
        break;
    default:
        break;
    }
}



void
NIImporter_OpenDrive::addLink(LinkType lt, const std::string &elementType,
                              const std::string &elementID,
                              const std::string &contactPoint) throw(ProcessError) {
    OpenDriveLink l(lt, elementID);
    // elementType
    if (elementType=="road") {
        l.elementType = OPENDRIVE_ET_ROAD;
    } else if (elementType=="junction") {
        l.elementType = OPENDRIVE_ET_JUNCTION;
    }
    // contact point
    if (contactPoint=="start") {
        l.contactPoint = OPENDRIVE_CP_START;
    } else if (contactPoint=="end") {
        l.contactPoint = OPENDRIVE_CP_END;
    }
    // add
    myCurrentEdge.links.push_back(l);
}


void
NIImporter_OpenDrive::addGeometryShape(GeometryType type, const std::vector<SUMOReal> &vals) throw(ProcessError) {
    // checks
    if (myCurrentEdge.geometries.size()==0) {
        throw ProcessError("Mismatching paranthesis in geometry definition for road '" + myCurrentEdge.id + "'");
    }
    OpenDriveGeometry &last = myCurrentEdge.geometries[myCurrentEdge.geometries.size()-1];
    if (last.type!=OPENDRIVE_GT_UNKNOWN) {
        throw ProcessError("Double geometry information for road '" + myCurrentEdge.id + "'");
    }
    // set
    last.type = type;
    last.params = vals;
}


/****************************************************************************/

