#ifndef MFXEventQue_h
#define MFXEventQue_h

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
#include <list>

class MFXEventQue {
public:
    MFXEventQue() { }
    ~MFXEventQue() { }

    void *top();
    void pop();
    void add(void *what);
    size_t size();
    bool empty();
private:
    FXMutex myMutex;
    std::list<void*> myEvents;
};

#endif
