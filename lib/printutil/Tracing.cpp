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

#include "Tracing.h"
#include "yError.h"
#include "PrintUtils.h"

using namespace std;

//////////////////
// Trace Stream //
//////////////////

CTraceStream::CTraceStream(COutFile* pOutputFile) :
        m_pOutputFile(pOutputFile)
{
}

CTraceStream::CTraceStream(CTraceStream const & TraceStream) :
        m_pOutputFile(TraceStream.m_pOutputFile)
{
}

void
CTraceStream::SetOutputFile(COutFile* pOutputFile)
{
    m_pOutputFile = pOutputFile;
}

CTraceStream&
CTraceStream::operator<<(CPrintObj* pPrintObj)
{
    // need to get the format and indentation even if eventually nothing
    // will be printed (because this resets the format for the next object)
    eFormat Format = GetNextFormat();
    unsigned int Indent = GetIndent();
    
    if(m_pOutputFile && TracingOn()) {
        
        if(!pPrintObj)
            ((ostream&)*m_pOutputFile) << "<NULL>";
        else
            pPrintObj->
                PrintObj((CRefOStream*)*m_pOutputFile, Indent, 0, Format, 0);
    }
    return *this;
}

CTraceStream&
CTraceStream::operator<<(CPrintObj& PrintObj)
{
    return *this << &PrintObj;
}

// print a tab to the stream
CTraceStream&
CTraceStream::Tab()
{
    if(m_pOutputFile && TracingOn()) {
        string Tab(PRT_TAB, ' ');
        ((ostream&)*m_pOutputFile) << Tab;
    }
    return *this;
}

// print a third tab to the stream
CTraceStream&
CTraceStream::Tab_1_3()
{
    if(m_pOutputFile && TracingOn()) {
        string Tab(PRT_1_3_TAB, ' ');
        ((ostream&)*m_pOutputFile) << Tab;
    }
    return *this;
}

// print a end of line to the stream
CTraceStream&
CTraceStream::Endl()
{
    if(m_pOutputFile && TracingOn())
        ((ostream&)*m_pOutputFile) << std::endl;
    return *this;
}

//////////////////
// Manipulators //
//////////////////

//
// Manipulators for setting the next object format
//

CTraceStream&
Long(CTraceStream& TraceStream)
{
    TraceStream.SetNextFormat(eLong);
    return TraceStream;
}

CTraceStream&
Short(CTraceStream& TraceStream)
{
    TraceStream.SetNextFormat(eShort);
    return TraceStream;
}

//
// Indentation manipulators
//

CTraceStream&
Indent(CTraceStream& TraceStream)
{
    TraceStream.IncIndent();
    return TraceStream;
}

CTraceStream&
UnIndent(CTraceStream& TraceStream)
{
    TraceStream.DecIndent();
    return TraceStream;
}

CTraceStream&
NoIndent(CTraceStream& TraceStream)
{
    TraceStream.SetIndent(0);
    return TraceStream;
}

// Tab manipulator

CTraceStream&
Tab(CTraceStream& TraceStream)
{
    return TraceStream.Tab();
}

// Third tab manipulator

CTraceStream&
Tab_1_3(CTraceStream& TraceStream)
{
    return TraceStream.Tab_1_3();
}

// End of line manipulator

CTraceStream&
Endl(CTraceStream& TraceStream)
{
    return TraceStream.Endl();
}

////////////////////
// Tracing Object //
////////////////////

CTracing::CTracing() :
        CTraceStream(NULL), m_NextObjFormat(eNoPrint), m_Indent(0),
        m_TraceTypes(0), m_bDisabled(true)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CTracing::CTracing(COutFile* pOutputFile, unsigned int TraceTypes) :
        CTraceStream(pOutputFile), m_NextObjFormat(eNoPrint), m_Indent(0),
        m_TraceTypes(TraceTypes), m_bDisabled(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CTracing::CTracing(CTracing const & Tracing) :
        CTraceStream((CTraceStream&)Tracing), m_NextObjFormat(eNoPrint),
        m_Indent(Tracing.m_Indent), m_TraceTypes(Tracing.m_TraceTypes),
        m_bDisabled(Tracing.m_bDisabled)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CTracing::~CTracing()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CTracing::Initialize(COutFile* pOutputFile, unsigned int TraceTypes)
{
    SetOutputFile(pOutputFile);
    m_TraceTypes = TraceTypes;
    m_bDisabled = false;
}

eFormat
CTracing::GetNextFormat()
{
    eFormat Format = m_NextObjFormat;
    m_NextObjFormat = eNoPrint;
    
    return Format; 
}

unsigned int
CTracing::GetIndent()
{
    unsigned int Indent = m_Indent;
    m_Indent = 0;
    return Indent;
}

// operator for accessing the stream (the argument indicates the
// type of tracing at this point, so that we can determine whether
// this type of tracing is currently on)

CTraceStream&
CTracing::operator()(unsigned int TraceType)
{
    m_bTraceOn = (m_TraceTypes & TraceType);
    
    return (CTraceStream&)*this;
}

void
CTracing::SetTraceTypes(unsigned int TraceTypes)
{
    m_TraceTypes |= TraceTypes;
}

void
CTracing::UnsetTraceTypes(unsigned int TraceTypes)
{
    m_TraceTypes &= ~TraceTypes;
}
