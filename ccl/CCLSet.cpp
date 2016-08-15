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

#include <algorithm>
#include "CCLSet.h"
#include "CCLUnit.h"
#include "yError.h"

using namespace std;

/////////////////////////////
// One Side of Node Object //
/////////////////////////////

CCCLNodeSide::CCCLNodeSide() : m_LastOutbound0((unsigned int)-1)
{
}

////////////////////////
// Single Node Object //
////////////////////////

CCCLNode::CCCLNode(unsigned int Pos) : m_Pos(Pos)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    for(unsigned int Side = LEFT ; Side <= RIGHT ; Side++) {
        // The longest extending paths from this node end at the
        // present position
        m_Side[Side].m_Paths.push_back(new CRefInt(Pos));
        // No depth 0 links yet
        m_Side[Side].m_LastOutbound0 = Pos;
        // Objects for blocking
        m_Side[Side].m_pCompleteBlock = new CRefInt(-1);
    }
}

CCCLNode::~CCCLNode()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

bool
CCCLNode::HasInbound(unsigned int Side)
{
    return (m_Side[Side].m_Inbound.size() > 0);
}

SLinkPair
CCCLNode::GetInbound(unsigned int Side)
{
    if(!m_Side[Side].m_Inbound.size())
        return SLinkPair(m_Pos, (unsigned int)(-1));

    return m_Side[Side].m_Inbound.front();
}

SLinkPair
CCCLNode::GetLastOutbound(unsigned int Side)
{
    if(!m_Side[Side].m_Outbound.size())
        return SLinkPair(m_Pos, (unsigned int)(-1));

    return m_Side[Side].m_Outbound.back();
}

// return the beginning of the iterator on the outbound links
vector<SLinkPair>::iterator
CCCLNode::OutboundBegin(unsigned int Side)
{
    return m_Side[Side].m_Outbound.begin();
}

// return the end of the iterator on the outbound links
vector<SLinkPair>::iterator
CCCLNode::OutboundEnd(unsigned int Side)
{
    return m_Side[Side].m_Outbound.end();
}

// return the beginning of the reverse iterator on the outbound links
vector<SLinkPair>::reverse_iterator
CCCLNode::OutboundRBegin(unsigned int Side)
{
    return m_Side[Side].m_Outbound.rbegin();
}

// return the end of the reverse iterator on the outbound links
vector<SLinkPair>::reverse_iterator
CCCLNode::OutboundREnd(unsigned int Side)
{
    return m_Side[Side].m_Outbound.rend();
}

unsigned int
CCCLNode::GetLongestPath(unsigned int Side)
{
    return (int&)(*(m_Side[Side].m_Paths.back()));
}

unsigned int
CCCLNode::GetLongestPath(unsigned int Side, unsigned int FirstDepth)
{
    if(FirstDepth >= m_Side[Side].m_Paths.size())
        return GetPos();
    
    return (int&)(*(m_Side[Side].m_Paths[FirstDepth]));
}

bool
CCCLNode::HasPathTo(unsigned int Pos)
{
    if(Pos == m_Pos)
        return true;

    unsigned int Side = (Pos < m_Pos) ? LEFT : RIGHT;

    if(Side == LEFT)
        return((int&)(*(m_Side[Side].m_Paths.back())) <= Pos);
    else
        return((int&)(*(m_Side[Side].m_Paths.back())) >= Pos);
}

int
CCCLNode::GetPathFirstDepth(unsigned int Head)
{
    if(Head == m_Pos)
        return -1;

    unsigned int Side = (Head < m_Pos) ? LEFT : RIGHT;
    unsigned int d = 0;
    
    for(vector<CpCRefInt>::iterator Iter = m_Side[Side].m_Paths.begin() ;
        Iter != m_Side[Side].m_Paths.end() ; Iter++) {
        if(Side == LEFT) {
            if((int &)(*(*Iter)) <= Head)
                return d;
        } else {
            if((int &)(*(*Iter)) >= Head)
                return d;
        }
        d++;
    }

    return -1; // no path to 'Head'
}

unsigned int
CCCLNode::Blocking(int Pos)
{
    unsigned int Side = (Pos < (int)m_Pos) ? LEFT : RIGHT;  
    unsigned int Blocking = 0;

    if(m_Side[Side].m_Inbound.size()) {
        if(Side == LEFT) {
            if((int)(GetInbound(Side).m_End) > Pos)
                Blocking = 1;
        } else {
            if((int)(GetInbound(Side).m_End) < Pos)
                Blocking = 1;
        }

        if((int&)(*(m_Side[Side].m_pCompleteBlock)) >= 0) {
            if(Side == LEFT) {
                if((int&)(*(m_Side[Side].m_pCompleteBlock)) > Pos)
                    Blocking = (unsigned int)(-1);
            } else {
                if((int&)(*(m_Side[Side].m_pCompleteBlock)) < Pos)
                    Blocking = (unsigned int)(-1);
            }
        }
    }

    return Blocking;
}

SLinkPair
CCCLNode::UnusedAdj(unsigned int Side)
{
    SLinkPair Unused;

    Unused.m_End = (int&)(*(m_Side[Side].m_Paths.back())) +
        ((Side == LEFT) ? -1 : 1); 

    unsigned int Depth = m_Side[Side].m_Outbound.size() ?
        m_Side[Side].m_Outbound.back().m_Depth : 0;

    Unused.m_Depth = max(Depth, Blocking(Unused.m_End));

    return Unused;
}

void
CCCLNode::SetCompleteRightBlocking(CCCLSet* pSet, unsigned int Pos)
{
    if(!pSet) {
        yPError(ERR_MISSING, "CCL set pointer missing");
    }
        
    if((int&)*(m_Side[RIGHT].m_pCompleteBlock) >= 0)
        return; // value already set

    if(Pos != GetPos()) { // a node cannot be its own blocking position
        (int&)*(m_Side[RIGHT].m_pCompleteBlock) = (int)Pos;
    }

    // loop over all nodes reachable from this node
    // Actually, it is probably enough to do this only for the longest
    // outbound link because all other nodes do not have an adjacency
    // anymore. Since the algorithm remains linear either way, I leave it
    // as it is.
    for(vector<SLinkPair>::iterator Iter = m_Side[LEFT].m_Outbound.begin() ;
        Iter != m_Side[LEFT].m_Outbound.end() ; Iter++) {
        pSet->m_Nodes[(*Iter).m_End]->SetCompleteRightBlocking(pSet, Pos);
    }
}

void
CCCLNode::ReplaceMaxRightPathPtr(CCCLSet* pSet, CRefInt* pPos)
{
    if(!pSet) {
        yPError(ERR_MISSING, "CCL set pointer missing");
    }

    m_Side[RIGHT].m_Paths.back() = pPos;
    
    if(m_Side[RIGHT].m_Outbound.size())
        pSet->m_Nodes[GetLastOutbound(RIGHT).m_End]->
            ReplaceMaxRightPathPtr(pSet, pPos);
}

// Add a link whose base is at this node, which is the last word in the
// utterance (head and depth of the new link are given)

bool
CCCLNode::AddLinkAtLastWordBase(CCCLSet* pSet, CCCLNode* pHead,
                                unsigned int Depth)
{
    if(!pHead || Depth > 1) {
        yPError(ERR_MISSING, "no head pointer or invalid depth");
    }
    
    // append the head and depth to the list of outbound links
    m_Side[LEFT].m_Outbound.push_back(SLinkPair(pHead->GetPos(), Depth));
    if(!Depth)
        m_Side[LEFT].m_LastOutbound0 = pHead->GetPos();

    // update the path information

    if(m_Side[LEFT].m_Paths.size() <= Depth)
        m_Side[LEFT].m_Paths.resize(Depth+1);
    m_Side[LEFT].m_Paths[Depth] = pHead->m_Side[LEFT].m_Paths.back();        

    // Update blocking implied by this link
    if(Depth == 1) {
        // the base position is the first complete block position for
        // all nodes reachable by a path from the base. This should
        // be recorded on all nodes which do not have an earlier set
        // complete blocking.
        SetCompleteRightBlocking(pSet, GetPos());
    }
    
    return true;
}

// Add a link whose head is at this node, which is in the utterance prefix
// (not the last word).

bool
CCCLNode::AddLinkAtPrefixHead(CCCLSet* pSet, CCCLNode* pBase,
                              unsigned int Depth)
{
    if(!pBase || Depth > 1) {
        yPError(ERR_MISSING, "no head pointer or invalid depth");
    }
    
    // append the head and depth to the list of inbound links
    // (the list should be empty).

    if(m_Side[RIGHT].m_Inbound.size()) {
        yPError(ERR_SHOULDNT, "Attempting to add multiple inbound links");
    }
    
    m_Side[RIGHT].m_Inbound.push_back(SLinkPair(pBase->GetPos(), Depth));

    return true;
}

// Add a link whose base is at this node, which is in the prefix of the
// utterance (head and depth of the new link are given)

bool
CCCLNode::AddLinkAtPrefixBase(CCCLSet* pSet, CCCLNode* pHead,
                              unsigned int Depth)
{
    if(!pHead || Depth > 1) {
        yPError(ERR_MISSING, "no head pointer or invalid depth");
    }

    // Need to replace the path position pointer for the previous path
    // leaving this node (because the path pointer will be updated).
    if(m_Side[RIGHT].m_Outbound.size())
        pSet->m_Nodes[GetLastOutbound(RIGHT).m_End]->
            ReplaceMaxRightPathPtr(pSet,
                                   new CRefInt((int)GetLongestPath(RIGHT)));
    
    // append the head and depth to the list of outbound links
    m_Side[RIGHT].m_Outbound.push_back(SLinkPair(pHead->GetPos(), Depth));
    if(!Depth)
        m_Side[RIGHT].m_LastOutbound0 = pHead->GetPos();

    // update path information
    
    if(m_Side[RIGHT].m_Paths.size() <= Depth) {
        // First link with depth 1 on this side
        m_Side[RIGHT].m_Paths.resize(Depth+1);
        // The depth 0 path position is the one pointed at by other nodes,
        // so it must be preserved as the object for the longest path.
        m_Side[RIGHT].m_Paths[Depth] = m_Side[RIGHT].m_Paths[0];
        // Create a new object for depth 0 paths.
        m_Side[RIGHT].m_Paths[0] = 
            new CRefInt((int&)*(m_Side[RIGHT].m_Paths[Depth]));
    }
    // Set the new position as the longest path
    (int&)*(m_Side[RIGHT].m_Paths[Depth]) = pHead->GetPos();

    // Update blocking implied by this link
    if(Depth == 1) {
        // the base position (this node) is the first complete block
        // position for all nodes reachable by a path from the base.
        // This should be recorded on all nodes which do not have
        // an earlier set complete blocking.
        SetCompleteRightBlocking(pSet, GetPos());
    }
    
    return true;
}

// Add a link whose head is at this node, which is the last word in the
// utterance.

bool
CCCLNode::AddLinkAtLastWordHead(CCCLNode* pBase, unsigned int Depth)
{
    if(!pBase || Depth > 1) {
        yPError(ERR_MISSING, "no head pointer or invalid depth");
    }
    
    // append the head and depth to the list of inbound links
    // (the list should be empty).

    if(m_Side[LEFT].m_Inbound.size()) {
        yPError(ERR_SHOULDNT, "Attempting to add multiple inbound links");
    }
    
    m_Side[LEFT].m_Inbound.push_back(SLinkPair(pBase->GetPos(), Depth));

    // the longest path entry of the base should be used.
    m_Side[RIGHT].m_Paths[0] = pBase->m_Side[RIGHT].m_Paths[Depth];
    (int&)*(m_Side[RIGHT].m_Paths[0]) = m_Pos;
    
    // Update blocking implied by this link
    if(Depth == 1)
        // the base position is the complete blocking position
        (int&)*(m_Side[LEFT].m_pCompleteBlock) = pBase->GetPos();
    else
        (int&)*(m_Side[LEFT].m_pCompleteBlock) =
            (int&)*(pBase->m_Side[LEFT].m_pCompleteBlock);

    return true;
}

//////////////////////////////////
// Iterator over Outbound Links //
//////////////////////////////////

CCCLOutboundIter::CCCLOutboundIter(CCCLSet* pSet, unsigned int Node,
                                   unsigned int Side)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif

    if(!pSet || pSet->LastNode() < (int)Node) {
        yPError(ERR_MISSING, "CCL set pointer missing or node out of range");
    }

    if(Side >= SIDE_NUM) {
        yPError(ERR_OUT_OF_RANGE, "Side not left/right");
    }
    
    m_pNode = pSet->m_Nodes[Node];
    m_Side = Side;
    m_Iter = m_pNode->OutboundBegin(Side);
}

CCCLOutboundIter::~CCCLOutboundIter()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

// advance the iterator - return false if advance beyond the end
bool
CCCLOutboundIter::operator++()
{
    if(!m_pNode || m_Iter == m_pNode->OutboundEnd(m_Side))
        return false;
    
    ++m_Iter;
    return (m_Iter != m_pNode->OutboundEnd(m_Side));
}

CCCLOutboundIter::operator bool()
{
    return (m_pNode && m_Iter != m_pNode->OutboundEnd(m_Side));
}

CCCLOutboundIter::operator SLinkPair()
{
    if(!m_pNode || m_Iter == m_pNode->OutboundEnd(m_Side))
        return SLinkPair(-1, (unsigned int)(-1));

    return *m_Iter;
}

unsigned int
CCCLOutboundIter::Head()
{
    if(!m_pNode || m_Iter == m_pNode->OutboundEnd(m_Side))
        return (unsigned int)(-1);

    return (*m_Iter).m_End;
}

unsigned int
CCCLOutboundIter::Depth()
{
    if(!m_pNode || m_Iter == m_pNode->OutboundEnd(m_Side))
        return (unsigned int)(-1);

    return (*m_Iter).m_Depth;
}

//////////////////////////////////
// Common Cover Link Set Object //
//////////////////////////////////

CCCLSet::CCCLSet(CTracing* pTracing) :
        m_RVLeftPos(-1), m_RVLeftDepth((unsigned int)(-1)), m_bClosed(true),
        m_pTracing(pTracing)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif
    m_pUnits = new CCCLUnitVector();
}

CCCLSet::~CCCLSet()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CCCLSet::ClearSet()
{
    m_Nodes.clear();
    if(m_pUnits)
        m_pUnits->clear();

    m_RVLeftPos = -1;
    m_RVLeftDepth = (unsigned int)(-1);

    m_PrefixAdj.clear();

    m_bClosed = true; // cannot add links to an empty set
}

int
CCCLSet::LastNode()
{
    return m_Nodes.size() - 1;
}

CCCLUnit*
CCCLSet::GetUnit(unsigned int Pos)
{
    if(!m_pUnits || m_pUnits->size() <= Pos)
        return NULL;

    return (*m_pUnits)[Pos];
}

CCCLUnit*
CCCLSet::GetLastUnit()
{
    if(!m_pUnits || !m_pUnits->size())
        return NULL;

    return (*m_pUnits).back();
}

bool
CCCLSet::UnusedAdjacency(unsigned int Base, unsigned int Head,
                         unsigned int Depth)
{
    if(Head == Base || Depth > 1)
        return false;

    if((int)Head > LastNode() || (int)Base > LastNode())
        return false;

    SLinkPair Unused(m_Nodes[Base]->UnusedAdj((Head < Base) ? LEFT : RIGHT));
    
    return (Unused.m_End == Head && Unused.m_Depth <= Depth);
}

bool
CCCLSet::Equality(unsigned int Base, unsigned int Head, unsigned int Depth)
{
    if(Head == Base)
        return false;

    if((int)Head > LastNode() || (int)Base > LastNode())
        return false;

    // First, need to check the depth of the first link on the path from
    // the head to the base (if such a path exists)
    int OpPathDepth = m_Nodes[Head]->GetPathFirstDepth(Base);

    if(OpPathDepth >= 0 && OpPathDepth != Depth)
        return false;

    // Next, check whether there is an incoming link to the base from the
    // side of the head from a node beyond the head and such that there is
    // a link from the head to that node.

    unsigned int Side = (Head < Base) ? LEFT : RIGHT;
    
    // get incoming link (there is at most one)
    if(m_Nodes[Base]->HasInbound(Side)) {
        SLinkPair Inbound(m_Nodes[Base]->GetInbound(Side));

        if(Side == LEFT) {
            if(Inbound.m_End <= Head &&
               m_Nodes[Head]->HasPathTo(Inbound.m_End))
                return (Inbound.m_Depth == Depth);
        } else {
            if(Inbound.m_End >= Head &&
               m_Nodes[Head]->HasPathTo(Inbound.m_End))
                return (Inbound.m_Depth == Depth);
        }
    }

    return true;
}

bool
CCCLSet::NotCoveredToXk(unsigned int Base)
{
    // Check whether there is a link to or from the last word which
    // begins to the left of 'Base'.

    if(!m_Nodes.size())
        return false;
    
    // check the inbound link
    if(m_Nodes.back()->GetInbound(LEFT).m_End < Base)
        return false;

    // check the outbound links
    if(m_Nodes.back()->GetLastOutbound(LEFT).m_End < Base)
        return false;

    return true;
}

bool
CCCLSet::Forcing(unsigned int Base, unsigned int Head, unsigned int Depth)
{
    unsigned int Side = (Head < Base) ? LEFT : RIGHT;
    
    // check whether the inbound link results in forcing
    if(!m_Nodes[Base]->HasInbound(Side))
        return true;

    SLinkPair Inbound = m_Nodes[Base]->GetInbound(Side);
    
    if(Side == LEFT)
        return (Inbound.m_End > Head || Inbound.m_Depth < 1 || Depth == 1);
    else
        return (Inbound.m_End < Head || Inbound.m_Depth < 1 || Depth == 1);
}

bool
CCCLSet::HasRV()
{
    return (m_RVLeftPos >= 0);
}

int
CCCLSet::MinRVLeftPos()
{
    return m_RVLeftPos;
}

unsigned int
CCCLSet::LeftRVDepth()
{
    return m_RVLeftDepth;
}

unsigned int
CCCLSet::RightRVDepth()
{
    if(!HasRV())
        return (unsigned int)(-1);

    return m_Nodes.back()->GetLastOutbound(LEFT).m_Depth;
}

void
CCCLSet::UpdateRV(unsigned int Base, unsigned int Head, unsigned int Depth)
{
    if(Base < Head) {
        // A link from the prefix to the last word cannot create a
        // violation, so we only need to check repair of a violation.
        if(!HasRV())
            return;

        // If there was a resolution violation and
        // the link could be added, then it must have repaired all
        // resolution violations.
        m_RVLeftPos = -1;
        m_RVLeftDepth = (unsigned int)(-1);
        return;
    }

    // A link from the last word to the prefix

    // Check whether a violation was repaired by this link
    unsigned int Longest = m_Nodes[Head]->GetLongestPath(LEFT);

    if(Longest <= m_RVLeftPos) {
        // Repaired the violation (if there was any)
        m_RVLeftPos = -1;
        m_RVLeftDepth = (unsigned int)(-1);
    }

    // Check whether a minimal violation was created by this link
    SLinkPair Violation(m_Nodes[Longest]->GetInbound(LEFT));
    
    if(Violation.m_End < Longest &&
       m_Nodes[Violation.m_End]->GetPathFirstDepth(LastNode()) < 0
       && // This is a violation!
       (Violation.m_End > m_RVLeftPos ||
        (Violation.m_End == m_RVLeftPos &&
         Violation.m_Depth < m_RVLeftDepth))) {
        // This is a minimal violation
        m_RVLeftPos = Violation.m_End;
        m_RVLeftDepth = Violation.m_Depth;
    }

    return;
}

void
CCCLSet::UpdatePrefixAdj()
{
    if(!m_Nodes.size()) { // an empty utterance has no adjacencies
        m_PrefixAdj.clear();
        return;
    }
    // If there is no link from the prefix to the next word, remove
    // all adjacencies from the list.
    if(!m_Nodes.back()->HasInbound(LEFT)) {
        m_PrefixAdj.clear();
    } else {
        // remove all nodes which follow the word attached to
        // the last word.
        unsigned int Connected = m_Nodes.back()->GetInbound(LEFT).m_End;

        while(m_PrefixAdj.size() && m_PrefixAdj.front().m_End > Connected)
            m_PrefixAdj.pop_front();

        // for the remaining nodes, check whether their adjacency properties
        // have changed.
        list<SLinkPair>::iterator Iter = m_PrefixAdj.begin();
        while(Iter != m_PrefixAdj.end()) {
            unsigned int NewDepth =
                m_Nodes[(*Iter).m_End]->UnusedAdj(RIGHT).m_Depth;
            if((*Iter).m_Depth == NewDepth) {
                // if this node is not attached to the last node, there
                // is no need to go further (by a lemma).
                if((*Iter).m_End != Connected)
                    break;
                Iter++;
            } else if(NewDepth > 1)
                Iter = m_PrefixAdj.erase(Iter);
            else {
                (*Iter).m_Depth = NewDepth;
                Iter++;
            }
        }
    }

    // The last word is always adjacent with depth 0
    m_PrefixAdj.push_front(SLinkPair(LastNode(),0));
}

bool
CCCLSet::CanIncNodeNum()
{
    return !HasRV(); // cannot increment before violations are repaired
}

bool
CCCLSet::IncNodeNum(CCCLUnit* pNextUnit)
{
    if(!CanIncNodeNum())
        return false; // cannot increment before violations are repaired

    // Update list of adjacencies
    UpdatePrefixAdj();
    
    // Append a new node
    m_Nodes.push_back(new CCCLNode(m_Nodes.size()));

    // store the given unit
    m_pUnits->push_back(pNextUnit);

    if(pNextUnit)
        pNextUnit->SetNode(m_Nodes.back());

    m_bClosed = false;
    
    return true;
}

bool
CCCLSet::CloseSet()
{
    if(!CanIncNodeNum())
        return false;
    
    m_bClosed = true;
    return true;
}

bool
CCCLSet::AddLink(unsigned int Base, unsigned int Head, unsigned int Depth)
{
    if(m_bClosed) {
        yPError(ERR_SHOULDNT, "adding a link to a closed set");
        return false;
    }
    
    if(Base == LastNode()) {
        if(Head >= LastNode()) {
            yPError(ERR_SHOULDNT,"head not in prefix");
            return false;
        }
        
        // attach a link from the last node to the prefix

        // Check for resolution violation
        if(HasRV() && (Head < MinRVLeftPos() || Depth != RightRVDepth())) {
            TraceSet();
            yPError(ERR_SHOULDNT,"not allowed while resolution violation");
            return false;
        }
        
        // Check that the link is addable
        if(!UnusedAdjacency(Base, Head, Depth) ||
           !Equality(Base, Head, Depth) || !Forcing(Base, Head, Depth)) {
            TraceSet();
            yPError(ERR_SHOULDNT,"adjacency, equality or forcing violation");
            return false;
        }

    } else if(Head == LastNode()) {
        if(Base >= LastNode()) {
            yPError(ERR_SHOULDNT,"base not in prefix");
            return false;
        }
        
        // attach a link from the prefix to the last node

        // Check for resolution violation
        if(HasRV() && (Base < MinRVLeftPos() ||
                       (Base == MinRVLeftPos() && Depth != LeftRVDepth()))) {
            TraceSet();
            yPError(ERR_SHOULDNT,"not allowed while resolution violation");
            return false; // not allowed
        }
        
        // Check that the link is addable. There is no need to check forcing
        // because the only possible violation of forcing is also a
        // violation of equality.
        if(!UnusedAdjacency(Base, Head, Depth) ||
           !NotCoveredToXk(Base) || !Equality(Base, Head, Depth)) {
            TraceSet();
            yPError(ERR_SHOULDNT,"adjacency, covered or equality violation");
            return false;
        }

    } else {
        yPError(ERR_SHOULDNT, "neither head nor base at last node");
        return false;
    }
    
    // Add the link
    if(Base < Head) {
        m_Nodes[Base]->AddLinkAtPrefixBase(this, m_Nodes[Head], Depth);
        m_Nodes[Head]->AddLinkAtLastWordHead(m_Nodes[Base], Depth);
    } else {
        m_Nodes[Base]->AddLinkAtLastWordBase(this, m_Nodes[Head], Depth);
        m_Nodes[Head]->AddLinkAtPrefixHead(this, m_Nodes[Base], Depth);
    }

    UpdateRV(Base, Head, Depth);
    return true;
}

SLinkPair
CCCLSet::GetInbound(unsigned int Node, unsigned int Side)
{
    if(Side > SIDE_NUM) {
        yPError(ERR_OUT_OF_RANGE, "unknown side value");
    }

    if(Node > LastNode()) {
        return SLinkPair(Node, (unsigned int)(-1));
    }

    return m_Nodes[Node]->GetInbound(Side);
}

unsigned int
CCCLSet::GetLastOutbound0(unsigned int Node, unsigned int Side)
{
    if(Side > SIDE_NUM) {
        yPError(ERR_OUT_OF_RANGE, "unknown side value");
    }

    if(Node > LastNode())
        return (unsigned int)(-1);

    return m_Nodes[Node]->GetLastOutbound0(Side);
}

SLinkPair
CCCLSet::GetLastOutbound(unsigned int Node, unsigned int Side)
{
    if(Side > SIDE_NUM) {
        yPError(ERR_OUT_OF_RANGE, "unknown side value");
    }

    if(Node > LastNode()) {
        return SLinkPair(Node, (unsigned int)(-1));
    }

    return m_Nodes[Node]->GetLastOutbound(Side);
}

unsigned int
CCCLSet::GetOutboundNum(unsigned int Node, unsigned int Side)
{
    if(Side > SIDE_NUM) {
        yPError(ERR_OUT_OF_RANGE, "unknown side value");
    }

    if(Node > LastNode()) {
        return 0;
    }

    return m_Nodes[Node]->GetOutboundNum(Side);
}

CCCLOutboundIter*
CCCLSet::GetOutboundIter(unsigned int Node, unsigned int Side)
{
    if(Side > SIDE_NUM) {
        yPError(ERR_OUT_OF_RANGE, "unknown side value");
    }

    if(Node > LastNode()) {
        yPError(ERR_OUT_OF_RANGE, "node beyond end of utterance");
    }

    return new CCCLOutboundIter(this, Node, Side);
}

unsigned int
CCCLSet::GetLongestPath(unsigned int Node, unsigned int Side)
{
    if(Node >= m_Nodes.size())
        return Node;

    return m_Nodes[Node]->GetLongestPath(Side);
}

unsigned int
CCCLSet::GetLongestPath(unsigned int Node, unsigned int Side,
                        unsigned int FirstDepth)
{
    if(Node >= m_Nodes.size())
        return Node;

    return m_Nodes[Node]->GetLongestPath(Side, FirstDepth);
}

SLinkPair
CCCLSet::UnusedAdj(unsigned int Node, unsigned int Side)
{
    if(Node >= m_Nodes.size())
        return SLinkPair(-1, (unsigned int)(-1));

    return m_Nodes[Node]->UnusedAdj(Side);
}

int
CCCLSet::LastAddableLink(CCCLDepths& bDepths)
{
    if(m_Nodes.empty()) {
        bDepths[0] = bDepths[1] = false;
        return -1;
    }
    
    SLinkPair Unused(m_Nodes.back()->UnusedAdj(LEFT));

    bDepths[0] = (Unused.m_Depth <= 0 && Unused.m_End >= 0);
    bDepths[1] = (Unused.m_Depth <= 1 && Unused.m_End >= 0);
    
    if(!bDepths.Allowed())
        return Unused.m_End;

    // still need to check whether equality and forcing imply any additional
    // restrictions on the depth.

    SLinkPair Inbound(m_Nodes.back()->GetInbound(LEFT));
    
    // forcing (iff there is an inbound link of depth 1)
    
    if(Inbound.m_Depth == 1)
        bDepths[0] = false;

    // equality
    
    // First, need to check the depth of the first link on the path from
    // the head to the base (if such a path exists)
    int OpPathDepth = m_Nodes[Unused.m_End]->GetPathFirstDepth(LastNode());
    if(OpPathDepth >= 0)
        bDepths[1 - OpPathDepth] = false;
    
    // Next, check whether there is an opposite link such that adding the
    // addable link will create a path from the head of that link to the
    // base of that link.
    if(Inbound.m_Depth <= 1 &&
       m_Nodes[Unused.m_End]->HasPathTo(Inbound.m_End))
        bDepths[1-Inbound.m_Depth] = false;

    // Check whether a resolution violation adds additional restrictions
    if(HasRV())
        bDepths[1 - RightRVDepth()] = false;
    
    return Unused.m_End;
}

//
// Debugging
//

void
CCCLSet::TraceSet()
{
    if(!m_pTracing || !m_pTracing->IsTraceOn(TB_CCL_SET))
        return;

    // for each node, output its outbound links
    for(vector<CpCCCLNode>::iterator Iter = m_Nodes.begin() ;
        Iter != m_Nodes.end() ; Iter++) {

        // print the node position and the unit name
        (*m_pTracing)(TB_CCL_SET) << (*Iter)->GetPos() << " ("
                                  << (char const*)(*GetUnit((*Iter)->GetPos()))
                                  << "):";
        
        // reverse loop over the left outbound links 
        for(vector<SLinkPair>::reverse_iterator LIter =
                (*Iter)->OutboundRBegin(LEFT) ;
            LIter != (*Iter)->OutboundREnd(LEFT) ; LIter++) {
            (*m_pTracing)(TB_CCL_SET) << " <" << (*LIter).m_End
                                      << "(" << (*LIter).m_Depth << ")";
        }

        // forward loop over the right outbound links 
        for(vector<SLinkPair>::iterator LIter =
                (*Iter)->OutboundBegin(RIGHT) ;
            LIter != (*Iter)->OutboundEnd(RIGHT) ; LIter++) {
            (*m_pTracing)(TB_CCL_SET) << " (" << (*LIter).m_Depth << ")"
                                      << (*LIter).m_End << ">";
        }
        
        (*m_pTracing)(TB_CCL_SET) << Endl;
    }
}

////////////////////////////////////////////////
// Prefix Incrementally Addable Link Iterator //
////////////////////////////////////////////////

CCCLPrefixIncAddableIter::CCCLPrefixIncAddableIter(CCCLSet* pSet) :
        m_pSet(pSet)
{
#ifdef DETAILED_DEBUG
    IncObjCount();
#endif   

    if(!pSet) {
        yPError(ERR_MISSING, "Iterator initialized with NULL set");
    }

    m_Iter = m_Begin = m_pSet->m_PrefixAdj.begin();
    
    m_bEmpty = (m_pSet->LastNode() < 0 ||
                !m_pSet->m_Nodes.back() ||
                m_pSet->m_Nodes.back()->HasInbound(LEFT));

    if(m_bEmpty)
        return;
    
    // skip any position which is covered by an opposite link
    unsigned int Covered = m_pSet->m_Nodes.back()->GetLastOutbound(LEFT).m_End;

    for( ; m_Begin != m_pSet->m_PrefixAdj.end() && (*m_Begin).m_End > Covered ;
         m_Begin++);
    
    m_Iter = m_Begin;

    CalcDepths();
    m_bEmpty = (m_Iter == m_pSet->m_PrefixAdj.end());
}

CCCLPrefixIncAddableIter::~CCCLPrefixIncAddableIter()
{
#ifdef DETAILED_DEBUG
    DecObjCount();
#endif
}

void
CCCLPrefixIncAddableIter::CalcDepths()
{
    m_bDepth[0] = m_bDepth[1] = false;
    
    while(1) {
        if(End())
            return;
    
        // Initialize the depth to those specified by the adjacency
        m_bDepth[0] = ((*m_Iter).m_Depth == 0);
        m_bDepth[1] = true;
    
        // Check equality restrictions. Need only check the depth of
        // the first link in a path from the last word to the current
        // iterator position.
        int OpPathDepth =
            m_pSet->m_Nodes.back()->GetPathFirstDepth((*m_Iter).m_End);

        if(OpPathDepth >= 0)
            m_bDepth[1 - OpPathDepth] = false;
        
        // Check resolution violation restrictions
        if(m_pSet->HasRV() && (*m_Iter).m_End == m_pSet->MinRVLeftPos())
            m_bDepth[1 - m_pSet->LeftRVDepth()] = false;
        
        // No need to check forcing (for links from the prefix forcing
        // reduces to equality).

        if(m_bDepth[0] || m_bDepth[1])
            return;

        m_Iter++;
    }
}

bool
CCCLPrefixIncAddableIter::End()
{
    return (m_bEmpty || m_Iter == m_pSet->m_PrefixAdj.end() ||
            (m_pSet->HasRV() && (*m_Iter).m_End < m_pSet->MinRVLeftPos()));
}

bool
CCCLPrefixIncAddableIter::Begin()
{
    return (m_Iter == m_Begin);
}

bool
CCCLPrefixIncAddableIter::Restart()
{
    m_Iter = m_Begin;
    CalcDepths();
    
    return !(m_bEmpty);
}

unsigned int
CCCLPrefixIncAddableIter::Base()
{
    if(End())
        return (unsigned int)(-1);
    
    return (*m_Iter).m_End;
}

CCCLUnit*
CCCLPrefixIncAddableIter::Unit()
{
    if(End())
        return NULL;
    
    return m_pSet->GetUnit((*m_Iter).m_End);
}

bool
CCCLPrefixIncAddableIter::AllowedDepth(unsigned int Depth)
{
    if(Depth > 1)
        return false;

    return m_bDepth[Depth];
}

bool
CCCLPrefixIncAddableIter::operator++()
{
    if(End())
        return false;
    
    ++m_Iter;
    CalcDepths();
    return !End();
}
