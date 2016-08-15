#ifndef __OBJSIO_H__
#define __OBJSIO_H__

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
#include <vector>
#include "Reference.h"
#include "Loop.h"
#include "Objs.h"

//
// The following object is based on the CLoop object and reads objects
// from the given input files.
//

class CObjsIO : public CLoop
{
private:
    // parse object to be used by this IO object
    CpCObjParse m_pParser;
    // a copy of that part of the input stream which belongs to this object
    // (including all line breaks, including the one immediately following
    // the object, if any)
    std::string m_ObjSource; 
    // Number of objects completed in the last line read
    unsigned int m_IncObjNum;
public:
    CObjsIO(CObjParse* pParser,
            std::vector<std::string> const & InFilePatterns,
            CCmdArgOpts* pArgs, CpCMessageLine& MsgLine, COutFile* pOutFile);
    ~CObjsIO() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
private:
    // returns true if the line is a comment line.
    virtual bool IsCommentLine(std::string const& Line) = 0;
public:
    bool NextLine(std::string const& Line);
    bool EndLoop();
    
    unsigned int IncObjNum() { return m_IncObjNum; }
public:
    // function called after each object is read. Returns false if an error
    // occurred.
    virtual bool ProcessObject(CObj* pObj, std::string const& ObjInput) = 0;
};

typedef CPtr<CObjsIO> CpCObjsIO;

//
// different classes for different object formats
//

class CXMLObjsIO : public CObjsIO
{
public:
    CXMLObjsIO(std::vector<std::string> const & InFilePatterns,
               CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
               COutFile* pOutFile);
    ~CXMLObjsIO() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
};

typedef CPtr<CXMLObjsIO> CpCXMLObjsIO;

class CPennTBObjsIO : public CObjsIO
{
public:
    CPennTBObjsIO(std::vector<std::string> const & InFilePatterns,
                  CCmdArgOpts* pArgs, CpCMessageLine& MsgLine,
                  COutFile* pOutFile);
    ~CPennTBObjsIO() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
};

typedef CPtr<CPennTBObjsIO> CpCPennTBObjsIO;

#endif /* __OBJSIO_H__ */
