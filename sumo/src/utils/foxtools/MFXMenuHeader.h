#ifndef MFXMenuHeader_h
#define MFXMenuHeader_h

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

#include <fx.h>

class MFXMenuHeader : public FXMenuCommand {
public:
    MFXMenuHeader(FXComposite* p, FXFont *fnt, const FXString& text,FXIcon* ic=NULL,
        FXObject* tgt=NULL,FXSelector sel=0,FXuint opts=0);
    ~MFXMenuHeader();
};


#endif

