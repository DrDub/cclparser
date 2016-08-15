#ifndef __YVECTOR_H__
#define __YVECTOR_H__

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

//
// The classes in this file extend the standard vector template. They are
// all derived from the vector<type> class which they extend and therefore
// can be cast back to that class.
//

// Integer vector that can be initialized from an array of int* 

template<class T>
class CVector : public std::vector<T>
{
public:
    // This constructor initializes the vector based on the list of entries
    // in pInit. The EoL object must be the last object in the initialization
    // array (and is not added to the vector).
    CVector(T* pInit, T EoL) {
        for(T* pVal = pInit ; pVal && !(*pVal == EoL) ; pVal++)
            std::vector<T>::push_back(*pVal);
    }
};

#endif /* __YVECTOR_H__ */
