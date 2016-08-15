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

#include "Unit.h"
#include "yError.h"

using namespace std;

CUnit::CUnit(string const& Name) : m_pName(new CStrKey(Name))
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CUnit::CUnit(CStrKey* pName) : m_pName(pName)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    if(!m_pName) {
        yPError(ERR_MISSING, "Initializing unit with NULL name");
    }
}

CUnit::~CUnit()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CUnit::PrintSymbol(ostream& Out)
{
    if(m_pName)
        Out << (char const*)*m_pName;
    else
        Out << "<no name>";
}

// This is the defualt implementation of unit printing

void
CUnit::PrintObj(CRefOStream* pOut, unsigned int Indent,
                unsigned int SubIndent, eFormat Format,
                int Parameter)
{
    if(!pOut) {
        yPError(ERR_MISSING, "output stream missing");
    }
    
    (*pOut) << string(Indent * PRT_TAB + SubIndent * PRT_1_3_TAB, ' ');
    PrintSymbol(*pOut);
}
