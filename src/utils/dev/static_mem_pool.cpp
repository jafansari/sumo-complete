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


#ifdef _DEBUG
#ifdef CHECK_MEMORY_LEAKS

#include <foreign/nvwa/static_mem_pool.cpp>

#endif // CHECK_MEMORY_LEAKS
#endif // _DEBUG
