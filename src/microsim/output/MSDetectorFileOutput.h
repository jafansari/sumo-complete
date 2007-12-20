/****************************************************************************/
/// @file    MSDetectorFileOutput.h
/// @author  Christian Roessel
/// @date    2004-11-23
/// @version $Id$
///
// Base of value-generating classes (detectors)
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
#ifndef MSDetectorFileOutput_h
#define MSDetectorFileOutput_h


// ===========================================================================
// included modules
// ===========================================================================
#ifdef _MSC_VER
#include <windows_config.h>
#else
#include <config.h>
#endif

#include <string>

#include <utils/common/SUMOTime.h>
#include <utils/iodevices/OutputDevice.h>


// ===========================================================================
// class definitions
// ===========================================================================
enum DetectorUsage {
    DU_USER_DEFINED,
    DU_SUMO_INTERNAL,
    DU_TL_CONTROL
};

/**
 * @class MSDetectorFileOutput
 * @brief Base of value-generating classes (detectors)
 *
 * Pure virtual base class for classes (e.g. MSInductLoop) that should produce
 *  XML-output via MSDetector2File.
 *
 * @see MSDetector2File
 */
class MSDetectorFileOutput
{
public:
    /// @brief (virtual) destructor
    virtual ~MSDetectorFileOutput() { }

    /// @name Virtual methods to implement by derived classes
    /// @{
    /** @brief Write the generated output to the given device
     * @param[in] dev The output device to write the data into
     * @param[in] startTime First time step the data were gathered
     * @param[in] stopTime Last time step the data were gathered
     */
    virtual void writeXMLOutput(OutputDevice &dev,
                                SUMOTime startTime, SUMOTime stopTime) = 0;

    /** @brief Open the XML-output
     *
     * @param[in] dev The output device to write the root into
     * @see MSDetectorFileOutput::writeXMLDetectorProlog
     */
    virtual void writeXMLDetectorProlog(OutputDevice &dev) const = 0;


};


#endif

/****************************************************************************/

