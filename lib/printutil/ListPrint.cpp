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

#include "PrintUtils.h"
#include "ListPrint.h"
#include "yError.h"

using namespace std;

CListPrint::CListPrint(CRefOStream* pOut, unsigned int Indent,
                       unsigned int SubIndent, unsigned int Fold,
                       unsigned int SepLen, bool bNullButton) :
        m_pOut(pOut), m_Indent(Indent), m_SubIndent(SubIndent), m_Fold(Fold),
        m_SepLen(SepLen), m_bNullButton(bNullButton), m_Count(0),
        m_bLastHadRegion(false), m_bClosed(false), m_TempLabel(ios::out)
{
    static char Rname[] = "CListPrint::CListPrint";
    
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    
    if(!m_Fold) // zero value not allowed
        m_Fold = 1;

    if(!m_pOut) {
        yPError(ERR_MISSING, "output stream missing");
    }
}

CListPrint::~CListPrint()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

// Increase the sub indentation by so many 1/3 tabs
void
CListPrint::IncIndent(unsigned int Tab_1_3) {
    m_SubIndent += Tab_1_3;
    m_Indent += m_SubIndent / 3;
    m_SubIndent = m_SubIndent % 3;
}

// 1. If this is not the last entry in a line, we have to print an eol
//    inside the controlled region (if there is one) but there should be
//    no eol outside the region.
// 2. If this entry is at the end of the line, however, there must be an
//    eol outside the region and therefore no eol inside the region
//    (if there is any).
// 3. If the entry is at the end of the whole list, it does not have
//    an eol either inside the region (as at the end of a line) or
//    outside (because the external eol should be added by the level
//    of printing.
//
// Since we don't know yet whether this is the last entry or not,
// we only indicate whether there is a controlled region to close
// and leave the actual closing of the entry to the printing of the
// next round (or the closing of the whole list).
//
// 'Parameter' is passed as is to the PrintObj routine of pPrintObj
// when it is called.

void
CListPrint::PrintNextEntry(string const& EntryStr, CPrintObj* pPrintObj,
                           int Parameter)
{
    static char Rname[] = "CListPrint::PrintNextEntry";
    
    if(m_bClosed) {
        yPError(ERR_SHOULDNT, "printing entry after close");
    }
    
    // Check first whether (and how) the previous entry needs to be closed
    if(m_Count % m_Fold) {
        // Previous entry was not at the end of the line
        if(m_bLastHadRegion)
            ((ostream&)*m_pOut) << endl
                                << string(m_Indent * PRT_TAB, ' ')
                                << string(m_SubIndent * PRT_1_3_TAB, ' ')
                                << PRT_REGION_END;
    } else if(m_Count) {
        // There was a previous entry and it was last in line
        if(m_bLastHadRegion)
            ((ostream&)*m_pOut) << PRT_REGION_END;
        ((ostream&)*m_pOut) << endl;
    }
        
    // If this is the first entry in a line, print the tabs required by
    // the indentation.

    if(!(m_Count % m_Fold))
        ((ostream&)*m_pOut) << string(m_Indent * PRT_TAB, ' ')
                            << string(m_SubIndent * PRT_1_3_TAB, ' ');

    if(pPrintObj || m_bNullButton)
        ((ostream&)*m_pOut)
            << (pPrintObj ? PRT_BUTTON_CLOSED : PRT_BUTTON_INACTIVE) << " ";
    ((ostream&)*m_pOut) << EntryStr << string(m_SepLen, ' ');

    if(pPrintObj) {
        ((ostream&)*m_pOut) <<  PRT_REGION_START << endl;
        pPrintObj->PrintObj(m_pOut, m_Indent, m_SubIndent, eDefault,
                            Parameter);
    }
    
    m_bLastHadRegion = (bool)pPrintObj;
    m_Count++;

    // clear the auxiliary buffer
    m_TempLabel.str("");
}

void
CListPrint::PrintNextEntry(CPrintObj* pPrintObj, int Parameter)
{
    PrintNextEntry(m_TempLabel.str(), pPrintObj, Parameter);
}

void
CListPrint::TerminateLine()
{
    if(!(m_Count % m_Fold))
        return; // line has been terminated (or no entry was printed)

    m_Count += m_Fold - (m_Count % m_Fold);
}

void
CListPrint::CloseList()
{
    if(m_bClosed)
        return;

    m_bClosed = true;

    if(m_bLastHadRegion)
        *m_pOut << PRT_REGION_END;
}

// print a end of line to the auxiliary stream

CListPrint&
CListPrint::Endl()
{
    m_TempLabel << endl;
    return *this;
}


// End of line manipulator

CListPrint&
Endl(CListPrint& ListPrint)
{
    return ListPrint.Endl();
}
