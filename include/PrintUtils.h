#ifndef __PRINTUTILS_H__
#define __PRINTUTILS_H__

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

#include "RefStream.h"

#define PRT_BUTTON_CLOSED      ""
#define PRT_HALF_BUTTON_CLOSED ""   // functionally the same as a full button
#define PRT_BUTTON_INACTIVE    ""
#define PRT_REGION_START       ""
#define PRT_REGION_END         ""

// tab sizes

#define PRT_1_3_TAB  2
#define PRT_TAB      6

// Different printing modes are available. The following codes define
// the different printing modes.

// Basic printing: parses in parse cycle and final evaluation results.
// This printing is always on.
#define PMODE_BASIC            0x00
// Timing information
#define PMODE_TIMING           0x01
// Object count information
#define PMODE_OBJ_COUNT        0x02
// Output extra parse information
#define PMODE_EXTRA_PARSE      0x04
// Output even more parse information (always contains PMODE_EXTRA_PARSE)
#define PMODE_MORE_EXTRA_PARSE 0x08
// Output the source text of the utterance
#define PMODE_SOURCE_TEXT      0x10
// Extra evaluation information
#define PMODE_EXTRA_EVAL       0x20
// Configuration information
#define PMODE_CONFIG           0x40

// Call this to indicate that the printing mode should be reinitialized
// (when the global variables are reset).
extern void ResetPrintingMode();
// This function takes as input any combination of the above bits.
// It returns true if the printing mode of any of these bits is turned on.
extern bool PrintingModeOn(unsigned int PrintingMode);

// For some objects there may be different formats which the objects can
// be printed in.

enum eFormat {
    eNoPrint,   // print nothing
    eDefault,   // default for this object
    eShort,
    eLong,
    eDebug,     // additional debug information
    eLongDebug, // a lot of debug information
};

// returns true if the format is a debug format
#define IS_DEBUG_FORMAT(Format) ((Format) == eDebug || (Format) == eLongDebug)
// the following function returns a format code for parse printing as
// determined by the printing mode
extern eFormat ParsePrintingFormat();

//
// The following object is the abstract interface which allows an object
// to be printed at the given indentation (measured in the tab units defined
// above).
// The object should always be printed without an eol at the end. This is
// in order to allow the object to be placed inside a region controlled
// by a button. In this case only the function which generated the button
// can determine whether the eol should be inserted before the end of the
// region (if there is no eol when the region is closed) or after it
// (when the region is at the end of the line even when it is closed).
//

class CPrintObj
{
public:
    // Print the object into the given output stream at the given indentation.
    // 'Parameter' is a single argument which can be used freely by each
    // object in its implementation of the PrintObj routine. All
    // implementations should reserve 0 as the default value for this
    // parameter.
    virtual void PrintObj(CRefOStream* pOut, unsigned int Indent,
                          unsigned int SubIndent, eFormat Format,
                          int Parameter) = 0;
};

#endif /* __PRINTUTILS_H__ */
