#ifndef NBDistrict_h
#define NBDistrict_h
//---------------------------------------------------------------------------//
//                        NBDistrict.h -
//  A class representing districts
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
// $Log$
// Revision 1.2  2003/02/07 10:43:44  dkrajzew
// updated
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <utils/common/Named.h>


/* =========================================================================
 * class declarations
 * ======================================================================= */
class NBEdge;


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * NBDistrict
 * A class that represents a district together with incoming and outgoing
 * connections.
 */
class NBDistrict : public Named {
public:
    /** @brief constructor with position */
    NBDistrict(const std::string &id, const std::string &name,
        bool sourceConnectorsWeighted, bool sinkConnectorsWeighted,
        double x, double y);

    /** @brief constructor without position
        The position must be computed later */
    NBDistrict(const std::string &id, const std::string &name,
        bool sourceConnectorsWeighted, bool sinkConnectorsWeighted);

    /// destructor
    ~NBDistrict();

    /// adds a source
    bool addSource(NBEdge *source, double weight=-1);

    /// adds a sink
    bool addSink(NBEdge *sink, double weight=-1);

    /// writes the sumo-xml-representation into the given stream
    void writeXML(std::ostream &into);

    /// returns the district's x-coordinate
    double getXCoordinate() const;

    /// returns the district's y-coordinate
    double getYCoordinate() const;

    /// computes the center of the district
    void computeCenter();

    /** @brief Sets the center coordinates
        Throws an exception when a center was already given */
    void setCenter(double x, double y);

private:
    /// a secondary name
    std::string _name;

    /// definition of a vector of connections to the real network
    typedef std::vector<NBEdge*> ConnectorCont;

    /// definition of a vector of connection weights
    typedef std::vector<double> WeightsCont;

    /// the sources (connection from district to network)
    ConnectorCont _sources;

    /// the weights of the sources
    WeightsCont _sourceWeights;

    /// the sinks (connection from network to district)
    ConnectorCont _sinks;

    /// the weights of the sinks
    WeightsCont _sinkWeights;

    /// information whether the sources are weighted
    bool _sourceConnectorsWeighted;

    /// information whether the sinks are weighted
    bool _sinkConnectorsWeighted;

    /// the position of the distrct (it's mass-middle-point)
    double _x, _y;

    /// Information whether the position is given
    bool _posKnown;
};


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifndef DISABLE_INLINE
//#include "NBDistrict.icc"
//#endif

#endif

// Local Variables:
// mode:C++
// End:

