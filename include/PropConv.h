#ifndef __PROPCONV_H__
#define __PROPCONV_H__

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

#include <vector>

/////////////////////////////////////
// Property Code Conversion Vector //
/////////////////////////////////////

// The property code conversion vector has one position for every absolute
// property. It is initialized by two arrays of property codes (integers)
// which should be stored in the conversion array. The first list consists
// of properties which should have a top list and the second list consists
// of properties which should not have a top list (the first occurrence
// of each property is used). The arrays are terminated by -1.
// Local codes are assigned to the properties in the order in which they
// appear in the list.

class CPropConv
{
private:
    // Conversion vector
    std::vector<int> m_Conv;
    // Opposite conversion (from local code to property)
    std::vector<unsigned int> m_OpConv;
    // Total number of properties
    unsigned int m_PropNum;
    // The number of properties which have a top list
    unsigned int m_TopListNum;
    // Can the list of properties be extended when an unknown property
    // is looked up?
    bool m_bExtendable;
public:
    // The third argument to this function (bExtendable) indicates whether
    // the list of properties may be extended automatically when an
    // unknown property is looked up. This unknown property will always
    // be added as a no-top property.
    CPropConv(int* pTop, int* pNoTop, bool bExtendable = false);

    // Add a property to the list (after initialization). For
    // implementational reasons, Only properties without a top list may be
    // added. Returns false if the property already exists.
    bool AddNonTopProp(int Prop);
    
    // Returns the number of properties which have a top list
    unsigned int TopListNum() const { return m_TopListNum; }
    // Returns the code assigned to the given property. If the object was
    // created as non-extendable, returns -1 if no code was assigned.
    // Otherwise, it adds the property as a new no-top property.
    int GetPropCode(unsigned int Prop);
    int GetPropByLocalCode(unsigned int Code) const {
        return (m_PropNum <= Code) ? -1 : (int)m_OpConv[Code];
    }
    unsigned int GetPropNum() const { return m_PropNum; }
};

#endif /* __PROPCONV_H__ */
