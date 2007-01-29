#ifndef MFXImageHelper_h
#define MFXImageHelper_h

/* =========================================================================
 * compiler pragmas
 * ======================================================================= */
#pragma warning(disable: 4786)


#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include <string>
#include <vector>
#include <fx.h>

class MFXImageHelper {
public:
    static FXImage * loadimage(FXApp *a, const std::string& file);
    static FXbool saveimage(FXApp *a, const std::string& file,
        int width, int height, FXColor *data);


};


#endif

