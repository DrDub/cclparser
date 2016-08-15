#ifndef __TRACING_H__
#define __TRACING_H__

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

#include "OutFile.h"
#include "PrintUtils.h"

//
// The classes and macros in this file define the interface for outputing
// trace information for parsing.
//

// To ensure that tracing code can be excluded from the executable, it
// should alway be included within a Tracing { <tracing code> } block.

#ifdef TRACING
#define DO_TRACE if(1)
#else
#define DO_TRACE if(0)
#endif

//
// Bits describing the different types of tracing that may be activated
//

#define TB_UTTER       0x0001  // print the utterance being processed
#define TB_CCL_SET     0x0002  // print the CCL set
#define TB_PARSER      0x0004  // processing by the parser
#define TB_FILTER      0x0008  // processing by a filter
// Evaluator tracing
#define TB_EVAL        0x0010  // print evaluator debug information

//
// Tracing stream class which supports the insertion of special parsing
// objects in addition to the standard types.
// This is a base class which is inherited by the tracing objects which
// define the printing format for each object.

class CTraceStream
{
private:
    CpCOutFile m_pOutputFile; // pointer to the output stream
public:
    // constructors and destructors
    CTraceStream(COutFile* pOutputFile);
    CTraceStream(CTraceStream const & TraceStream);

    // The following function allows the output stream to be changed after
    // construction
    void SetOutputFile(COutFile* pOutputFile);
private:

    // format control functions (all purely virtual)
    
    virtual bool TracingOn() = 0;

    // returns the next object format
    
    virtual eFormat GetNextFormat() = 0;

    // return the current indentation
    
    virtual unsigned int GetIndent() = 0;
    
public:
    // format setting

    virtual void SetNextFormat(eFormat Format) = 0;
    virtual void IncIndent() = 0;
    virtual void DecIndent() = 0;
    virtual void SetIndent(unsigned int Indent) = 0;
    
public:
    // template for the insertion operator for standard types
    template <typename T>
    CTraceStream& operator<<(T const & Input) {
        // We ignore the format and indentation but have to read them since
        // this resets their values for the next object
        GetNextFormat();
        GetIndent();
        if(m_pOutputFile && TracingOn())
            ((std::ostream&)*m_pOutputFile) << Input;
        return *this;
    }
    
    // insertion operator for manipulators
    CTraceStream& operator<<(CTraceStream& (*Func)(CTraceStream& TraceStream)){
        return (*Func)(*this);
    }
    
    // insertion operators for special objects    
    CTraceStream& operator<<(CPrintObj* pPrintObj);
    CTraceStream& operator<<(CPrintObj& PrintObj);

    // Print a full tab to the stream
    CTraceStream& Tab();
    // Print a thrid tab to the stream
    CTraceStream& Tab_1_3();
    // print a end of line to the stream
    CTraceStream& Endl();
};

//
// Manipulators
//

// Manipulators for setting the format
extern CTraceStream& Long(CTraceStream& TraceStream);
extern CTraceStream& Short(CTraceStream& TraceStream);

// Indentation manipulators
extern CTraceStream& Indent(CTraceStream& TraceStream);
extern CTraceStream& UnIndent(CTraceStream& TraceStream);
extern CTraceStream& NoIndent(CTraceStream& TraceStream);

// Tab manipulators
// print a tab to the stream
extern CTraceStream& Tab(CTraceStream& TraceStream);
// print a third tab to the stream
extern CTraceStream& Tab_1_3(CTraceStream& TraceStream);
// End of line manipulator
extern CTraceStream& Endl(CTraceStream& TraceStream);

// The CTracing class has an output stream interface which supports, in
// addition to the standard types, also special parser objects. The CTracing
// class determines which of the messages printed to it will actually
// be printed out, in which format and to which output stream.
//
// A CTrace object may point to a higher object. 

class CTracing : private CTraceStream, public CRef
{
private:
    // Bits indicating the types of tracings which are currently on
    unsigned int m_TraceTypes;
    // If this flag is set, all tracing is disabled
    bool m_bDisabled;
    // Print/don't print flag for the next object
    bool m_bTraceOn;
    // Format of the next object to be printed (applies only once)
    // Ignored if set to eNoPrint.
    eFormat m_NextObjFormat;
    // Current indentation
    unsigned int m_Indent;
public:
    CTracing();
    CTracing(COutFile* pOutputFile, unsigned int TraceTypes);
    CTracing(CTracing const & Tracing);
    ~CTracing();

    // (Re-)initialize the tracing object
    void Initialize(COutFile* pOutputFile, unsigned int TraceTypes);
private:

    // format control 

    bool TracingOn() { return !m_bDisabled && m_bTraceOn; }

    // returns the next object format and resets the next object
    // format to eNoPrint
    eFormat GetNextFormat();

    // Returns the current indentation
    unsigned int GetIndent();
public:
    void SetNextFormat(eFormat Format) { m_NextObjFormat = Format; }

    void IncIndent() { m_Indent++; }
    void DecIndent() { if(m_Indent) m_Indent--; }
    void SetIndent(unsigned int Indent) { m_Indent = Indent; }
public:

    // operator for accessing the stream (the argument indicates where the
    // stream has been called so that it can be determined whether to
    // output the message to the stream

    CTraceStream& operator()(unsigned int TraceType);

    // Check whether tracing is on for the given trace type
    bool IsTraceOn(unsigned int TraceType) {
        return (m_TraceTypes & TraceType);
    }
    
    // Disable tracing
    void Disable() { m_bDisabled = true; }
    // Enable tracing
    void Enable() { m_bDisabled = false; }
    
    // Functions for setting and unsetting specific trace types (this allows
    // to modify the type of tracing performed for specific objects)
    void SetTraceTypes(unsigned int TraceTypes);
    void UnsetTraceTypes(unsigned int TraceTypes);
    // return the tracing flags current set
    unsigned int GetTraceTypes() { return m_TraceTypes; }
};

typedef CPtr<CTracing> CpCTracing;

#endif /* __TRACING_H__ */
