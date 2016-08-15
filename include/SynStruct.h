#ifndef __SYNSTRUCT_H__
#define __SYNSTRUCT_H__

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

#include <ostream>
#include <string>
#include <vector>
#include "Reference.h"

// These objects describe the structure of a parse. Every parse is described
// as a list of nodes, both terminal and non-terminal, and the domination
// relations between them. The nodes are numbered, with terminal nodes
// receiving numbers >= 0 (according to their position in the sentence)
// and non-terminal nodes receiving numbers < 0.
// Terminal nodes can only be dominated (but see below for dependency
// structures). The non-terminal nodes can both dominate and be dominated.
// A non-terminal node can dominate any node with a higher number than itself.
// note that since the terminals have positive numbers and non-terminals
// have negative numbers, any terminal node can be dominated by any
// non-terminal node.
// Every dominating node may optionally have a head among the nodes it
// dominates (in this case the other nodes dominated by this dominating
// node can be seen as dominated by the head node in a dependency
// interpretation of the structure).
// This seems similar to the format chosen for the Corpus Gesproken
// Nederlands and allows mixing of dependency and constituent structure.
// In addition, we support dominating nodes where all dominated
// nodes are "alternatives". This should be used for coordination
// structures (not yet implemented).
// Every node has a label associated with it. Terminal labels also carry
// the word associated with them (or punctutation). A terminal node
// is also marked for additional properties (see below).

// forward declaration
class CSynStruct;

// Class to hold pointer to syntactic structure (this structure is attached
// to all nodes and allows them to access other nodes)

class CSynStructPtr : public CRef
{
    friend class CSynStruct;
private:
    CSynStruct* m_pStruct;

    CSynStructPtr(CSynStruct* pStruct) : m_pStruct(pStruct) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
public:
    ~CSynStructPtr() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
    }
    
public:
    CSynStruct* Ptr() { return m_pStruct; }
};

typedef CPtr<CSynStructPtr> CpCSynStructPtr;

// Basic node

class CSynNode : public CRef
{
private:
    std::string m_Tag;    // Node tag
    // When a tag has multiple parts, this holds the parts of the tag.
    // The use of this field is optional and the list may remain empty.
    std::vector<std::string> m_TagParts;
    // nodes directly dominating this node
    std::vector<int> m_Dominating;
protected:
    // object holding the pointer to the CSynStruct object containing this node
    CpCSynStructPtr m_pStruct; 
public:
    
    CSynNode(CSynStructPtr* pStruct) : m_Tag(), m_pStruct(pStruct) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    
    CSynNode(CSynStructPtr* pStruct, std::string const& Tag) :
            m_pStruct(pStruct), m_Tag(Tag) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }

    // this constructor also splits the tag if a splitting character is given
    CSynNode(CSynStructPtr* pStruct, std::string const& Tag,
             char TagSplitChar);
    
    ~CSynNode() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif       
    }

    void AddDominating(int NodeNum) { m_Dominating.push_back(NodeNum); }
    std::string const& Tag() { return m_Tag; }
    std::vector<std::string> const& TagParts() { return m_TagParts; }
    void SetTag(std::string const& Tag) { m_Tag = Tag; m_TagParts.clear(); }
    // also splits the tag if a splitting character is given
    void SetTag(std::string const& Tag, char TagSplitChar);
    std::vector<int>::iterator DominatingBegin() {
        return m_Dominating.begin();
    }
    std::vector<int>::iterator DominatingEnd() {
        return m_Dominating.end();
    }
};

typedef CPtr<CSynNode> CpCSynNode;

// Terminal node

class CTerminal : public CSynNode
{
private:
    std::string m_Unit;   // word, punctutation, etc.
    bool m_bPunct;        // is this node a punctuation symbol ?
    bool m_bEmpty;        // is this an empty node (trace, etc.)
public:
    CTerminal(CSynStructPtr* pStruct) :
            CSynNode(pStruct, ""), m_Unit(""), m_bPunct(false),
            m_bEmpty(true) {
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    
    CTerminal(CSynStructPtr* pStruct, std::string const& Unit,
              std::string const& Tag, char TagSplitChar,
              bool bPunct = false, bool bEmpty = false) :
            CSynNode(pStruct, Tag, TagSplitChar), m_Unit(Unit),
            m_bPunct(bPunct), m_bEmpty(bEmpty)
        {    
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }

    ~CTerminal() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif       
    }

    std::string const& Unit() { return m_Unit; }
    bool Punct() { return m_bPunct; }
    bool Empty() { return m_bEmpty; }
};

typedef CPtr<CTerminal> CpCTerminal;

// Non-terminal node

class CNonTerminal : public CSynNode
{
private:
    // nodes directly dominated by this node
    std::vector<int> m_Dominated;
    // head node among the dominated nodes (position in array, -1 if none)
    int m_Head;
    // Was the head inferred from the syntactic structure or was it given
    // explicitly in the course of construction ?
    bool m_bInferredHead;
public:
    CNonTerminal(CSynStructPtr* pStruct) :
            CSynNode(pStruct), m_Head(-1), m_bInferredHead(false) {    
#ifdef DETAILED_DEBUG
        IncObjCount();
#endif
    }
    
    CNonTerminal(CSynStructPtr* pStruct, std::string const& Tag,
                 char TagSplitChar, std::vector<int> const& SubNodes,
                 bool bInferHead = false, int Head = -1);

    ~CNonTerminal() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif       
    }

    std::vector<int>::iterator DominatedBegin() {
        return m_Dominated.begin();
    }
    std::vector<int>::iterator DominatedEnd() {
        return m_Dominated.end();
    }

    // Calculates the inferred head of this non-terminal.
    void CalcInferredHead();
    
    // returns CSynStruct::NoNode if there is no head 
    int GetHead();

    // Is the head (if exists) an inferred head (or explicitly given)
    bool IsInferredHead() { return m_bInferredHead; }
    
    // returns the number of nodes dominated by this non-terminal
    unsigned int NumDominated() { return m_Dominated.size(); }
};

typedef CPtr<CNonTerminal> CpCNonTerminal;

class CSynStruct : public CRef
{
private:
    // object holding a pointer to this structure
    CpCSynStructPtr m_pThis;
    // character used to split terminal tags (0 if no splitting)
    char m_TerminalTagSplitChar;
    // character used to split non-terminal tags (0 if no splitting)
    char m_NonTerminalTagSplitChar;
    // vector of terminals (positon in vector is equal to the node number) 
    std::vector<CpCTerminal> m_Terminals;
    // vector of non-terminals (position in vector is equal to the node number)
    std::vector<CpCNonTerminal> m_NonTerminals;
public:
    // the largest possible integer 
    static const int NoNode = (int)((unsigned int)(-1) >> 1);
    
    CSynStruct() : m_pThis(new CSynStructPtr(this)),
                   m_TerminalTagSplitChar(), m_NonTerminalTagSplitChar()
        {
#ifdef DETAILED_DEBUG
            IncObjCount();
#endif       
        }
    CSynStruct(char TerminalTagSplitChar, char NonTerminalTagSplitChar)
            : m_pThis(new CSynStructPtr(this)),
              m_TerminalTagSplitChar(TerminalTagSplitChar),
              m_NonTerminalTagSplitChar(NonTerminalTagSplitChar)
        {
#ifdef DETAILED_DEBUG
            IncObjCount();
#endif       
        }
    virtual ~CSynStruct() {
#ifdef DETAILED_DEBUG
        DecObjCount();
#endif
        m_pThis->m_pStruct = NULL;
    }
public:
    static int NonTerm2NodeNum(int const NonTerm) { return -(NonTerm+1); }
    static int NodeNum2NonTerm(int const NodeNum) { return -(NodeNum+1); }

    CSynNode* GetNode(int NodeNum);

    int AddTerminal(std::string const& Unit, std::string const& Tag,
                    bool bPunct = false, bool bEmpty = false);
    int AddTerminal(std::string const& Unit, std::string const& Tag,
                    unsigned int Pos,
                    bool bPunct = false, bool bEmpty = false);
    int AddNonTerminal(std::string const& Tag,
                       std::vector<int>& SubNodes, bool bInferHead = false,
                       int Head = -1);
    
    // get the pointer object

    CSynStructPtr* This() { return m_pThis; }
    
    // terminal iterator

    std::vector<CpCTerminal>::iterator TerminalsBegin() {
        return m_Terminals.begin();
    }

    std::vector<CpCTerminal>::iterator TerminalsEnd() {
        return m_Terminals.end();
    }

    // non-terminal iterator

    std::vector<CpCNonTerminal>::iterator NonTerminalsBegin() {
        return m_NonTerminals.begin();
    }

    std::vector<CpCNonTerminal>::iterator NonTerminalsEnd() {
        return m_NonTerminals.end();
    }

    // Returns the number of terminals ignoring empty nodes if
    // bNoEmpty is set and ignoring punctuation if bNoPunct is set 
    unsigned int CountTerminals(bool bNoEmpty, bool bNoPunct);
    // Returns the number of terminals which are not punctuation or empty
    unsigned int CountContentUnits();
    // Return the number of non-terminals
    unsigned int NonTerminalNum() { return m_NonTerminals.size(); }
    // Return the number of terminals
    unsigned int TerminalNum() { return m_Terminals.size(); }

    // returns the terminal with the give node number (NULL if not found)
    CTerminal* TerminalByNode(int NodeNum) {
        if(NodeNum < 0 || m_Terminals.size() <= NodeNum)
            return NULL;
        return m_Terminals[NodeNum];
    }
    // returns the non-terminal with the give node number (NULL if not found)
    CNonTerminal* NonTerminalByNode(int NodeNum);

    // Calculates (recursively) all inferred heads
    void CalcInferredHeads();

    // Set the tags of the terminals based on those of the given synstruct.
    // This assumes that the two synstructs represent the same sequence
    // of terminal, possibly ignoring empty nodes (if bNoEmpty is set)
    // and punctuation (if bNoPunct) is set. 
    // Returns 'false' if this assumption fails.
    bool SetTerminalTagsFromSuperSequence(CSynStruct* pSuper,
                                          bool bNoEmpty, bool bNoPunct);
    
    // printing routines
    void SimplePrint(std::ostream& Out);
};

typedef CPtr<CSynStruct> CpCSynStruct;

#endif /* __SYNSTRUCT_H__ */
