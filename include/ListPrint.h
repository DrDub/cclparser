#ifndef __LISTPRINT_H__
#define __LISTPRINT_H__

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

#include <string>
#include <sstream>
#include "Reference.h"
#include "RefStream.h"
#include "PrintUtils.h"

// The following object allows a nested list to be printed into an output
// stream at the given indentation.
// Some general formatting parameters are given at construction. Next,
// the object expects to be called repeatedly with a pair of label
// (which is the label of the button in the list) and a printable object
// (CPrintObj) pointer which is to be printed under the button (that is,
// this is the information which is revealed and concealed by the button).
// If this object is NULL, the button is disabled (a disabled button is
// printed only if bNullButton is set to true at initialization).
//
// As an auxiliary function, the objects supplies a memory stream in
// which the label can be constructed (using the standard << operator).
// If no label is given when calling the print function of the object,
// this internal stream is used. This stream is clear every time a
// label is printed, whether it was taken from this buffer or not.

class CListPrint : public CRef
{
private:

    // Formating constants

    // Output stream (currently, must be a file stream)
    CpCRefOStream m_pOut;
    // Number of list elements which should be placed on a single line
    unsigned int m_Fold;
    // Number of spaces between consecutive entries on the same line
    unsigned int m_SepLen;
    // Indentation of the list.
    unsigned int m_Indent;
    // Sub-indentation of the list.
    unsigned int m_SubIndent;
    // Should a button be created for entries where the object to be printed
    // under the button is NULL?
    bool m_bNullButton;
    
    // Print tracking variables
    unsigned int m_Count; // position in the list currently being printed
    bool m_bLastHadRegion; // did the previous entry have a controlled region
    bool m_bClosed;        // was the region already closed?

    // Auxiliary stream
    std::ostringstream m_TempLabel;
public:

    CListPrint(CRefOStream* pOut, unsigned int Indent,
               unsigned int SubIndent, unsigned int Fold = 3,
               unsigned int SepLen = 1, bool bNullButton = true);

    ~CListPrint();

    // Increase the indentation by the given number of 1/3 tabs
    void IncIndent(unsigned int Tab_1_3);
    
    // 'Parameter' is transferred to the printing routine of pPrintObj
    // and is not used by PrintNextEntry in any other way.
    void PrintNextEntry(std::string const& EntryStr, CPrintObj* pPrintObj,
                        int Parameter);
    // Same as above, but using the auxiliary memory stream
    void PrintNextEntry(CPrintObj* pPrintObj, int Parameter);
    // Forces the line to be terminated so that the next entry will be
    // printed on the next line. Multiple consecutive calls to this routine
    // have no effect (no blank lines are left).
    void TerminateLine();
    void CloseList();

    //
    // Operators for auxiliary stream
    //
    
    // template function for the insertion operator into the auxiliary stream
    template <typename T>
    CListPrint& operator<<(T const & Input) {
            m_TempLabel << Input;
        return *this;
    }

    CListPrint& operator<<(CListPrint& (*Func)(CListPrint& ListPrint)){
        return (*Func)(*this);
    }

    CListPrint& Endl();
};

//
// Manipulators
//

// End of line manipulator
extern CListPrint& Endl(CListPrint& ListPrint);

#endif /* __LISTPRINT_H__ */
