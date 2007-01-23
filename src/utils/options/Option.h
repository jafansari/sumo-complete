#ifndef Option_h
#define Option_h
/***************************************************************************
                          Option.h
              A class representing a single program option
              together with her derivates to represent different
              value types
                             -------------------
    project              : SUMO
    begin                : Mon, 17 Dec 2001
    copyright            : (C) 2001 by DLR/IVF http://ivf.dlr.de/
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
// $Log: Option.h,v $
// Revision 1.9  2005/10/07 11:46:56  dkrajzew
// THIRD LARGE CODE RECHECK: patched problems on Linux/Windows configs
//
// Revision 1.8  2005/09/23 06:11:58  dkrajzew
// SECOND LARGE CODE RECHECK: converted doubles and floats to SUMOReal
//
// Revision 1.7  2005/09/15 12:21:19  dkrajzew
// LARGE CODE RECHECK
//
// Revision 1.6  2005/05/04 09:28:01  dkrajzew
// level 3 warnings removed; a certain SUMOTime time description added
//
// Revision 1.5  2004/11/23 10:36:02  dkrajzew
// debugging
//
// Revision 1.4  2004/07/02 09:41:39  dkrajzew
// debugging the repeated setting of a value
//
// Revision 1.3  2003/08/20 11:49:55  dkrajzew
// allowed the retrival of an uint-vector encoded as string; not the best, but the fastest solution
//
// Revision 1.2  2003/02/07 10:51:59  dkrajzew
// updated
//
// Revision 1.1  2002/10/16 14:58:18  dkrajzew
// initial release for utilities that handle program options
//
// Revision 1.4  2002/07/31 17:30:06  roessel
// Changes since sourceforge cvs request.
//
// Revision 1.4  2002/07/11 07:42:59  dkrajzew
// Usage of relative pathnames within configuration files implemented
//
// Revision 1.4  2002/06/11 15:58:24  dkrajzew
// windows eol removed
//
// Revision 1.3  2002/05/14 04:45:49  dkrajzew
// Bresenham added; some minor changes; windows eol removed
//
// Revision 1.2  2002/04/26 10:08:38  dkrajzew
// Windows eol removed
//
// Revision 1.1.1.1  2002/04/09 14:18:27  dkrajzew
// new version-free project name (try2)
//
// Revision 1.1.1.1  2002/04/09 13:22:01  dkrajzew
// new version-free project name
//
// Revision 1.2  2002/03/20 08:50:37  dkrajzew
// Revisions patched
//
// Revision 1.1  2002/02/13 15:48:18  croessel
// Merge between SourgeForgeRelease and tesseraCVS.
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
#include <vector>
#include <exception>


/* =========================================================================
 * class definitions
 * ======================================================================= */
/**
 * Definition of a vector of unsigned ints for later parsing of values
 * such as frequency lists
 */
typedef std::vector<int> IntVector;


/**
 * @class Option
 * The base class for a single program option. All options which hold values
 * are derived from this class as the type may differ.
 *
 * When set, the value is valid. May be set with an initial, default value.
 *
 * Usage:
 * As the options should only be used during the program initialisation, some
 * methods do output error messages to cout. Error messages may be printed,
 * when a default value is overwritten.
 *
 * Most of the getter- and setter-methods throw exceptions as this base class
 * is not meant to hold any values by itself. Theses are stored in the derived
 * classes. This class does only supply common access for a common base class
 * while the real access methods are implemented in the derived classes.
 * The access of the proper valuetype for each of the derived class must be
 * validated by the programmer.
 *
 * Exceptions:
 * Only the exception "InvalidArgument" from "UtilExceptions" is thrown
 */
class Option {
public:
    /** destructor */
    virtual ~Option();

    /** returns the information whether the value is valid */
    bool isSet() const;

    /** returns the SUMOReal value */
    virtual SUMOReal getFloat() const;

    /** returns the long value */
    virtual long getLong() const;

    /** returns the int value */
    virtual int getInt() const;

    /** returns the string value */
    virtual std::string getString() const;

    /** returns the bool value */
    virtual bool getBool() const;

    /** returns the bool value */
    virtual const IntVector &getIntVector() const;

    /** sets the value (used for non-bool options) */
    virtual bool set(std::string v, bool isDefault=false);

    /** sets the value (used for bool options) */
    virtual bool set(bool v, bool isDefault=false);

    /** returns the string-representation of the value */
    virtual std::string getValue() const;

    /** returns the information whether the option is a bool option */
    virtual bool isBool() const;

    /** returns the information whether the option holds the default value */
    virtual bool isDefault() const;

    /// returns the information whether this option is a file name
    virtual bool isFileName() const;

    /** Returns the information whether the option may be set a further time */
    bool isWriteable() const;

    /** OptionsCont is a friend class as it may reinitialise the option
        to be a default value */
    friend class OptionsCont;

    const std::string &getDescription() const;
    virtual std::string getTypeName() const;


protected:
    /** sets the "myAmSet" - information. returns whether the option was a
        default option */
    bool markSet(bool isDefault);

protected:
    /** constructor (paramter: value is valid) */
    Option(bool set=false);

    /** copy constructor */
    Option(const Option &s);

    /** assignment operator */
    virtual Option &operator=(const Option &s);

private:
    /** information whether the value is set */
    bool myAmSet;

    /** information whether the value is the default value (is then set) */
    bool myHaveTheDefaultValue;

    /** information whether the value may be changed */
    bool myAmWritable;

    std::string myDescription;

};

/**
 * @class Option_Integer
 * An integer-option
 */
class Option_Integer : public Option {
 private:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    int      _value;
 public:
    /** constructor; the value will be invalid (unset) */
    Option_Integer();
    /** constructor; the default value is given */
    Option_Integer(int value);
    /** copy constructor */
    Option_Integer(const Option_Integer &s);
    /** destructor */
    ~Option_Integer();
    /** assignment operator */
    Option_Integer &operator=(const Option_Integer &s);
    /** returns the integer value */
    int getInt() const;
    /** sets the given value (converts it to int) */
    bool set(std::string v, bool isDefault=false);
    /** returns the values string-representation */
    std::string getValue() const;
    virtual std::string getTypeName() const;
};

class Option_Long : public Option {
 private:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    long      _value;
 public:
    /** constructor; the value will be invalid (unset) */
    Option_Long();
    /** constructor; the default value is given */
    Option_Long(long value);
    /** copy constructor */
    Option_Long(const Option_Long &s);
    /** destructor */
    ~Option_Long();
    /** assignment operator */
    Option_Long &operator=(const Option_Long &s);
    /** returns the long value */
    long getLong() const;
    /** sets the given value (converts it to long) */
    bool set(std::string v, bool isDefault=false);
    /** returns the values string-representation */
    std::string getValue() const;
    virtual std::string getTypeName() const;
};


class Option_String : public Option {
 protected:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    std::string      _value;
 public:
    /** constructor; the value will be invalid (unset) */
    Option_String();
    /** constructor; the default value is given */
    Option_String(std::string value);
    /** copy constructor */
    Option_String(const Option_String &s);
    /** destructor */
    virtual ~Option_String();
    /** assignment operator */
    Option_String &operator=(const Option_String &s);
    /** returns the string value */
    std::string getString() const;
    /** sets the given value */
    bool set(std::string v, bool isDefault=false);
    /** returns the values string-representation */
    std::string getValue() const;
    virtual std::string getTypeName() const;
};


class Option_Float : public Option {
 private:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    SUMOReal       _value;
 public:
    /** constructor; the value will be invalid (unset) */
    Option_Float();
    /** constructor; the default value is given */
    Option_Float(SUMOReal value);
    /** copy constructor */
    Option_Float(const Option_Float &s);
    /** destructor */
    ~Option_Float();
    /** assignment operator */
    Option_Float &operator=(const Option_Float &s);
    /** returns the SUMOReal value */
    SUMOReal getFloat() const;
    /** sets the given value (converts it to SUMOReal) */
    bool set(std::string v, bool isDefault=false);
    /** returns the values string-representation */
    std::string getValue() const;
    virtual std::string getTypeName() const;
};


class Option_Bool : public Option {
 private:
    /** the value, valid only when the base-classes "myAmSet"-member is true */
    bool        _value;
 public:
    /** constructor; the value will be invalid (unset) */
    Option_Bool();
    /** constructor; the default value is given */
    Option_Bool(bool value);
    /** copy constructor */
    Option_Bool(const Option_Bool &s);
    /** destructor */
    ~Option_Bool();
    /** assignment operator */
    Option_Bool &operator=(const Option_Bool &s);
    /** returns the boolean value */
    bool getBool() const;
    /** sets the given value (converts it to bool) */
    bool set(bool v, bool isDefault=false);
    /** returns the values string-representation */
    std::string getValue() const;
    /** returns always true */
    bool isBool() const;
    virtual std::string getTypeName() const;
};

class Option_FileName : public Option_String {
public:
    /** constructor; the value will be invalid (unset) */
    Option_FileName();
    /** constructor; the default value is given */
    Option_FileName(std::string value);
    /** copy constructor */
    Option_FileName(const Option_String &s);
    /** destructor */
    virtual ~Option_FileName();
    /** assignment operator */
    Option_FileName &operator=(const Option_FileName &s);
    /// returns the information whether this option is a file name
    bool isFileName() const;
    virtual std::string getTypeName() const;
};


class Option_IntVector : public Option {
protected:
    IntVector _value;
public:
    /** constructor; the value will be invalid (unset) */
    Option_IntVector();
    /** constructor; the default value is given */
    Option_IntVector(const IntVector &value);
    /** constructor;
        the default value will be parsed from the string
        use ';' as delimiters */
    Option_IntVector(const std::string &value);
    /** copy constructor */
    Option_IntVector(const Option_IntVector &s);
    /** destructor */
    virtual ~Option_IntVector();
    /** assignment operator */
    Option_IntVector &operator=(const Option_IntVector &s);

    const IntVector &getIntVector() const;
    /** sets the given value (converts it to int) */
    bool set(std::string v, bool isDefault=false);
    /** returns the values string-representation */
    std::string getValue() const;
    std::string getString() const;
    virtual std::string getTypeName() const;
};


/**************** DO NOT DECLARE ANYTHING AFTER THE INCLUDE ****************/

#endif

// Local Variables:
// mode:C++
// End:
