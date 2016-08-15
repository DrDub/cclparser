#ifndef __YERROR_H__
#define __YERROR_H__

// Copyright 2007 Yoav Seginer

// This file is part of CCL-Parser.
// CCL-Parser is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// CCL-Parser is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with CCL-Parser.  If not, see <http://www.gnu.org/licenses/>.

// Erro code (for throwing)

#define ERR_MEM 0     // memory allocation/de-allocation error
#define ERR_DEBUG 1   // debug printing
#define ERR_OUT_OF_RANGE 2  // out of range argument/parameter
#define ERR_SYS 3     // error as a result of a system call (may have no errno)
#define ERR_FILE 4    // error in file operations
#define ERR_INIT 5    // improperly initialized object (NULL pointers, etc.)
#define ERR_MISSING 6 // missing value (e.g. NULL pointer)
#define ERR_SHOULDNT 7 // things that should not happen
#define ERR_NOT_YET  8 // The requested functionality is not yet supported

// Basic error handling routines

#define yError(__Code, __ReportErrno, __Str) { \
        Err::Write(__ReportErrno, "%s:%d: %s", __FILE__, __LINE__, __Str); \
        throw(__Code);                                                     \
}

#define yWarn(__Code, __ReportErrno,  __Str) { \
        Err::Write(__ReportErrno, "%s:%d: %s", __FILE__, __LINE__, __Str); \
}

#define yMemError(__Str) yError(ERR_MEM, true, __Str)
#define yMemWarn(__Str) yWarn(ERR_MEM, true, __Str)

// program error (no errno code)
#define yPError(__Code, __Str) yError(__Code, false, __Str)
// system error (with errno code)
#define ySError(__Code, __Str) yError(__Code, true, __Str)

// program warning (no errno code)
#define yPWarn(__Code, __Str) yWarn(__Code, false, __Str)
// system warning (with errno code)
#define ySWarn(__Code, __Str) yWarn(__Code, true, __Str)


// Debug error routined

#ifdef DEBUG
#define derror(__Str) yWarn(DEBUG_MSG , false, __Str)
#else
#define derror(__Str)
#endif

class Err
{
public:
    static void Write(bool ReportErrorCode, char const* pFmt, ...);
};

#endif /* __YERROR_H__ */
