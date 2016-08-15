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

#include "EvaluatorTable.h"

using namespace std;

CEvaluatorTable::CEvaluatorTable() : m_Error(false)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
}

CEvaluatorTable::~CEvaluatorTable()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

static struct SEvalTypeNames {
    string m_Name;
    CEvaluatorTable::eEvalTypes m_Type;
} EvalTypeNames[] = {
    "PrecisionAndRecall", CEvaluatorTable::ePrecisionAndRecall,
    "PR", CEvaluatorTable::ePrecisionAndRecall,
    "PrecisionAndRecall", CEvaluatorTable::ePrecisionAndRecall,
    "PRTag", CEvaluatorTable::ePnRByTag,
    "PRSTag", CEvaluatorTable::ePnRByShortTag,
    "PRTagSeq", CEvaluatorTable::ePnRByTagSeq,
    "PRSTagSeq", CEvaluatorTable::ePnRByShortTagSeq,
    "PRLen", CEvaluatorTable::ePnRByLen,
    "None", CEvaluatorTable::eNone,
    "", CEvaluatorTable::eError
};

static CEvaluatorTable::eEvalTypes
GetEvalType(string const& EvalTypeName)
{
    for(struct SEvalTypeNames* Types = EvalTypeNames ;
        Types && Types[0].m_Name.size() ; Types++) {
        if(Types->m_Name == EvalTypeName)
            return Types->m_Type;
    }

    return CEvaluatorTable::eError;
}

CEvaluator*
CEvaluatorTable::CreateEvaluator(eEvalTypes Type)
{
    switch(Type) {
        case ePrecisionAndRecall:
            return new CPrecisionAndRecall(true);
        case ePnRByTag:
            return new CEvalGroupedPnR(true, CEvalGroupedPnR::eTag);
        case ePnRByShortTag:
            return new CEvalGroupedPnR(true, CEvalGroupedPnR::eShortTag);
        case ePnRByTagSeq:
            return new CEvalGroupedPnR(true, CEvalGroupedPnR::eTagSeq);
        case ePnRByShortTagSeq:
            return new CEvalGroupedPnR(true, CEvalGroupedPnR::eShortTagSeq);
        case ePnRByLen:
            return new CEvalGroupedPnR(true, CEvalGroupedPnR::eLen);
        default:
            return NULL;
    }
}

bool
CEvaluatorTable::SetCurrentEvaluators(vector<string>& Evaluators)
{
    m_Evaluators.clear();
    
    for(vector<string>::iterator Iter = Evaluators.begin() ;
        Iter != Evaluators.end() ; Iter++) {
        // Each evaluator entry has the format [<name>:]<type>
        eEvalTypes Type;
        string Name;
        if((*Iter).find(':',0) == string::npos) {
            // Type only
            Type = GetEvalType(*Iter); 
        } else {
            Name = (*Iter).substr(0, (*Iter).find(':',0));
            Type = GetEvalType((*Iter).substr((*Iter).find(':',0)+1));
        }

        if(Type == eNone)
            continue;
        
        if(Type == eError) {
            SetError("Incorrect evaluator specification: " + (*Iter));
            return false;
        }

        CpCEvaluator pNextEvaluator;
        
        // If the evaluator has a name, look it up in the table and
        // if it is new, store it in the table.
        if(Name.size()) {
            map<string, CpCEvaluator>::iterator NamedEval =
                m_NamedEvaluators.find(Name);

            if(NamedEval == m_NamedEvaluators.end()) {
                // not found, add it
                m_NamedEvaluators[Name] = pNextEvaluator =
                    CreateEvaluator(Type);
            } else {
                pNextEvaluator = (*NamedEval).second;
            }
        } else
            pNextEvaluator = CreateEvaluator(Type);

        pNextEvaluator->Activate(); // just in case (should be active already)
        m_Evaluators.push_back(pNextEvaluator);
    }

    return true;
}
