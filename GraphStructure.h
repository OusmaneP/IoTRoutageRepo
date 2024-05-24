/*
 * GraphStructure.h
 *
 *  Created on: 12 oct. 2023
 *      Author: Garrik Brel
 */

#ifndef GRAPHSTRUCTURE_H_
#define GRAPHSTRUCTURE_H_

#include<iostream>
#include<list>
#include<map>
#include <vector>
#include <algorithm>
using namespace std;

class GraphStructure {
    map<int, bool> visitedNodes;
    map<int, list<int>> adjacencyList;
    list<list<int>> circuitList; //The list of circuits
    list<int> circuit; // one circuit



    public:
        GraphStructure();
        virtual ~GraphStructure();
        void addEdge(int node1, int node2);
        void addAllNodeEdges(int node1, list<int> edges);
        void DFS(int startNode);
        map<int, list<int>> getAdjancyList();
        list<list<int>> getCircuitList();


};

#endif /* GRAPHSTRUCTURE_H_ */
