#ifndef __REFSTREAM_H__
#define __REFSTREAM_H__

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

#include "Reference.h"
#include <ostream>
#include <fstream>

// base class for all ostream objects with a reference count.

class CRefOStream : public CRef
{
public:
    CRefOStream();
    ~CRefOStream();
    
    template <typename T>
    CRefOStream& operator<<(T const & Input) {
        GetStream() << Input;
        return *this;
    }

    operator std::ostream&() { return GetStream(); }
private:
    virtual std::ostream& GetStream() = 0;
public:
    // Derived streams which have an open/closed state should redefine this
    // function.
    virtual bool IsOpen() { return true; }
};

typedef CPtr<CRefOStream> CpCRefOStream;

class CRefOFStream : public CRefOStream
{
private:
    std::ofstream m_Stream;
public:
    CRefOFStream();
    CRefOFStream(char const* FileName);
    ~CRefOFStream();
private:
    std::ostream& GetStream() { return (std::ostream&)m_Stream; }
public:
    bool IsOpen() { return m_Stream.is_open(); }
};

typedef CPtr<CRefOFStream> CpCRefOFStream;
    
#endif /* __REFSTREAM_H__ */
