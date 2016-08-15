#ifndef __LOOP_H__
#define __LOOP_H__

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
#include "Reference.h"
#include "InFile.h"
#include "CmdArgs.h"
#include "MessageLine.h"
#include "OutFile.h"

// This base class reads a multi-file line by line.
// For every line read, it calls a function which must be implemented by
// derived classes and which allows the derived class to process the data.
// At the end of the multi-file an exit function implemented by the
// derived class is called.

class CLoop : public CRef
{
private:
    CMultiInFile m_InFiles;  // multi-file serving as input.
    CpCCmdArgOpts m_Args;
    CpCMessageLine m_MsgLine;
    bool m_Error;
    std::string m_ErrorStr;
    // stream for output
    CpCOutFile m_pOutFile;

    // Counters and counting
    
    unsigned int m_ObjNum; // number of object currently being processed
    // When this flag is set, the derived class should only parse the
    // input to determine the number of objects and should not process
    // it any further.
    bool m_bCountOnly;
    
    // number of objects processed (this is updated by the processing class)
    unsigned int m_ObjsProcessed;
public:
    CLoop(std::vector<std::string> const & InFilePatterns, CCmdArgOpts* pArgs,
          CpCMessageLine& MsgLine, COutFile* pOutFile);
    CLoop(std::vector<std::string> const & InFilePatterns, CCmdArgOpts* pArgs,
          COutFile* pOutFile);
    ~CLoop();
    virtual bool ResetLoop();
    // run the loop. Returns false on error.
    bool DoLoop();

    // functions implemented by derived classes

    // Process the next line
    virtual bool NextLine(std::string const& Line) = 0;
    // Called to indicate that the end of the input has been reached.
    virtual bool EndLoop() = 0;

    // Access to command line arguments

    CCmdArgOpts* GetArgs() { return m_Args; }
    
    // Access to error information
    bool IsError() { return m_Error; }
    std::string const& GetErrorStr() { return m_ErrorStr; }
protected:
    void SetErrorStr(std::string const& Str) {
        m_ErrorStr = Str;
        m_Error = true;
    }
    
    // Opens the output file on the loop according to the path given
    // by the argument structure and the given suffix.
    bool OpenOutFileFromArgs(std::string const& Suffix);
    // Returns an output stream. If it is not open, tries to open it.
    // If fails to open it, returns stdout.
public:
    std::ostream& GetOutputStream();
    // Return a pointer to the output stream object. If it is not open,
    // attempt to open it.
    CRefOStream* GetOutputStreamObj();
    COutFile* GetOutFile() { return m_pOutFile; }
protected:
    // This function should be defined by derived classes to determine
    // object counting. This function should return the number of objects
    // terminated during the last call to NextLine.
    virtual unsigned int IncObjNum() = 0;
    // Should be called after an object was processed.
    void IncObjsProcessed() { m_ObjsProcessed += 1; }
public:
    void SetCountOnly(bool bCountOnly) { m_bCountOnly = bCountOnly; }
    bool CountOnly() {
        return (m_bCountOnly || m_ObjNum < m_Args->GetFirstObjToProcess()); }

    // Number of the object currently being read (after the loop terminates
    // this is the number of objects read, unless an error occurred,
    // in which case the last object was not yet fully read, perhaps).
    unsigned int GetObjNum() { return m_ObjNum; }

    // Number of objects processed
    unsigned int GetObjsProcessedNum() { return m_ObjsProcessed; }
};

typedef CPtr<CLoop> CpCLoop; 

#endif /* __LOOP_H__ */
