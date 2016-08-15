#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

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
#include "RefStream.h"
#include "SynStruct.h"
#include "Tracing.h"
#include "Hash.h"
#include "HashKey.h"

// The following class is a base class used to implement different evaluation
// metrics.

class CEvaluator : public CRef
{
public:
    enum eEvalType {
        eSyn, // evaluation based on the syntactic structure
    };
protected:
    // Evaluator type
    eEvalType m_Type;
    
    //
    // Tracing
    //
    
    // Tracing object
    CTracing m_Tracing;
public:
    CEvaluator(eEvalType Type) : m_Type(Type) {}
    ~CEvaluator() {}

    eEvalType GetType() { return m_Type; }
    
    // (Re-)set the tracing object with the given parameters
    void SetTracing(COutFile* pOutputFile, unsigned int TraceTypes);
    
    // Receives two syn structs and adds the result of their evaluation
    // to the total evaluation. 'pStandard' is the gold standard against
    // which 'pParsed' is evaluated.
    virtual void FromSynStruct(CSynStruct* pStandard, CSynStruct* pParsed) = 0;
    // Function to activate calculation of evaluation
    virtual void Activate() = 0;
    // Function to deactivate calculation of evaluation
    virtual void Deactivate() = 0;

    //
    // Evaluator printing functions
    //

    // Prints the evaluation for the last structure evaluated
    // (this may not be relevant for some evaluators)
    virtual void PrintLastEval(std::ostream& Out) = 0;
    // Print evaluation for all structures processed until now.
    virtual void PrintTotalEval(std::ostream& Out) = 0;
};

typedef CPtr<CEvaluator> CpCEvaluator;

//
// Bracket precision an recall evaluator
//

class CPrecisionAndRecall : public CEvaluator
{
private:
    // When the object is inactive, it does not change the evaluation
    // scores (ignoring all requests to update it).
    bool m_bActive;
    // Accumulative sum of number of nodes in parsed structure which are
    // also in the standard structure
    float m_Precision;
    // Accumulative sum over the maximal possible precision (number of
    // nodes in parsed structure)
    float m_PrecisionMax;
    // Accumulative sum of number of nodes in standard structure which are
    // also in the parsed structure
    float m_Recall;
    // Accumulative sum over the maximal possible recall (number of
    // nodes in standard structure)
    float m_RecallMax;

    // Same values as above, but only for the last structure evaluated
    float m_LastPrecision;
    float m_LastPrecisionMax;
    float m_LastRecall;
    float m_LastRecallMax;
    
public:
    CPrecisionAndRecall(bool bActive);
    ~CPrecisionAndRecall();

    void FromSynStruct(CSynStruct* pStandard, CSynStruct* pParsed);
    
    // set the evaluation to active/unactive
    void Activate() { m_bActive = true; }
    void Deactivate() { m_bActive = false; }
    
    // return the current total precision score
    float Precision();
    // return the current total recall score
    float Recall();

    // return the current last precision score
    float LastPrecision();
    // return the current last recall score
    float LastRecall();
    
    // Prints the evaluation for the last structure evaluated
    // (this may not be relevant for some evaluators)
    void PrintLastEval(std::ostream& Out);
    // Print evaluation for all structures processed until now.
    void PrintTotalEval(std::ostream& Out);
};

//
// Precision and recall evaluator for constituents grouped by non-terminal
// tag or underlying terminal tag sequence.
//


// hash table value to record statistics for each label

class CEvalGroupedVal : public CRef
{
public:
    unsigned int m_Expected; // expected (standard)
    unsigned int m_Observed; // observed in evaluated structure
    unsigned int m_Matched;  // match between the standard and observed

    CEvalGroupedVal() : m_Expected(0), m_Observed(0), m_Matched(0) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }

    ~CEvalGroupedVal() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
};

typedef CPtr<CEvalGroupedVal> CpCEvalGroupedVal;

typedef CHash<CStrKey, CEvalGroupedVal> CEvalGroupedHash;
typedef CPtr<CEvalGroupedHash> CpCEvalGroupedHash;

class CEvalGroupedPnR : public CEvaluator
{
public:
    // type of grouping used
    enum eGrouping {
        eTag, // grouping by non-terminal tag
        eShortTag, // same as 'eTag' but using only first part of tag
        eTagSeq, // grouping by terminal tag sequence
        eShortTagSeq, // same as 'eTagSeq' but using only first part of tag
        eLen, // grouping by bracket length
    };
    
private:
    // When the object is inactive, it does not change the evaluation
    // scores (ignoring all requests to update it).
    bool m_bActive;
    // Grouping type
    eGrouping m_GroupingType;
    // hash to collect matches/mismatches
    CpCEvalGroupedHash m_pHash;

    // expected unmatched labels of last evaluation
    std::vector<std::string> m_Expected;
    // observed unmatched labels of last evaluation
    std::vector<std::string> m_Observed;
    // matched labels of last evaluation
    std::vector<std::string> m_Matched;
    
public:

    // 'bActive' determines whether the evaluator is initialized in active
    // or inactive mode. 'GroupingType' determines the criterion
    // to be used for grouping.
    CEvalGroupedPnR(bool bActive, eGrouping GroupingType);
    ~CEvalGroupedPnR();

private:
    // Returns the hash entry for the given string. Creates a new one if
    // the entry does not yet exist.
    CEvalGroupedVal* GetHashVal(std::string const& Key);

public:
    void FromSynStruct(CSynStruct* pStandard, CSynStruct* pParsed);
    
    // set the evaluation to active/unactive
    void Activate() { m_bActive = true; }
    void Deactivate() { m_bActive = false; }
    
    // Prints the evaluation for the last structure evaluated
    // (this may not be relevant for some evaluators)
    void PrintLastEval(std::ostream& Out);
    // Print evaluation for all structures processed until now.
    void PrintTotalEval(std::ostream& Out);
};

#endif /* __EVALUATOR_H__ */
