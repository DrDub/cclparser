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

#include <sstream>
#include "PrintUtils.h"
#include "yError.h"
#include "Globals.h"

using namespace std;

// Initilize printing mode from global variable

// this is reset to 'false' every time the mode should be reinitialized
static bool bInitialized = false;

static struct SPrintingModeNames {
    string m_Name;
    unsigned int m_Bit;
} PrintingModeNames[] = {
    "timing", PMODE_TIMING,
    "obj_count", PMODE_OBJ_COUNT,
    "extra_parse", PMODE_EXTRA_PARSE,
    "more_extra_parse", PMODE_MORE_EXTRA_PARSE,
    "source_text", PMODE_SOURCE_TEXT,
    "extra_eval", PMODE_EXTRA_EVAL,
    "config", PMODE_CONFIG,
    "", 0
};

static void
InitPrintingModes(unsigned int& ActiveModes)
{
    ActiveModes = 0;
    
    // initialize from global variable
    istringstream IStr(g_PrintingMode);
    while(!IStr.fail()) {

        string ModeName;
        IStr >> ModeName;
        if(IStr.fail())
            break;

        bool bFound = false;
        
        // lookup mode in table
        for(struct SPrintingModeNames* Modes = PrintingModeNames ;
            Modes && Modes[0].m_Name.size() ; Modes++) {
            if(Modes->m_Name == ModeName) {
                ActiveModes |= Modes->m_Bit;
                bFound = true;
                break;
            }
        }

        if(!bFound) {
            // warn, but don't abort
            yPWarn(ERR_OUT_OF_RANGE, "unknown printing mode");
        }
    }
}

// Call this to indicate that the printing mode should be reinitialized
// (when the global variables are reset).

void
ResetPrintingMode()
{
    bInitialized = false;
}


// This function takes as input any combination of the above bits.
// It returns true if the printing mode of any of these bits is turned on.

bool
PrintingModeOn(unsigned int PrintingMode)
{
    static unsigned int ActiveModes = 0;
    
    if(!bInitialized) {
        InitPrintingModes(ActiveModes);
        bInitialized = true;
    }

    return (ActiveModes & PrintingMode);
}

eFormat
ParsePrintingFormat()
{
    if(PrintingModeOn(PMODE_MORE_EXTRA_PARSE))
        return eLongDebug;
    if(PrintingModeOn(PMODE_EXTRA_PARSE))
        return eDebug;
    return eDefault;
}
