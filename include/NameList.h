#ifndef __NAMELIST_H__
#define __NAMELIST_H__

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
#include <string>

// The following structure is used to hold a pair of code + string. An array
// of such pairs (in whatever order) is used to initialize the name list
// class below.
// It is assumed that the strings stores in this table are constant strings
// and therefore no automatic pointers is being used for them.

struct SNameList {
    unsigned int m_Code;
    char const* m_Name;
};

// The following class is initialized by an array of SNameList pairs
// terminated by a pair with a NULL string. These are put into the
// vector in the position equal to their code, enabling quick lookup.
// Lookup is also safe - if the code is not in the table, the default
// string (given at initialization) is returned.
//
// This is only a default implementation. Classes can be derived from this
// class which return the name in some other way, which is why the lookup
// operator [] is virtual.
//
// This class does not support automatic pointers since it is assumed that
// it is only used as a static constant. 

class CNameList {
private:
    std::string m_Default;
    std::vector<std::string> m_List;
    // An array indicating for each entry whether it is defined or not.
    std::vector<bool> m_Defined;
public:
    // Initialized by an array of SNameList pairs (terminated by a pair
    // with a NULL string) and a default string.
    CNameList(SNameList const* pPairs, char const* pDefault);
protected:
    // This function allows a derived class to add additional names after
    // initialization.
    void AddName(unsigned int Code, std::string const& Name);
    // Return the name stored for the given code.
    std::string const& GetName(unsigned int Code) const;
public:
    // Operator for retrieving the string for a given code.
    // This is only a default implementation. Derived classes can override it.
    virtual std::string const& operator[](unsigned int Code) {
        return GetName(Code);
    }
    // Returns true if a name is defined for this entry
    bool IsDefined(unsigned int const Code) const;
};
#endif /* __NAMELIST_H__ */
