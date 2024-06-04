/*
 * GraphStructure.h
 *
 *  Created on: 12 oct. 2023
 *      Author: Garrik Brel
 */

#ifndef GRAPHSTRUCTURE_H_
#define GRAPHSTRUCTURE_H_

#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <set>
#include <algorithm>
using namespace std;

class GraphStructure {
    map<int, bool> visitedNodes;
    map<int, list<int>> adjacencyList;
    list<list<int>> cycleList;  // The list of cycles
    list<list<int>> pathList;   // The list of remaining paths
    list<int> stack;            // Stack to track the current path/cycle

    public:
        GraphStructure();
        virtual ~GraphStructure();
        void addEdge(int node1, int node2);
        void addAllNodeEdges(int node1, list<int> edges);
        void DFS(int startNode);
        map<int, list<int>> getAdjancyList();
        list<list<int>> getCircuitList();
        list<list<int>> getPathList();

        void findAndRemoveCycles(int startNode, int currentNode);
        void findAndRemovePaths(int currentNode);
        void traverseGraph() ;
        void myCircuits(int debut, int firstValue);

        list<list<int>> buildQuintupletsFromCircuitsList(list<list<int>> circuitList);

};

#endif /* GRAPHSTRUCTURE_H_ */
