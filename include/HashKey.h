#ifndef __HASHKEY_H__
#define __HASHKEY_H__

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
#include "Reference.h"

//
// Base abstract key class
//

// In addition to a reference pointer, every key must supply a hash function
// for its value an a comparison function (equal/not equal) with other
// keys (of the same type). These functions are only accessible to
// the hash table classes.
// Except for the base (abstract) hash class, hash classes are only
// defined for specific classes of keys, so it is guaranteed that
// the CKey pointer received by HashCmp can safely be cast to the specific
// key type required.

class CKey : public CRef
{
    friend class CHashBase;
private:
    virtual unsigned int HashFunc() = 0; // Hash function
    virtual bool HashEqual(CKey* pKey) = 0; // compares itself with pKey
};

typedef CPtr<CKey> CpCKey;

////////////////////////
// Specific key types //
////////////////////////

//
// String key types
//

// The basic string key type is based on the standard C string class.
// The string value is static - that is, it can only be determined
// at the moment of construction. In this way, the same key may be
// used in different tables without fear of it being changed.

class CStrKey : public CKey
{
private:
    std::string m_Str;
public:
    CStrKey() : m_Str() {}
    CStrKey(std::string const& s) : m_Str(s) {}
    CStrKey(char const* s) : m_Str(s ? s : "") {}
    CStrKey(CStrKey& Key) : m_Str(Key.m_Str.c_str()) {}
    unsigned int HashFunc();
    bool HashEqual(CKey* pKey);
    char const* GetStr() { return m_Str.c_str(); }
    operator std::string const&() { return m_Str; }
    operator char const*() { return m_Str.c_str(); }
    bool operator==(CStrKey& StrKey);
};

typedef CPtr<CStrKey> CpCStrKey;

#endif /* __HASHKEY_H__ */
