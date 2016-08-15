#ifndef __PENNSYNSTRUCT_H__
#define __PENNSYNSTRUCT_H__

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
#include "Objs.h"
#include "SynStruct.h"

// The following are syntactic structure objects (CSynStruct) which
// are constructed from objects read from Penn treebank style trees
// (combined format)

class CPennSynStruct : public CSynStruct
{
private:
    bool m_bError;
    std::string m_ErrorMsg;
public:
    // create a syntactic structure based on the Penn Treebank style object
    // pObj. If bReverse is true, the object is read from right to left
    // (this is used in some experiments).
    CPennSynStruct(CObj* pObj, bool bReverse = false);
    virtual ~CPennSynStruct() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif       
    }

protected:

    int AppendSynStruct(CObj* pObj, bool bReverse);

private:
    void SetError(std::string const& Message) {
        m_ErrorMsg = Message;
        m_bError = true;
    }

    // returns true if the given unit is punctuation
    virtual bool IsPunct(std::string const& Unit, std::string const& Tag) = 0;
    // returns true if the given unit is empty
    virtual bool IsEmpty(std::string const& Unit, std::string const& Tag) = 0;
public:
    bool IsError() { return m_bError; }
    std::string const& GetErrorStr() { return m_ErrorMsg; } 
};

typedef CPtr<CPennSynStruct> CpCPennSynStruct;

//
// Derived classes for specific corpora
//

// Wall Street Journal Corpus

class CWSJPennSynStruct : public CPennSynStruct
{
public:
    CWSJPennSynStruct(CObj* pObj, bool bReverse = false);
    ~CWSJPennSynStruct();

private:
    bool IsPunct(std::string const& Unit, std::string const& Tag);
    bool IsEmpty(std::string const& Unit, std::string const& Tag);
};

// Negra Corpus

class CNegraPennSynStruct : public CPennSynStruct
{
public:
    CNegraPennSynStruct(CObj* pObj, bool bReverse = false);
    ~CNegraPennSynStruct();

private:
    bool IsPunct(std::string const& Unit, std::string const& Tag);
    bool IsEmpty(std::string const& Unit, std::string const& Tag);
};

// Chinese Treebank

class CCTBPennSynStruct : public CPennSynStruct
{
public:
    CCTBPennSynStruct(CObj* pObj, bool bReverse = false);
    ~CCTBPennSynStruct();

private:
    bool IsPunct(std::string const& Unit, std::string const& Tag);
    bool IsEmpty(std::string const& Unit, std::string const& Tag);
};

#endif /* __PENNSYNSTRUCT_H__ */
