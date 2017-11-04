#ifndef CSTDMF_PCH_HPP
#define CSTDMF_PCH_HPP


#ifdef _WIN32

// use 'override' and 'final' keywords if available
#ifdef _MSC_VER
#define OVERRIDE override
#define FINAL
#else
#define OVERRIDE
#define FINAL
#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <cwctype>
#include <fstream>
#include <functional>
#include <io.h>
#include <iostream>
#include <list>
#include <malloc.h>
#include <map>
#include <process.h>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "cstdmf_windows.hpp"

#endif // _WIN32


#endif // CSTDMF_PCH_HPP
