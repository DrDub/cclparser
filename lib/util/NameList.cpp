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

#include "NameList.h"

using namespace std;

CNameList::CNameList(SNameList const* pPairs, char const* pDefault) :
        m_Default(pDefault)
{
    for( ; pPairs && pPairs->m_Name ; pPairs++)
        AddName(pPairs->m_Code, pPairs->m_Name);
}

void
CNameList::AddName(unsigned int Code, string const& Name)
{
    if(m_List.size() <= Code)
        m_List.resize(Code+1, m_Default);
    if(m_Defined.size() <= Code)
        m_Defined.resize(Code+1, false);
    
    m_List[Code] = Name;
    m_Defined[Code] = true;
}

string const&
CNameList::GetName(unsigned int Code) const
{
    if(Code >= m_List.size())
        return m_Default;

    return m_List[Code];
}

bool
CNameList::IsDefined(unsigned int const Code) const
{
    if(Code >= m_Defined.size())
        return false;

    return m_Defined[Code];
}
