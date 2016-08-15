#ifndef __STATTABLE_H__
#define __STATTABLE_H__

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

#include "Strength.h"
#include "PropConv.h"
#include "yError.h"

///////////////////////
// Statistics Tables //
///////////////////////

// This is a template for constructing statistics tables for different
// objects. It is based on the CStrengths template and provides similar
// functionality. The difference between between the statistics table
// and the strength table is that the statistics table can store the
// strengths in a more memory efficient manner when the codes of the
// properties are not consecutive numbers beginning at 0. This is achieved
// by using a conversion vector (of type CPropConv) which converts an
// arbitrary list of property codes to a list of consecutive codes beginning
// at zero. Moreover, properties which have a top list get a smaller code,
// so lists only need to be constructed for those properties which need
// them.
//
// In addition, the statistics table offers a cache which allows fast access
// to multiple properties of the same key.

template <class K, class V>
class CStatTable : public CStrengths<K, V>
{
private:
    //
    // Cache
    //
    // Cache of the last accessed key and value - this allows quick access
    // in succession to different properties of the same key.
    // The cached key and value are updated every time an entry is looked-up
    // either for retrieval or for update.
    CPtr<K> m_pLastKey;
    CPtr<V> m_pLastVal;
public:
    CStatTable(unsigned int TopNum, unsigned int TopLength,
               unsigned int HashSize, bool bReserve = false) :
            CStrengths<K, V>(TopNum, TopLength, HashSize, bReserve) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    ~CStatTable() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

private:

    // The following routine returns a reference to the property
    // convertor (which must be defined in the derived class)
    virtual CPropConv& GetTablePropConv() = 0;
    
    // The following function converts the absolute property code
    // to the position of the property in the current table. Returns -1
    // if the requested code is not supported by the table.
    int GetLocalCode(unsigned int AbsCode) {
        return GetTablePropConv().GetPropCode(AbsCode);
    }

public:
    
    // Returns the property to which the given code was assigned (locally).
    // If no property was assigned the given code, -1 is returned.
    int GetPropByLocalCode(unsigned int Code) {
        return GetTablePropConv().GetPropByLocalCode(Code);
    }
    
private:
    
    // Clear the cache
    void ClearCache() {
        m_pLastKey = NULL;
        m_pLastVal = NULL;
    }

public:
    
    //
    // Reading and updating element properties
    //
    
    // Returns the strength of the given (absolute) property for the
    // entry with the given value.
    float GetStrg(V* pVal, unsigned int Prop) {
        if(!pVal)
            return 0;

        int LocalCode = GetLocalCode(Prop);

        if(LocalCode < 0) {
            yPError(ERR_OUT_OF_RANGE, "property not supported by table");
            return 0; // the property is not supported
        }

        return CStrengths<K, V>::GetStrengthFromVec((CRvector<float>*)pVal,
                                                    LocalCode);
    }
    // Same as above, only for the entry currently in the cache.
    float GetStrg(unsigned int Prop) {
        return GetStrg(m_pLastVal, Prop);
    }
    // Returns the strength of the given (absolute) property for the given
    // key.
    float GetStrg(K* pKey, unsigned int Prop) {
        if(!pKey) {
            ClearCache();
            return 0;
        }
        // Find the key in the table
        m_pLastVal = GetVal(*pKey);
        // The key stored in the table should be identical to pKey, but may
        // be a different object. Here we read the object actually stored in
        // the table.
        m_pLastKey = CStrengths<K, V>::GetKey();
        return GetStrg(Prop);
    }

    // Get the strength of the strongest entry for this property. Returns
    // 0 if no such entry exists - also if there is no top list for the
    // given property.
    float GetTopStrg(unsigned int Prop) {
        int LocalCode = GetLocalCode(Prop);

        if(LocalCode < 0) {
            yPError(ERR_OUT_OF_RANGE, "property not supported by table");
            return 0; // the property is not supported
        }

        return CStrengths<K, V>::GetTopStrg(LocalCode);
    }

    // Get the strength of the weakest entry in the list of strongest
    // entries for this property. Returns 0 if no such entry exists - also
    // if there is no top list for the given property.
    float GetLastTopStrg(unsigned int Prop) {
        int LocalCode = GetLocalCode(Prop);

        if(LocalCode < 0) {
            yPError(ERR_OUT_OF_RANGE, "property not supported by table");
            return 0; // the property is not supported
        }

        return CStrengths<K, V>::GetLastTopStrg(LocalCode);
    }
    
    // Increment the strength of the given property for the given key
    // by the given amount.
    void IncStrg(K* pKey, unsigned int Prop, float Strg) {
        if(!pKey)
            return;
        int Code = GetLocalCode(Prop);
    
        if(Code < 0) {
            yPError(ERR_OUT_OF_RANGE, "property not supported by table");
        }

        m_pLastVal = IncStrength(*pKey, Code, Strg);
        m_pLastKey = CStrengths<K, V>::GetKey();
    }
    // Same as above, but for the entry currently cached.
    void IncStrg(unsigned int Prop, float Strg) {
        if(!m_pLastKey || !m_pLastVal)
            return;

        int Code = GetLocalCode(Prop);
    
        if(Code < 0) {
            yPError(ERR_OUT_OF_RANGE, "property not supported by table");
        }

        IncStrength(m_pLastVal, Code, Strg);
    }
    
    // Get the number of entries in the top list
    unsigned int GetTopLength(unsigned int Prop) {
        int Code = GetLocalCode(Prop);
        if(Code < 0)
            return false;

        return CStrengths<K, V>::GetTopLength(Code);
    }
    
    // Returns true if the entry with the given value object is in the
    // top list for the given property.
    bool IsInTopList(V* pVal, unsigned int Prop) {
        int Code = GetLocalCode(Prop);
    
        if(Code < 0)
            return false;

        return IsInTheTopList((CRvector<float>*)pVal, Code);
    }

    // Same as above, only for the cached (last read) entry
    bool IsInTopList(unsigned int Prop) {
        return IsInTopList(m_pLastVal, Prop);
    }
    
    //
    // Top List Iterator 
    //

    // Return the top list iterator for the given property. If there is no
    // top list for the given property, a NULL pointer is returned.
    CTopIter<K, V>* GetIter(unsigned int Prop) {
        int Code = GetLocalCode(Prop);
        if(Code < 0)
            return NULL;
        return CStrengths<K,V>::GetIter(Code);
    }
};

#endif /* __STATTABLE_H__ */
