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

//    visitedNodes[startNode] = true;
//
//    circuit.push_back(startNode);
//    map<int, list<int>> tempAdjencyList = adjacencyList;
//    for(int nextNode : tempAdjencyList[startNode]) {
//        //adjacencyList[startNode].remove(nextNode);
//        if (!visitedNodes[nextNode]) {
//            adjacencyList[startNode].remove(nextNode);
//            DFS(nextNode);
//        }
//        else {
//            if (circuit.size() != 0){
//                circuitList.push_back(circuit);
//            }
//            visitedNodes.clear();
//            circuit.clear();
//        }
//    }
}

map<int, list<int>> GraphStructure::getAdjancyList() {
    return this->adjacencyList;
}

void GraphStructure::addAllNodeEdges(int node1, list<int> edges) {
    adjacencyList.insert({node1, edges});
}

list<list<int>> GraphStructure::getCircuitList() {
    return this->cycleList;
}

list<list<int>> GraphStructure::getPathList(){
    return pathList;
}

void GraphStructure::findAndRemoveCycles(int startNode, int currentNode){
    stack.push_back(currentNode);
    visitedNodes[currentNode] = true;

    map<int, list<int>> tempAdjencyList = adjacencyList;

    for (auto it = tempAdjencyList[currentNode].begin(); it != tempAdjencyList[currentNode].end();) {
        int nextNode = *it;

        // Remove the edge before making the recursive call
        it = adjacencyList[currentNode].erase(it);

        auto yt = std::find(adjacencyList[currentNode].begin(), adjacencyList[currentNode].end(), nextNode);
        if (yt != adjacencyList[currentNode].end()) {
            // Remove the element at the iterator position
            adjacencyList[currentNode].erase(yt);
        }


        if (nextNode == startNode) {
            // Found a cycle
            stack.push_back(nextNode);
            cycleList.push_back(list<int>(stack.begin(), stack.end()));
            stack.pop_back();
        } else if (!visitedNodes[nextNode]) {

            findAndRemoveCycles(startNode, nextNode);
        }
    }

    // Backtrack
    stack.pop_back();
    visitedNodes[currentNode] = false;
}

void GraphStructure::myCircuits(int debut, int currentNode){

    if (!adjacencyList[currentNode].empty()) {

        // if stack is empty
        if (stack.empty()) {
            stack.push_back(debut);
        }

        // Get the first value
        int firstValue = adjacencyList[currentNode].front();
        stack.push_back(firstValue);

        // remove first value from List[current]
        auto yt = std::find(adjacencyList[currentNode].begin(), adjacencyList[currentNode].end(), firstValue);
        if (yt != adjacencyList[currentNode].end()) {
            // Remove the element at the iterator position
            adjacencyList[currentNode].erase(yt);
        }

        // Found a cycle or recursive call
        if(firstValue == debut){
            cycleList.push_back(list<int>(stack.begin(), stack.end()));
            stack.clear();
            myCircuits(debut, debut);
        }
        else{
            myCircuits(debut, firstValue);
        }
    }
    if (!stack.empty()) {
        cycleList.push_back(list<int>(stack.begin(), stack.end()));
        stack.clear();
    }

}

void GraphStructure::findAndRemovePaths(int currentNode){
    stack.push_back(currentNode);
    visitedNodes[currentNode] = true;

    map<int, list<int>> tempAdjencyList = adjacencyList;

    bool hasChildren = false;
    for (auto it = tempAdjencyList[currentNode].begin(); it != tempAdjencyList[currentNode].end();) {
        int nextNode = *it;
        if (!visitedNodes[nextNode]) {
            hasChildren = true;
            findAndRemovePaths(nextNode);
            // Iterator needs to be reset after recursive call
            it = tempAdjencyList[currentNode].begin();
        } else {
            ++it;
        }
    }

    if (!hasChildren) {
        // Found a path (no more children to visit)
        pathList.push_back(list<int>(stack.begin(), stack.end()));

        // Remove the path from the graph
        for (auto it = stack.begin(); it != stack.end(); ++it) {
            auto nextIt = std::next(it);
            if (nextIt != stack.end()) {
                adjacencyList[*it].remove(*nextIt);
            }
        }
    }

    // Backtrack
    stack.pop_back();
    visitedNodes[currentNode] = false;
}

void GraphStructure::traverseGraph() {
    cycleList.clear();
    pathList.clear();
    stack.clear();


    // Find and remove all cycles
    for (auto& entry : adjacencyList) {
        int startNode = entry.first;
        while(!adjacencyList[startNode].empty()){
            myCircuits(startNode, startNode);
        }
    }

}

list<list<int>> GraphStructure::buildQuintupletsFromCircuitsList(list<list<int>> circuitList){
    list<list<int>> quintupletsList;
    int slot=1; //To increase the number of slots
    if (circuitList.size() != 0) {
        for (list<int> circuit: circuitList) {
            int indexOfNode = 0; // This will help us to verify if in a circuit, a node has a next one
            list<int>::iterator itr = circuit.begin(); //create an iterator to point on the first element of the ilst.
            for (int node:circuit) {
                if (indexOfNode < circuit.size() - 1) { //If the node is not the last in the circuit
                    list<int> quintuplet;
                    ++itr;

                    // emission quintuplet
                    quintuplet.push_back(node);     //emitter node
                    quintuplet.push_back(*itr);     //receiver node
                    quintuplet.push_back(slot);     //slot number
                    quintuplet.push_back(0);        // 0 for emission
                    quintuplet.push_back(*itr);     // receiver channel
                    quintupletsList.push_back(quintuplet);


                    indexOfNode = indexOfNode+1;
                    slot = slot + 1;
                }
            }
        }
    }
    return quintupletsList;
}
