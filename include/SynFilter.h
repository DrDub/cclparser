#ifndef __SYNFILTER_H__
#define __SYNFILTER_H__

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
#include "SynStruct.h"
#include "CmdArgs.h"
#include "Tracing.h"

class CSynFilter : public CRef
{
protected:
    // Tracing object
    CpCTracing m_pTracing;
public:
    CSynFilter() : m_pTracing(new CTracing()) {}
    virtual ~CSynFilter() {}

    // determines whether the given syntactic structure is matched by th filter
    virtual bool Match(CSynStruct* pSyn) = 0;

    // (Re-)set the tracing object with the given parameters
    virtual void SetTracing(COutFile* pOutputFile, unsigned int TraceTypes);

    // Return true if the filter is trivial (matches all structures).
    // The default is that the filter does not match all structures.
    virtual bool MatchesAll() { return false; }
};

typedef CPtr<CSynFilter> CpCSynFilter;

//
// conjunction filter
//

// This filter returns a 'true' match if the syntactic structure matches
// all filters in the list. If the list is empty, 'true' is returned.

class CSynAndFilter : public CSynFilter
{
private:
    std::vector<CpCSynFilter> m_Filters;
public:
    // initialize with first filter (can also be NULL and be added later).
    CSynAndFilter(CSynFilter* pFirstFilter);
    // Create a filter based on the command line arguments
    CSynAndFilter(CCmdArgOpts* pArgs);
    ~CSynAndFilter();

    void AddFilter(CSynFilter* pFilter);
    bool Match(CSynStruct* pSyn);

    // (Re-)set the tracing object with the given parameters
    void SetTracing(COutFile* pOutputFile, unsigned int TraceTypes);

    // Return true if the filter is trivial (an empty conjunction).
    bool MatchesAll() { return !m_Filters.size(); }
};

typedef CPtr<CSynAndFilter> CpCSynAndFilter;
    
/////////////////////////////
// Specific Filter Classes //
/////////////////////////////

//
// Word count filter
//

class CSynWordCountFilter : public CSynFilter
{
private:
    unsigned int m_WordCount;
public:
    CSynWordCountFilter(unsigned int WordCount) :
        m_WordCount(WordCount) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    
    ~CSynWordCountFilter() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }

    // Modify the count limit defined by the filter. A value of 0 means
    // that all structures match. This allows for temporary disabling
    // of filtering.
    void ResetCountLimit(unsigned int WordCount) {
        m_WordCount = WordCount;
    }
    
    bool Match(CSynStruct* pSyn);
};

typedef CPtr<CSynWordCountFilter> CpCSynWordCountFilter;

//
// Top tag filter
//

// This filter matches a syntactic structure based on the top tag of the
// syntactic structure. If m_bNeg == true then a structure matches iff its
// top tag is not equal to one of the given tags.
// If m_bNeg = false then a structure
// matches iff its top tag is equal to one of the given tag.

class CSynTopTagFilter : public CSynFilter
{
private:
    std::vector<std::string> m_Tags;
    bool m_bNeg;
public:
    CSynTopTagFilter(std::vector<std::string> const& Tags, bool bNeg);
    ~CSynTopTagFilter();
    
    bool Match(CSynStruct* pSyn);
};

typedef CPtr<CSynTopTagFilter> CpCTopTagFilter;

//
// Non-trivial structure filter
//

// This filter matches only syntactic structures which have more than
// one bracket covering more than a single word. Brackets covering
// the same words are considered identical. The top bracket is considered
// for this decision.

class CSynNonTrivialFilter : public CSynFilter
{
public:
    CSynNonTrivialFilter();
    ~CSynNonTrivialFilter();

    bool Match(CSynStruct* pSyn);
};

typedef CPtr<CSynNonTrivialFilter> CpCSynNonTrivialFilter;

#endif /* __SYNFILTER_H__ */
