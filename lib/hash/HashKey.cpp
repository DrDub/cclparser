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

#include <typeinfo>
#include "HashKey.h"

unsigned int
CStrKey::HashFunc()
{
    unsigned int h;
    const char* str = m_Str.c_str();
    
    for(h =0 ; str && *str ; str++)
        h = ((h >> 3) ^ (h << 5)) + *str;
    
    return h;
}

bool
CStrKey::HashEqual(CKey* pKey)
{
    if(!pKey || (typeid(*pKey) != typeid(*this)))
        return false;
    
    return (bool)!(m_Str.compare(((CStrKey*)pKey)->m_Str.c_str()));
}

bool
CStrKey::operator==(CStrKey& StrKey)
{
    return (bool)!(m_Str.compare(StrKey.m_Str.c_str()));
}
