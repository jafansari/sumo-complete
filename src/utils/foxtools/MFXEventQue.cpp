#ifdef HAVE_CONFIG_H
#ifdef WIN32
#include <windows_config.h>
#else
#include <config.h>
#endif
#endif // HAVE_CONFIG_H

#include "MFXEventQue.h"
#include <cassert>

#ifdef _DEBUG
#include <utils/dev/debug_new.h>
#endif // _DEBUG

void *
MFXEventQue::top()
{
    assert(size()!=0);
    myMutex.lock();
    void *ret = *(myEvents.begin());
    myMutex.unlock();
    return ret;
}


void
MFXEventQue::pop()
{
    myMutex.lock();
    myEvents.erase(myEvents.begin());
    myMutex.unlock();
}


void
MFXEventQue::add(void *what)
{
    myMutex.lock();
    myEvents.push_back(what);
    myMutex.unlock();
}


size_t
MFXEventQue::size()
{
    myMutex.lock();
    size_t msize = myEvents.size();
    myMutex.unlock();
    return msize;
}


bool
MFXEventQue::empty()
{
    myMutex.lock();
    bool ret = size()==0;
    myMutex.unlock();
    return ret;
}


