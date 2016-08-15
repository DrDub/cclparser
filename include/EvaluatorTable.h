#ifndef __EVALUATORTABLE_H__
#define __EVALUATORTABLE_H__

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
#include <map>
#include "Evaluator.h"

class CEvaluatorTable : public CRef
{
public:
    // Evaluator types
    enum eEvalTypes {
        eNone,
        eError,
        ePrecisionAndRecall,
        ePnRByTag,
        ePnRByShortTag,
        ePnRByTagSeq,
        ePnRByShortTagSeq,
        ePnRByLen,
    };
private:    
    // Current evaluators
    std::vector<CpCEvaluator> m_Evaluators;
    // List of named evaluators (for reuse).
    std::map<std::string, CpCEvaluator> m_NamedEvaluators;

    // error messages
    bool m_Error;
    std::string m_ErrorStr;
public:
    CEvaluatorTable();
    ~CEvaluatorTable();

private:
    // Creates (a non-active) evaluator of the given type
    CEvaluator* CreateEvaluator(eEvalTypes Type);

public:
    // Read a list of evaluator descriptions and set the corresponding
    // evaluators as the current evaluators.
    // Returns false if an error occurred.
    bool SetCurrentEvaluators(std::vector<std::string>& Evaluators);
    // Get vector of evaluators
    std::vector<CpCEvaluator>& Evaluators() { return m_Evaluators; }
    // Get iterators over the current evaluators
    std::vector<CpCEvaluator>::iterator Begin() {
        return m_Evaluators.begin();
    }
    std::vector<CpCEvaluator>::iterator End() {
        return m_Evaluators.end();
    }
    
    // check error status
    bool IsError() { return m_Error; }
    std::string& GetErrorStr() { return m_ErrorStr; }
protected:
    void SetError(std::string const& ErrorMsg) {
        m_ErrorStr = ErrorMsg;
        m_Error = true;
    }
};

#endif /* __EVALUATORTABLE_H__ */
