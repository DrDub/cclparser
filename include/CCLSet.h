#ifndef __CCLSET_H__
#define __CCLSET_H__

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
#include <list>
#include "PrsConst.h"
#include "RefSTL.h"
#include "CCLUnitDecl.h"
#include "Tracing.h"

class CCCLSet;
class CCCLPrefixIncAddableIter;

// The classes in this file describes a common cover link set. In addition
// to storing the links, it offers a range of functions to determine
// properties of the set.

//
// Simple auxiliary structure which holds a pair: position + depth
//

struct SLinkPair
{
    int m_End;   // one end of a link (either head or base)
    unsigned int m_Depth;

    SLinkPair() : m_End(0), m_Depth((unsigned int)(-1)) {}
    SLinkPair(int End, unsigned int Depth) :
            m_End(End), m_Depth(Depth) {}
    SLinkPair(SLinkPair const& LinkPair) :
            m_End(LinkPair.m_End), m_Depth(LinkPair.m_Depth) {}
    SLinkPair& operator=(SLinkPair const& LinkPair) {
        m_End = LinkPair.m_End;
        m_Depth = LinkPair.m_Depth;
    }
};

//
// An entry describing the link properties on one side of a unit.
//

class CCCLNodeSide
{
    friend class CCCLNode;
    
private:
    // list of outbound links (pairs: head position and depth)
    std::vector<SLinkPair> m_Outbound;
    // Head of last link of depth zero ((unsigned int)(-1) if no such link)
    unsigned int m_LastOutbound0;
    // list of inbound links (pairs: base position and depth).
    std::vector<SLinkPair> m_Inbound;

    // Outbound paths. Position x in the vector points to an integer
    // which holds the furthest position which can be reached from the current
    // node by a path which begins with a link of depth <= x.
    // (the position of the node itself is used if there is no such path).
    // Other nodes point to the last object in this list.
    std::vector<CpCRefInt> m_Paths;
    
    // Complete blocking position (the first position beyond which blocking
    // applies). -1 indicates that there is no such blocking.

    // first position of complete blocking
    CpCRefInt m_pCompleteBlock;
    
    // The constructor initializes an empty object with the given position
    // as the directly adjacent position.
    CCCLNodeSide();
    ~CCCLNodeSide() {}
};

//
// An entry describing the link properties on both sides of a unit.
//

class CCCLNode : public CRef
{
private:
    unsigned int m_Pos; // position of the current node in the list
    CCCLNodeSide m_Side[SIDE_NUM];

public:
    CCCLNode(unsigned int Pos);
    ~CCCLNode();

    unsigned int GetPos() { return m_Pos; }
    
    // returns true if there is an inbound link on the given side of the node
    bool HasInbound(unsigned int Side);
    
    // returns the pair describing the first inbound link on the given side
    // (if there is no such link, returns the position of the current node
    // and a depth of (unsigned int)(-1))
    SLinkPair GetInbound(unsigned int Side);
    // Returns the head of the last outbound link of depth 0
    // (returns the node position if no such link exists)
    unsigned int GetLastOutbound0(unsigned int Side) {
        return m_Side[Side].m_LastOutbound0;
    }
    // returns the pair describing the last outbound link on the given side
    // (if there is no such link, returns the position of the current node
    // and a depth of (unsigned int)(-1))
    SLinkPair GetLastOutbound(unsigned int Side);
    // return the number of outbound links
    unsigned int GetOutboundNum(unsigned int Side) {
        return m_Side[Side].m_Outbound.size();
    }
    
    // return the beginning of the iterator on the outbound links
    std::vector<SLinkPair>::iterator OutboundBegin(unsigned int Side);
    // return the end of the iterator on the outbound links
    std::vector<SLinkPair>::iterator OutboundEnd(unsigned int Side);
    // return the beginning of the reverse iterator on the outbound links
    std::vector<SLinkPair>::reverse_iterator OutboundRBegin(unsigned int Side);
    // return the end of the reverse iterator on the outbound links
    std::vector<SLinkPair>::reverse_iterator OutboundREnd(unsigned int Side);
    
    // Return the end of the longest path beginning at this node
    // in the given direction (returns the position of the current node
    // if there is no such path).
    unsigned int GetLongestPath(unsigned int Side);
    // Return the end of the longest path beginning at this node
    // in the given direction with the given depth as the depth of the
    // first link (returns the position of the current node
    // if there is no such path).
    unsigned int GetLongestPath(unsigned int Side, unsigned int FirstDepth);
    // Returns true if there is a path from this node to the given position
    // (true is also returned if these two nodes are identical).
    bool HasPathTo(unsigned int Pos);
    // Return the depth of the first link on a path to the given node.
    // Returns -1 if there is no such path
    int GetPathFirstDepth(unsigned int Head);

    // Returns the blocking which would apply at the given position
    // (which can be beyond the end of the utterance). Returns 0, 1 or
    // (unsigned int)(-1).
    unsigned int Blocking(int Pos);
    
    // Returns the position and depth of the unused adjacency on the given side
    // (this may be beyond the beginning or end of the utterance).
    // This is calculated based on the longest path from this node.
    // The depth returned is (unsigned int)(-1) if there is no such adjacency.
    SLinkPair UnusedAdj(unsigned int Side);

    // Set complete blocking on the right side of the node to the given
    // position (unless there is an already earlier set value).
    void SetCompleteRightBlocking(CCCLSet* pSet, unsigned int Pos);
    // Set the maximal path position pointer on the right with the given
    // pointer. Continues to do so recursively for the furthest node
    // attached on the right.
    void ReplaceMaxRightPathPtr(CCCLSet* pSet, CRefInt* pPos);
    
    // Add a link whose base is at this node, which is the last word in the
    // utterance (head and depth of the new link are given)
    bool AddLinkAtLastWordBase(CCCLSet* pSet, CCCLNode* pHead,
                               unsigned int Depth);
    // Add a link whose head is at this node, which is in the utterance prefix
    // (not the last word).
    bool AddLinkAtPrefixHead(CCCLSet* pSet, CCCLNode* pBase,
                             unsigned int Depth);
    // Add a link whose base is at this node, which is in the prefix of the
    // utterance (head and depth of the new link are given)
    bool AddLinkAtPrefixBase(CCCLSet* pSet, CCCLNode* pHead,
                             unsigned int Depth);

    // Add a link whose head is at this node, which is the last word in the
    // utterance.
    bool AddLinkAtLastWordHead(CCCLNode* pBase, unsigned int Depth);
};

typedef CPtr<CCCLNode> CpCCCLNode;

//////////////////////////////////
// Iterator over Outbound Links //
//////////////////////////////////

// This iterator provides a 'read only' interface for the outbound
// links of a node.

class CCCLOutboundIter : public CRef
{
private:
    CpCCCLNode m_pNode; // node whose outbound links are iterated on
    unsigned int m_Side; // Side of the iterator
    std::vector<SLinkPair>::iterator m_Iter; // current position
public:
    CCCLOutboundIter(CCCLSet* pSet, unsigned int Node, unsigned int Side);
    ~CCCLOutboundIter();

    // Advance the iterator. Returns false if the end of the iterator
    // was reached.
    bool operator++();
    // Returns true if the end of the iterator was not yet reached
    operator bool();
    // returns the value at the current position of the iterator. If the
    // iterator was not properly initialized or the end has been reached.
    // returns (-1, (unsigned int)(-1)).
    operator SLinkPair();
    // Return link head at current iterator position.
    // Returns (unsigned int)(-1) if no such link exists.
    unsigned int Head();
    // Return the depth of the current iterator position.
    // Returns (unsigned int)(-1) if no such link exists.
    unsigned int Depth();
};

typedef CPtr<CCCLOutboundIter> CpCCCLOutboundIter;

/////////////////
// Depth Class //
/////////////////

// This vector describes which depths are allowed for a given link.

class CCCLDepths
{
private:
    bool m_Depths[2];
public:
    // default constructor
    CCCLDepths() { m_Depths[0] = m_Depths[1] = false; }
    // constructor from minimal depth
    CCCLDepths(int MinDepth) {
        m_Depths[0] = (MinDepth == 0);
        m_Depths[1] = (MinDepth >= 0 && MinDepth <= 1);
    }
    // copy operator
    CCCLDepths(CCCLDepths const& Depths) {
        m_Depths[0] = Depths.m_Depths[0];
        m_Depths[1] = Depths.m_Depths[1];
    }

    // reset the value of the depth vector
    void SetMin(int MinDepth) {
        m_Depths[0] = (MinDepth == 0);
        m_Depths[1] = (MinDepth >= 0 && MinDepth <= 1);
    }
    
    bool& operator[](unsigned int Depth) {
        return m_Depths[Depth];
    }

    CCCLDepths& operator=(CCCLDepths const& Depths) {
        m_Depths[0] = Depths.m_Depths[0];
        m_Depths[1] = Depths.m_Depths[1];
        return (*this);
    }

    bool operator==(CCCLDepths const& Depths) {
        return (m_Depths[0] == Depths.m_Depths[0] &&
                m_Depths[1] == Depths.m_Depths[1]);
    }
    
    // returns true if at least one of the depths is allowed
    bool Allowed() { return m_Depths[0] || m_Depths[1]; }
};

///////////////////////////
// Common Cover Link Set //
///////////////////////////

class CCCLSet : public CRef
{
    // Allow nodes direct access to other nodes.
    friend class CCCLNode;
    // Allow the outbound iterator access to node information
    friend class CCCLOutboundIter;
    // Allow the prefix adjacency iterator access to the prefix adjacency list
    friend class CCCLPrefixIncAddableIter;
    
private:    
    std::vector<CpCCCLNode> m_Nodes;
    // vector of units covered by this set
    CpCCCLUnitVector m_pUnits;

    // Minimal resolution violation (-1 when there is no violation)
    int m_RVLeftPos;
    // Left depth of the minimal resolution violation. This is
    // (unsigned int)(-1) when there is no resolution violation.
    unsigned int m_RVLeftDepth;

    // List of nodes in the prefix which have an unused adjacency with
    // the current last word. This is given as pairs: the word in the
    // prefix and the depth of the adjacency. The entries are sorted
    // in increasing distance from the next word of the utterance.
    // The list is updated every time a new word is read.
    std::list<SLinkPair> m_PrefixAdj;

    // flag to indicate that no more links may be added until the next
    // word is read
    bool m_bClosed;

    //
    // Tracing (debugging)
    //
    CpCTracing m_pTracing;     // Pointer to tracing object
    
public:
    // The set is initialized to have a single node
    CCCLSet(CTracing* pTracing);
    virtual ~CCCLSet();

    // Set the tracing object on the set
    void SetTracing(COutFile* pOutputFile, unsigned int TraceTypes);
    
    // reset to an empty set of links
    void ClearSet();
    
    // return position of last node (-1 if there are no nodes)
    int LastNode();

    // return the unit at the given position (returns NULL if the unit does
    // not exist)
    CCCLUnit* GetUnit(unsigned int Pos);
    // return the last unit in the set. Returns NULL if there is no such
    // unit.
    CCCLUnit* GetLastUnit();
    // return the vector of all units
    CCCLUnitVector* GetUnits() { return m_pUnits; }
    
    // Functions for checking whether a given link satisfies a condition
    
    // Returns true if 'Head' is adjacent to 'Base' with depth no greater
    // than 'Depth' and the adjacency is not used.
    bool UnusedAdjacency(unsigned int Base, unsigned int Head,
                         unsigned int Depth);
    // Returns true if adding the given link does not violate the equality
    // condition.
    bool Equality(unsigned int Base, unsigned int Head, unsigned int Depth);
    // Returns true if the pair (Base, Xk = Last node) is not covered by links
    // already in the set
    bool NotCoveredToXk(unsigned int Base);
    // Returns true if adding the given link does not violate forcing
    bool Forcing(unsigned int Base, unsigned int Head, unsigned int Depth);
    
    // Resolution violation

    // Returns true if there is a resolution violation in the set.
    bool HasRV();
    // Returns the left position of the minimal resolution violation
    // (return -1 if there is no resolution violation)
    int MinRVLeftPos();
    // Returns the left depth of the minimal resolution violation
    // (return (unsigned int)(-1) if there is no violation).
    unsigned int LeftRVDepth();
    // Returns the right depth of the minimal resolution violation
    // (return (unsigned int)(-1) if there is no violation).
    unsigned int RightRVDepth();

private:
    
    // This function receives the last link added to the set. It then
    // determines whether this link creates a new minimal resolution violation
    // or repairs the current minimal resolution violation.
    void UpdateRV(unsigned int Base, unsigned int Head, unsigned int Depth);

    // Updates the list of words in the prefix which have the last node
    // as an adjacent.
    void UpdatePrefixAdj();
    
public:

    //
    // Modification functions
    //
    
    // Returns true if the set can be advanced to the next node
    // (currently this means that there is no resolution violation).
    bool CanIncNodeNum();
    
    // Increments the number of nodes in the utterance covered by the set.
    // Store the given unit as the unit at the new position
    // Returns false (and does not increment) if there is a resolution
    // violation.
    virtual bool IncNodeNum(CCCLUnit* pNextUnit);

    // indicate that no more links may be added to the set until an additional
    // word is read
    bool CloseSet();
    
    // Adds the given link to the common cover link set. Returns false if
    // the link cannot be added. The link can be added only if the incremental
    // parser algorithm allows it to be added.
    bool AddLink(unsigned int Base, unsigned int Head, unsigned int Depth);
    
    //
    // Access to the link structure (read only)
    //
    
    // Returns the description of the inbound link of the given node
    // on the given side. The return value is the same as the node's
    // GetInbound function.
    SLinkPair GetInbound(unsigned int Node, unsigned int Side);

    // Returns the head of the last outbound link of the given node of depth 0
    // (returns the given node if no such link exists)
    unsigned int GetLastOutbound0(unsigned int Node, unsigned int Side);
    
    // Returns the pair describing the last outbound link on the given side
    // of teh given node. The return value is the saem as the node's
    // GetLastOutbound function.
    SLinkPair GetLastOutbound(unsigned int Node, unsigned int Side);

    // Returns the number of outbound links on the given side of the
    // given unit.
    unsigned int GetOutboundNum(unsigned int Node, unsigned int Side);
    
    // Create an iterator over the outbound links of the given node and side
    CCCLOutboundIter* GetOutboundIter(unsigned int Node, unsigned int Side);

    // Return the end of the longest path beginning at the given node
    // in the given direction (returns the position of the given node
    // if there is no such path).
    unsigned int GetLongestPath(unsigned int Node, unsigned int Side);
    // Return the end of the longest path beginning at the given node
    // in the given direction with the given depth as the depth of the
    // first link (returns the position of the given node
    // if there is no such path).
    unsigned int GetLongestPath(unsigned int Node, unsigned int Side,
                                unsigned int FirstDepth);
    // Returns the position and depth of the unused adjacency on the given side
    // of the given node (this may be beyond the beginning or end of the
    // utterance). This is calculated based on the longest path from this node.
    // The depth returned is (unsigned int)(-1) if there is no such adjacency.
    SLinkPair UnusedAdj(unsigned int Node, unsigned int Side);
    
    //
    // Link addability
    //

    // This function returns the properties of an incrementally addable
    // link whose base is at the next word. The return value of the function
    // is the head position of the addable link and the vector
    // bDepths is true for every depth which is the link may have.
    // If there is no adable link, all the bDepth entries are false. 
    int LastAddableLink(CCCLDepths& bDepths);

    // Debugging

    // This function writes the current set to the trace object
    void TraceSet();
};

typedef CPtr<CCCLSet> CpCCCLSet;

//////////////////////////////
// Unused Adjacency Classes //
//////////////////////////////

//
// Prefix incrementally addable link iterator
//

// This class is an iterator over the incrementally addable links of a CCL set
// from the prefix to the next word. Only unused adjacencies which
// may be made into links are returned. This means that once a link
// is added from the prefix to the next word, the iterator becomes
// empty. Also, when there is a resolution violation, only adjacencies
// which may be used to add links are returned.
// The iterator must be restarted every time a link is added to the
// CCL set on which it is based.
// The links are ordered at increasing distance of the bsae from the next word.

class CCCLPrefixIncAddableIter : public CRef
{
private:
    // pointer to the set object
    CpCCCLSet m_pSet;
    // beginning of the iterator
    std::list<SLinkPair>::iterator m_Begin;
    // iterator into the list of adjacencies
    std::list<SLinkPair>::iterator m_Iter;
    // Depths allowed for the current position of the iterator
    // (depth is the position in the array)
    CCCLDepths m_bDepth;
    // Set to true if the iterator is empty
    bool m_bEmpty;
    
public:
    // Construction from a set object
    CCCLPrefixIncAddableIter(CCCLSet* pSet);
    ~CCCLPrefixIncAddableIter();

private:

    // This function determines the depths allowed for the current position
    // of the iterator. If no allowed depth exists, the iterator position
    // is advanced until a position with an allowable depth is found
    // (or the end of adjacencies is reached).
    void CalcDepths();
    
public:
    
    // returns true if the iterator is empty
    bool Empty() { return m_bEmpty; }
    
    // Returns true if the current position of the iterator has reached
    // the end of the iterator
    bool End();

    // Returns true if the current position of the iterator is at the
    // beginning of the iterator.
    bool Begin();

    // Reset the iterator to the beginning. Returns false if the iterator
    // is empty.
    bool Restart();

    // Returns the base position of the addable link.
    // Returns (unsigned int)(-1) when the position of the iterator is
    // at the end.
    unsigned int Base();

    // Return a pointer to the unit at the current iterator position.
    // (returns NULL if the iterator is at the end).
    CCCLUnit* Unit();

    // Return true if the given depth is allowed for the present position
    // of the iterator.
    bool AllowedDepth(unsigned int Depth);
    
    // return the object describing the allowed depths for the current link
    CCCLDepths& Depths() { return m_bDepth; }

    // Advance the iterator. Returns false if the iterator reached the end.
    bool operator++();
};

typedef CPtr<CCCLPrefixIncAddableIter> CpCCCLPrefixIncAddableIter;

#endif /* __CCLSET_H__ */
