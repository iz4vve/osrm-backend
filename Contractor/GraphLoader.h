/*
    open source routing machine
    Copyright (C) Dennis Luxen, others 2010

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU AFFERO General Public License as published by
the Free Software Foundation; either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
or see http://www.gnu.org/licenses/agpl.txt.
*/

#ifndef CREATEGRAPH_H
#define GRAPHLOADER_H

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>

#include <google/dense_hash_map>

#ifdef _GLIBCXX_PARALLEL
#include <parallel/algorithm>
#else
#include <algorithm>
#endif

#include "../DataStructures/ImportEdge.h"
#include "../typedefs.h"

typedef google::dense_hash_map<NodeID, NodeID> ExternalNodeMap;

template<typename EdgeT>
inline NodeID readOSMRGraphFromStream(istream &in, vector<EdgeT>& edgeList, vector<NodeInfo> * int2ExtNodeMap) {
    NodeID n, source, target, id;
    EdgeID m;
    int dir, xcoord, ycoord;// direction (0 = open, 1 = forward, 2+ = open)
    ExternalNodeMap ext2IntNodeMap;
    ext2IntNodeMap.set_empty_key(UINT_MAX);
    in >> n;
    VERBOSE(cout << "Importing n = " << n << " nodes ..." << flush;)
    for (NodeID i=0; i<n;i++) {
        in >> id >> ycoord >> xcoord;
        int2ExtNodeMap->push_back(NodeInfo(xcoord, ycoord, id));
        ext2IntNodeMap.insert(make_pair(id, i));
    }
    in >> m;
    VERBOSE(cout << " and " << m << " edges ..." << flush;)

    edgeList.reserve(m);
    for (EdgeID i=0; i<m; i++) {
        EdgeWeight weight;
        int length;
        in >> source >> target >> length >> dir >> weight;

        assert(length > 0);
        assert(weight > 0);
        assert(0<=dir && dir<=2);

        bool forward = true;
        bool backward = true;
        if (dir == 1) backward = false;
        if (dir == 2) forward = false;

        if(length == 0)
        { cerr << "loaded null length edge" << endl; exit(1); }

//         translate the external NodeIDs to internal IDs
        ExternalNodeMap::iterator intNodeID = ext2IntNodeMap.find(source);
        if( intNodeID == ext2IntNodeMap.end())
        {
        	cerr << "after " << edgeList.size() << " edges" << endl;
        	cerr << "->" << source << "," << target << "," << length << "," << dir << "," << weight << endl;
        	cerr << "unresolved source NodeID: " << source << endl; exit(0);
        }
        source = intNodeID->second;
        intNodeID = ext2IntNodeMap.find(target);
        if(intNodeID == ext2IntNodeMap.end()) { cerr << "unresolved target NodeID : " << target << endl; exit(0); }
        target = intNodeID->second;

        if(source == UINT_MAX || target == UINT_MAX) { cerr << "nonexisting source or target" << endl; exit(0); }

        EdgeT inputEdge(source, target, weight, forward, backward);
        edgeList.push_back(inputEdge);
    }
    ext2IntNodeMap.clear();
    cout << "ok" << endl;
    return n;
}

#endif // CREATEGRAPH_H
