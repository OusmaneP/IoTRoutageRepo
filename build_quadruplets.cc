 list<list<int>> Server::buildQuadrupletsFromCircuitsList(list<list<int>> circuitList) {
        list<list<int>> quadrupletsList;
        int slot=0; //To increase the number of slots
        if (circuitList.size() != 0) {
            for (list<int> circuit: circuitList) {
                int indexOfNode = 0; // This will help us to verify if in a circuit, a node has a next one
                list<int>::iterator itr = circuit.begin(); //create an iterator to point on the first element of the ilst.
                for (int node:circuit) {
                    if (indexOfNode < circuit.size() - 1) { //If the node is not the last in the circuit
                        list<int> quadruplet;
                        ++itr;
                        quadruplet.push_back(node);  //emitter node
                        quadruplet.push_back(*itr); //receiver node
                        quadruplet.push_back(slot); //slot number
                        quadruplet.push_back(1); // 1 for transmission
                        indexOfNode = indexOfNode+1;
                        slot = slot + 1;
                        quadrupletsList.push_back(quadruplet);
                        EV << "quadruplet= (" <<node <<";" <<*itr <<";" <<slot <<";" << 1 << ")\n";
                    }
                }
            }
        }
        return quadrupletsList;
    }