#ifndef P2CONF_HPP
#define P2CONF_HPP

#include "DataSection.hpp"

#include "DAVAEngine.h"

#define CODE_INLINE

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif

#define PRI64 "lld"
#define PRIu64 "llu"
#define PRIx64 "llx"
#define PRIX64 "llX"
#define PRIzu "lu"
#define PRIzd "ld"

#endif //P2CONF_HPP