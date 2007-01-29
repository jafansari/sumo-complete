#ifndef Command_H
#define Command_H
/***************************************************************************
                          Command.h  -  Command-pattern-class.
                             -------------------
    begin                : Thu, 20 Dec 2001
    copyright            : (C) 2001 by Christian Roessel
    email                : roessel@zpr.uni-koeln.de
 ***************************************************************************/

/**
 * @file   Command.h
 * @author Christian Roessel
 * @date   Started Thu, 20 Dec 2001
 * $Revision$ from $Date$ by $Author$
 *
 * @brief Contains the declaration/implementation of the Command base class.
 */


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// $Log$
// Revision 1.3  2006/03/17 09:15:11  dkrajzew
// changed the Event-interface (execute now gets the current simulation time, event handlers are non-static)
//
// Revision 1.2  2005/10/06 13:39:50  dksumo
// using of a configuration file rechecked
//
// Revision 1.1  2005/09/09 12:56:07  dksumo
// helpers added
//
// Revision 1.2  2005/02/01 09:49:24  dksumo
// got rid of MSNet::Time
//
// Revision 1.1  2004/10/22 12:49:22  dksumo
// initial checkin into an internal, standalone SUMO CVS
//
// Revision 1.6  2003/08/06 16:51:02  roessel
// Better distinction between steps and seconds added.
//
// Revision 1.5  2003/07/18 12:35:04  dkrajzew
// removed some warnings
//
// Revision 1.4  2003/06/06 14:18:51  roessel
// Documentation added.
//
// Revision 1.3  2003/02/07 10:40:13  dkrajzew
// updated
//
// Revision 1.2  2002/10/16 16:53:12  dkrajzew
// global inclusion
//
// Revision 1.1  2002/10/16 14:44:45  dkrajzew
// ROOT/sumo moved to ROOT/src
//
// Revision 1.2  2002/07/31 17:35:55  roessel
// Changes since sourceforge cvs request.
//
// Revision 1.4  2002/07/30 15:11:55  croessel
// Undid previous commit
//
// Revision 1.2  2002/07/23 13:54:00  croessel
// Removed constructor since Command is pure virtual.
//
// Revision 1.1.1.1  2002/04/08 07:21:22  traffic
// new project name
//
// Revision 2.0  2002/02/14 14:43:13  croessel
// Bringing all files to revision 2.0. This is just cosmetics.
//
// Revision 1.3  2002/01/30 11:15:38  croessel
// Documentation added.
//
// Revision 1.2  2002/01/17 15:19:32  croessel
// Changed the return-type of the possible operations from void to
// MSNet::Time. Valuable for periodic commands, i.e. traffic-light
// switches.
//
// Revision 1.1  2002/01/09 15:05:30  croessel
// Initial commit.
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

#include <utils/common/SUMOTime.h>


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * @brief Base class for all Command classes.
 *
 * When executed, the current time step will be given to the command.
 * The command must then return the time to the next call in simulation
 *  seconds. If zero is returned, this command will be descheduled and
 *  deleted by the event handler.
 *
 * @see Design Patterns, Gamma et al.
 * @see WrappingCommand
 * @see OneArgumentCommand
 * @see MSEventControl
 */
class Command
{
public:
    Command() { }

    /// Destructor.
    virtual ~Command() { }

    /**
     * @brief Executes the command.
     *
     * @param currentTime The current simulation time
     *
     * @return The receivers operation should return the next interval
     * in steps for recurring commands and 0 for single-execution
     * commands.
     */
    virtual SUMOTime execute(SUMOTime currentTime) = 0;

};

#endif

// Local Variables:
// mode:C++
// End:

