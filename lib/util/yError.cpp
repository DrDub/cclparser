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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include "StringUtil.h"
#include "yError.h"

void
Err::Write(bool ReportErrorCode, char const* pFmt, ...)
{
        va_list Args;
 
        va_start(Args, pFmt);
        (void) vfprintf(stderr, pFmt, Args);
        va_end(Args);
 
        if (ReportErrorCode)
            fprintf(stderr, " : %s", strerror(errno));

        fprintf(stderr, "%s", EoLDelimiter().c_str()); 
}
