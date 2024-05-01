/*
 * GraphStructure.cpp
 *
 *  Created on: 12 oct. 2023
 *      Author: Garrik Brel
 */

#include "GraphStructure.h"

GraphStructure::GraphStructure() {
    // TODO Auto-generated constructor stub

}

GraphStructure::~GraphStructure() {
    // TODO Auto-generated destructor stub
}

void GraphStructure::addEdge(int node1, int node2) {
    adjacencyList[node1].push_back(node2);
}

void GraphStructure::DFS(int startNode) {
    visitedNodes[startNode] = true;

    circuit.push_back(startNode);
    map<int, list<int>> tempAdjencyList = adjacencyList;
    for(int nextNode : tempAdjencyList[startNode]) {
        //adjacencyList[startNode].remove(nextNode);
        if (!visitedNodes[nextNode]) {
            adjacencyList[startNode].remove(nextNode);
            DFS(nextNode);
        }
        else {
            if (circuit.size() != 0){
                circuitList.push_back(circuit);
            }
            visitedNodes.clear();
            circuit.clear();
        }
    }
}

map<int, list<int>> GraphStructure::getAdjancyList() {
    return this->adjacencyList;
}

void GraphStructure::addAllNodeEdges(int node1, list<int> edges) {
    adjacencyList.insert({node1, edges});
}

list<list<int>> GraphStructure::getCircuitList() {
    return this->circuitList;
}

