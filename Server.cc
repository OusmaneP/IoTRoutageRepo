
#include "Server.h"


namespace pocSimulation {

    Define_Module(Server);

    Server::Server()
        {
            endRxEvent = nullptr;
        }

        Server::~Server()
        {
            cancelAndDelete(endRxEvent);
        }

        void Server::initialize()
        {
            HostsIdList = {};
            txRate = par("txRate");
            iaTime = &par("iaTime");
            pkLenBits = &par("pkLenBits");
            numberOfHosts = par("numHosts");
            aoiWidth = par("SbWidth");
            aoiheight = par("SbHeight");
            sinkX = par("sinkX");
            sinkY = par("sinkY");

            numberOfMessagesForInitialization = 0;
            channelStateSignal = registerSignal("channelState");
            endRxEvent = new cMessage("end-reception");
            channelBusy = false;
            emit(channelStateSignal, IDLE);

            gate("in")->setDeliverOnReceptionStart(true);

            currentCollisionNumFrames = 0;
            receiveCounter = 0;
            WATCH(currentCollisionNumFrames);

            receiveBeginSignal = registerSignal("receiveBegin");
            receiveSignal = registerSignal("receive");
            collisionSignal = registerSignal("collision");
            collisionLengthSignal = registerSignal("collisionLength");

            emit(receiveSignal, 0L);
            emit(receiveBeginSignal, 0L);

            getDisplayString().setTagArg("p", 0, par("sinkX").doubleValue());
            getDisplayString().setTagArg("p", 1, par("sinkY").doubleValue());

            // my params
            sinkName = getName();
            char msgnameDiffHostIds[] = "Receiving Hosts Ids from Sinks";
            listHostMsg = new ListHostMsg(msgnameDiffHostIds);
            listHostMsg->setMsgContent(msgnameDiffHostIds);
            listHostMsg->setSource(getId());



            // schedule send My ID
            char msgname[] = "sending myID";
            char datas[] = "Ids";
            sprintf(msgname, "sending myID");
            newMessage = generateMessage(msgname,getId(),datas);
            scheduleAt(0, newMessage);

            // schedule diffusion of Host ids
            char msgnameDiff[] = "broadcasting hosts Ids";
            char* datasDiff = (char*)("Diff IDs");
            PocMsg *msgDiffIds = generateMessage(msgnameDiff, getId(), datasDiff);
            simtime_t delayDiff = 0.099170222132;
            scheduleAt(simTime()  + delayDiff, msgDiffIds);

            // schedule send External Ids to sink1
            mySchduleAt(0.099172155333, "WakeUp and Send External Ids to Sink 1");

            // schedule sink1 to compute graph, cycles, paths, and quintuplets
            if (strcmp("sink1", getName()) == 0) {
                mySchduleAt(0.099172155341, "WakeUp Sink 1, And compute graph...");
            }

            // schedule External Routing t=0.099172155341
            mySchduleAt(0.099172155644, "WakeUp four External Routing");

            // schedule each sink in cluster to compute graph 0.099172155696
            mySchduleAt(0.099172155755, "WakeUp each sink in cluster to compute graph");

            // schedule each node for internal routing. 0.099172155755
            mySchduleAt(0.099172155844, "WakeUp each node in cluster to route internal");


        }

        void Server::handleMessage(cMessage *msg)
        {

            // Check if msg is of type PocMsg
            if (dynamic_cast<PocMsg *>(msg) != nullptr) {

                PocMsg *ttmsg = check_and_cast<PocMsg *>(msg);
                if (strcmp("sending myID", ttmsg->getMsgContent()) == 0){
                    for(int i=0; i<numberOfHosts; i++) {
                        host = getParentModule()->getSubmodule("host", i);
                        double hostX = host->par("x").doubleValue();
                        double hostY = host->par("y").doubleValue();
                        char msgname[] = "Sending my LeaderID";
                        sprintf(msgname, "Sending my LeaderID");
                        simtime_t duration = pkLenBits->intValue() / txRate;
                        PocMsg *msgToSend;
                        duration = pkLenBits->intValue() / txRate;
                        if (strcmp("sink1", getName()) == 0) {
                            if((0<hostX && hostX<aoiWidth/2) && (0<hostY && hostY<aoiheight/2)) {
                                double dist = std::sqrt((sinkX-hostX) * (sinkX-hostX) + (sinkY-hostY) * (sinkY-hostY));
                                char* datas = (char*)("Sink ID");
                                radioDelay = dist / propagationSpeed;
                                msgToSend = generateMessage(msgname,host->getId(), datas);
                                sendDirect(msgToSend, radioDelay, duration, host->gate("in"));
                            }
                        }
                        if (strcmp("sink2", getName()) == 0) {
                            if((aoiWidth/2<hostX && hostX<aoiWidth) && (0<hostY && hostY<aoiheight/2)) {
                                double dist = std::sqrt((sinkX-hostX) * (sinkX-hostX) + (sinkY-hostY) * (sinkY-hostY));
                                char* datas = (char*)("Sink ID");
                                radioDelay = dist / propagationSpeed;
                                msgToSend = generateMessage(msgname,host->getId(), datas);
                                sendDirect(msgToSend, radioDelay, duration, host->gate("in"));
                            }
                        }
                        if (strcmp("sink3", getName()) == 0) {
                            if((0<hostX && hostX<aoiWidth/2) && (aoiheight/2<hostY && hostY<aoiheight)) {
                                double dist = std::sqrt((sinkX-hostX) * (sinkX-hostX) + (sinkY-hostY) * (sinkY-hostY));
                                char* datas = (char*)("Sink ID");
                                radioDelay = dist / propagationSpeed;
                                msgToSend = generateMessage(msgname,host->getId(), datas);
                                sendDirect(msgToSend, radioDelay, duration, host->gate("in"));
                            }
                        }
                        if (strcmp("sink4", getName()) == 0) {
                            if((aoiWidth/2<hostX && hostX<aoiWidth) && (aoiheight/2<hostY && hostY<aoiheight)) {
                                double dist = std::sqrt((sinkX-hostX) * (sinkX-hostX) + (sinkY-hostY) * (sinkY-hostY));
                                char* datas = (char*)("Sink ID");
                                radioDelay = dist / propagationSpeed;
                                msgToSend = generateMessage(msgname,host->getId(), datas);
                                sendDirect(msgToSend, radioDelay, duration, host->gate("in"));
                            }
                        }
                    }
                }
                else if (strcmp("Receiving ACK", ttmsg->getMsgContent()) == 0) {
                    // Store the hosts ids in a list
                    HostsIdList.push_back(ttmsg->getSource());
                }
                    // Diffusion Host Ids
                else if (strcmp("broadcasting hosts Ids", ttmsg->getMsgContent()) == 0) {

                    EV << endl;
                    EV << sinkName  << " is ready to send the hosts Ids of cluster to : " << endl;
                    // put ids in the Msg array
                    listHostMsg->setDatasArraySize(HostsIdList.size());
                    int indexArray = 0;
                    for (auto it = HostsIdList.begin(); it != HostsIdList.end(); ++it) {
                        listHostMsg->setDatas(indexArray, *it);
                        indexArray++;
                    }


                    // Get a pointer to the system module
                    cModule *systemModule = getSystemModule();
                    // Iterate over all submodules of the system module
                    cModule::SubmoduleIterator iter(systemModule);
                    while (!iter.end()) {
                        cModule *module = iter();

                        // Check if the module name starts with "sink"
                        const char *moduleName = module->getName();
                        if ((strncmp(moduleName, "sink", 4) == 0) && (strcmp(moduleName, getName()) != 0)) {
                            // Module name starts with "sink"
                            EV << "---------------------------------- " << moduleName << endl;

                            // Send Ids Msg
                            listHostMsg->setDestination(module->getId());
                            listHostMsg->setMsgContent(getName());
                            sendDirect(listHostMsg->dup(), module, "in");
                        }
                        else if (isIdInList(module->getId())){
                            listHostMsg->setDestination(module->getId());
                            listHostMsg->setMsgContent(getName());
                            sendDirect(listHostMsg->dup(), module, "in");
                        }

                        // Move to the next submodule
                        ++iter;
                    }

                    EV << endl;


                }
                else if (strcmp("WakeUp and Send External Ids to Sink 1", ttmsg->getMsgContent()) == 0) {
                    EV << "Waking Up and Sending External Ids to Sink 1" << endl;

                    cModule *target = getParentModule()->getSubmodule("sink1");

                    ListHostMsg *listHostMsg;
                    char msgSendExternalPackets[] = "Sending External Packets Ids to Sink1";
                    listHostMsg = new ListHostMsg(msgSendExternalPackets);
                    listHostMsg->setMsgContent(msgSendExternalPackets);
                    listHostMsg->setSource(getId());

                    listHostMsg->setDatasArraySize(listExternalPackets.size());
                    int indexArray = 0;
                    for (auto it = listExternalPackets.begin(); it != listExternalPackets.end(); ++it) {
                        listHostMsg->setDatas(indexArray, *it);
                        indexArray++;
                    }

                    listHostMsg->setDestination(target->getId());
                    sendDirect(listHostMsg->dup(), target, "in");
                }
                else if (strcmp("WakeUp Sink 1, And compute graph...", ttmsg->getMsgContent()) == 0) {
                    if (strcmp("sink1", getName()) == 0) {

                        EV << endl;
                        EV << endl;
                        EV << sinkName << " Wake up Compute graph *** *** *** *** " << endl;

                        char msgSendExternalPackets[] = "Sending back List Quintuplets from sink1";

                        computeGraphCircuitsPaths(msgSendExternalPackets, graphMultiCanalSink1);
                    }
                }
                // wake up to rout external packets
                else if (strcmp("WakeUp four External Routing", ttmsg->getMsgContent()) == 0) {
                    EV << getName() << " Woke up for external Routing " << endl;

                    if(!quintupletsListEmExternalP.empty()){
                        for (auto& quintuplet : quintupletsListEmExternalP) {
                            // schedule a wake up to rout an external packet
                            int sinkReceiverId = getValueAtListPosition(quintuplet, 1);
                            int slot = getValueAtListPosition(quintuplet, 2);

                            bool foundPacket = false;
                            int position = 0;

                            while (!foundPacket && position < listExternalPackets.size()){
                                int packetIdToSend = getValueAtListPosition(listExternalPackets, position);

                                if(HostsIdMapExternal[packetIdToSend] == sinkReceiverId){
                                    foundPacket = true;
                                    // remove packet from List[current]
                                    auto yt = std::find(listExternalPackets.begin(), listExternalPackets.end(), packetIdToSend);
                                    if (yt != listExternalPackets.end()) {
                                        // Remove the element at the iterator position
                                        listExternalPackets.erase(yt);
                                    }

                                    EV << getName() << " will send packet of " << packetIdToSend << " to " << sinkReceiverId << endl;
//                                    sendOnePacketToSink(packetIdToSend, sinkReceiverId);
                                    simtime_t routDelay = 0.000000000002 * slot;
                                    char message[] = "message route on packet to a sink";
                                    mySchduleRountAPacketAt(routDelay , message, packetIdToSend, sinkReceiverId);
                                }
                                position++;
                            }

                        }
                    }
                }
                // wake up and route one external packets
                else if (strcmp("message route on packet to a sink", ttmsg->getMsgContent()) == 0) {
                    sendOnePacketToSink(ttmsg->getDestination(), ttmsg->getSource()); //packetIdToSend  , sinkReceiverId
                }
                // WakeUp each sink in cluster to compute graph
                else if (strcmp("WakeUp each sink in cluster to compute graph", ttmsg->getMsgContent()) == 0) {
                    char msgSendExternalPackets[] = "Sending back List Quintuplets from my cluster sink";

                    computeGraphCircuitsPaths(msgSendExternalPackets, graph);
                }
                else if (strcmp("WakeUp each node in cluster to route internal", ttmsg->getMsgContent()) == 0) {
                    EV << getName() << " Woke up for internal Routing " << endl;

                    if(!quintupletsListEmission.empty()){
                        for (auto& quintuplet : quintupletsListEmission) {
                            // schedule a wake up to rout an external packet
                            int nodeReceiverId = getValueAtListPosition(quintuplet, 1);
                            int slot = getValueAtListPosition(quintuplet, 2);

                            EV << getName() << " will send a packet to " << nodeReceiverId << endl;
                            simtime_t routDelay = 0.000000000002 * slot;
                            char message[] = "message route one packet to a node";
                            mySchduleRountAPacketAt(routDelay , message, nodeReceiverId, nodeReceiverId);
                        }
                    }
                }
                else if (strcmp("message route one packet to a node", ttmsg->getMsgContent()) == 0) {
                    sendOnePacketToSink(ttmsg->getDestination(), ttmsg->getSource());
                }


//                if (strcmp("sendingId to server", ttmsg->getMsgContent()) == 0){
//                    if (numberOfMessagesForInitialization <= numberOfHosts) {
//                        HostsIdList.push_back(ttmsg->getSource());
//                        numberOfMessagesForInitialization = numberOfMessagesForInitialization+1;
//                        EV << "Received packet from " << ttmsg->getSource() << "\n";
//                    }
//                }
            }

            // Check if msg is of type ListHostMsg
            else if (dynamic_cast<ListHostMsg *>(msg) != nullptr) {
                ListHostMsg *rcvListHostMsg = check_and_cast<ListHostMsg *>(msg);

                // when sink receive all packets including external from his cluster hosts.
                if (strcmp("Send External Packets", rcvListHostMsg->getMsgContent()) == 0){
                    // Get the size of the datas[] array
                    int arraySize = rcvListHostMsg->getDatasArraySize();

                    // Read all values in the datas[] array and put them in listPackets
                    for (int i = 0; i < arraySize; ++i) {
                        int hostId = rcvListHostMsg->getDatas(i);

                        if(isIdInList(hostId)){
                            graph.addEdge(rcvListHostMsg->getSource(), hostId);
                        }else{
                            listExternalPackets.push_back(hostId);
                        }

                    }

                }
                    // when Receiving External Packets Ids to Sink1
                else if (strcmp("Sending External Packets Ids to Sink1", rcvListHostMsg->getMsgContent()) == 0){
                    int arraySize = rcvListHostMsg->getDatasArraySize();
                    EV << rcvListHostMsg->getSenderModule()->getName() << " Has these External packets :: ";

                    // Read all values in the datas[] array and put them in HostsIdMapExternal
                    for (int i = 0; i < arraySize; ++i) {
                        int hostId = rcvListHostMsg->getDatas(i);
                        EV << hostId << " -- ";

                        if(isIdInList(hostId)){
                            graphMultiCanalSink1.addEdge(rcvListHostMsg->getSource(), getId());
                        }else{
                            auto it = HostsIdMapExternal.find(hostId);

                            if(it != HostsIdMapExternal.end()){

                                int secondSinkId = it->second;

                                graphMultiCanalSink1.addEdge(rcvListHostMsg->getSource(), secondSinkId);

                            }

                        }

                    }


                    EV << endl;
                }
                // when receive one packet from external sink
                else if (strcmp("Sending 1 packet to destination sink", rcvListHostMsg->getMsgContent()) == 0) {
                    EV << getName() << " Id : " << getId() << " Received one packet from external sink with id : " << rcvListHostMsg->getSource() << endl;
                    int packetReceived = rcvListHostMsg->getDestination();
                    graph.addEdge(getId(), packetReceived);

                }
                // when a sink receives host ids of external nodes
                else{

                    EV << getName() << " : Received from : " << rcvListHostMsg->getMsgContent() << endl;

                    if (rcvListHostMsg) {
                        // Get the size of the datas[] array
                        int arraySize = rcvListHostMsg->getDatasArraySize();

                        // Read all values in the datas[] array and put them in HostsIdMapExternal
                        for (int i = 0; i < arraySize; ++i) {
                            int hostId = rcvListHostMsg->getDatas(i);
                            EV << hostId << " -- ";

                            cModule *module = getParentModule()->getSubmodule(rcvListHostMsg->getMsgContent());

                            HostsIdMapExternal[hostId] = module->getId();

                        }
                        EV << endl;
                    } else {
                        // Handle the case where the message is not of type ListHostMsg
                        EV << "Received message is not of type ListHostMsg" << endl;
                    }

                }
            }
            // Check if msg is of type List Quintuplets Msg
            else if (dynamic_cast<ListQuintMsg *>(msg) != nullptr) {
                ListQuintMsg *rcvListQuintMsg = check_and_cast<ListQuintMsg *>(msg);

                // if  received List Quintuplets from sink1
                if (strcmp("Sending back List Quintuplets from sink1", rcvListQuintMsg->getMsgContent()) == 0){
                    EV << endl;
                    EV << getName() << " received Quintuplets from SB : " << rcvListQuintMsg->getSource() << endl;

                    int arraySize = rcvListQuintMsg->getDatasArraySize();

                    // Read all values in the datas[] array
                    for (int i = 0; i < arraySize; ++i) {
                        carListFromSB.push_back(rcvListQuintMsg->getDatas(i));

                    }

                    // destructure the char List
                    if(!carListFromSB.empty()){
                        std::pair<list<list<int>>, list<list<int>>> pairQuintuplets = destructureListCharQuint(carListFromSB);

                        // Extract the results
                        quintupletsListEmExternalP = pairQuintuplets.first;
                        quintupletsListRecepExternalP = pairQuintuplets.second;

                        for (const auto& quintuplet : quintupletsListEmExternalP) {
                            EV << "{";
                            for (const int node : quintuplet) {
                                EV << node << ",";
                            }
                            EV<< "}" << endl;
                        }
                        EV << endl;
                        EV << "Reception *** " << endl;
                        EV << endl;

                        for (const auto& quintuplet : quintupletsListRecepExternalP) {
                            EV << "{";
                            for (const int node : quintuplet) {
                                EV << node << ",";
                            }
                            EV<< "}" << endl;
                        }

                        EV << endl;
                        EV << endl;
                        EV << endl;

                    }

                }
                // if received List Quintuplets from me for routing in my cluster
                else if (strcmp("Sending back List Quintuplets from my cluster sink", rcvListQuintMsg->getMsgContent()) == 0){
                    EV << "I received quintuplets" << endl;


                    int arraySize = rcvListQuintMsg->getDatasArraySize();

                    // Read all values in the datas[] array
                    for (int i = 0; i < arraySize; ++i) {
                        carListFromMySink.push_back(rcvListQuintMsg->getDatas(i));

                    }

                    // destructure the char List
                    if(!carListFromMySink.empty()){
                        std::pair<std::list<std::list<int>>, std::list<std::list<int>>> pairQuintuplets = destructureListCharQuint(carListFromMySink);

                        // Extract the results
                        quintupletsListEmission = pairQuintuplets.first;
                        quintupletsListReception = pairQuintuplets.second;

                        EV << "Emission *** " << endl;
                        for (const auto& quintuplet : quintupletsListEmission) {
                            EV << "{";
                            for (const int node : quintuplet) {
                                EV << node << ",";
                            }
                            EV<< "}" << endl;
                        }

                        EV << "Reception *** " << endl;

                        for (const auto& quintuplet : quintupletsListReception) {
                            EV << "{";
                            for (const int node : quintuplet) {
                                EV << node << ",";
                            }
                            EV<< "}" << endl;
                        }

                        EV << endl;
                        EV << endl;

                    }

                }
            }

        }

        PocMsg *Server::generateMessage(char msgname[], int dest, char  items[])
        {
            // Produce source and destination addresses.

            int src = getId();  // our module Id
//            int src = getIndex();  // our module index
            PocMsg *msg = new PocMsg(msgname);
            msg->setSource(src);
            msg->setDestination(dest);
            msg->setMsgContent(msgname);
            msg->setDatas(items);
            return msg;
        }

        bool Server::isIdInList(int id){
            return std::find(HostsIdList.begin(), HostsIdList.end(), id) != HostsIdList.end();
        }


        void Server::refreshDisplay() const
        {
            if (!channelBusy) {
                getDisplayString().setTagArg("i2", 0, "status/off");
                getDisplayString().setTagArg("t", 0, "");
            }
            else if (currentCollisionNumFrames == 0) {
                getDisplayString().setTagArg("i2", 0, "status/yellow");
                getDisplayString().setTagArg("t", 0, "RECEIVE");
                getDisplayString().setTagArg("t", 2, "#808000");
            }
            else {
                getDisplayString().setTagArg("i2", 0, "status/red");
                getDisplayString().setTagArg("t", 0, "COLLISION");
                getDisplayString().setTagArg("t", 2, "#800000");
            }
        }

        void Server::mySchduleAt(simtime_t delay, char message[]){
            char datas[] = "Ids";
            PocMsg *msgToSend = generateMessage(message, getId(), datas);
            scheduleAt(simTime()  + delay, msgToSend);
        }

        void Server::setValueAtListPosition(std::list<int>& list, size_t position, int newVal) {
            if (position >= list.size()) {
                throw std::out_of_range("Position is out of range");
            }

            auto it = list.begin();
            std::advance(it, position); // Move the iterator to the desired position
            *it = newVal; // Update the value
        }

        int Server::getValueAtListPosition(std::list<int>& list, size_t position){
            if (position >= list.size()) {
                throw std::out_of_range("Position is out of range");
            }

            auto it = list.begin();
            std::advance(it, position); // Move the iterator to the desired position
            return *it;
        }

        list<char> Server::structureListQuintuplets(list<list<int>> quintupletsList, list<list<int>> quintupletsListRecep){
            list<char> listChar;

            // quint emission
            listChar.push_back('[');
            for (const auto& quintuplet : quintupletsList) {

                listChar.push_back('{');
                for (const int node : quintuplet) {
                    listChar.push_back(static_cast<char>(node));
                }
                listChar.push_back('}');
            }
            listChar.push_back(']');


            // quint Reception
            listChar.push_back('+');
            listChar.push_back('[');
            for (const auto& quintuplet : quintupletsListRecep) {

                listChar.push_back('{');
                for (const int node : quintuplet) {
                    listChar.push_back(static_cast<char>(node));
                }
                listChar.push_back('}');
            }
            listChar.push_back(']');

            return listChar;

        }

        void Server::sendBackQuintToSinks(char msgSendExternalPackets[], list<char> charList, int sinkReceiverId){
            cModule *target = getSimulation()->getModule(sinkReceiverId);

            ListQuintMsg *listQuintMsg;
            listQuintMsg = new ListQuintMsg(msgSendExternalPackets);
            listQuintMsg->setMsgContent(msgSendExternalPackets);
            listQuintMsg->setSource(getId());
            listQuintMsg->setDestination(sinkReceiverId);

            listQuintMsg->setDatasArraySize(charList.size());
            int indexArray = 0;
            for (auto it = charList.begin(); it != charList.end(); ++it) {
                listQuintMsg->setDatas(indexArray, *it);
                indexArray++;
            }


            sendDirect(listQuintMsg->dup(), target, "in");
        }

        std::pair<std::list<std::list<int>>, std::list<std::list<int>>> Server::destructureListCharQuint(const std::list<char>& listChar) {
            std::list<std::list<int>> quintupletsList;
            std::list<std::list<int>> quintupletsListRecep;

            auto it = listChar.begin();

            // Function to parse a quintuplet list
            auto parseQuintuplets = [&it, &listChar]() -> std::list<std::list<int>> {
                std::list<std::list<int>> quintuplets;
                while (it != listChar.end() && *it != ']') {
                    if (*it == '{') {
                        std::list<int> quintuplet;
                        ++it;
                        while (it != listChar.end() && *it != '}') {
                            quintuplet.push_back(static_cast<int>(*it));
                            ++it;
                        }
                        if (it != listChar.end() && *it == '}') {
                            ++it;
                        }
                        quintuplets.push_back(quintuplet);
                    } else {
                        ++it;
                    }
                }
                if (it != listChar.end() && *it == ']') {
                    ++it;
                }
                return quintuplets;
            };

            // Parse the first quintuplet list
            if (it != listChar.end() && *it == '[') {
                ++it;
                quintupletsList = parseQuintuplets();
            }

            // Skip the '+' character
            if (it != listChar.end() && *it == '+') {
                ++it;
            }

            // Parse the second quintuplet list
            if (it != listChar.end() && *it == '[') {
                ++it;
                quintupletsListRecep = parseQuintuplets();
            }

            return std::make_pair(quintupletsList, quintupletsListRecep);
        }

        void Server::sendOnePacketToSink(int packetIdToSend, int sinkReceiverId){

            cModule *target = getSimulation()->getModule(sinkReceiverId); // the sink receiver at external cluster

            ListHostMsg *listHostMsg;
            char msgSendExternalPackets[] = "Sending 1 packet to destination sink";
            listHostMsg = new ListHostMsg(msgSendExternalPackets);
            listHostMsg->setMsgContent(msgSendExternalPackets);
            listHostMsg->setSource(getId());
            listHostMsg->setDestination(packetIdToSend); // the final destination in the cluster


            sendDirect(listHostMsg->dup(), target, "in");
        }

        void Server::computeGraphCircuitsPaths(char msgSendExternalPackets[], GraphStructure graphStructure){

            // adjacency List
            map<int, list<int>> adjacencyList = graphStructure.getAdjancyList();
            // Iterate over each node in the adjacency list
            for (const auto& pair : adjacencyList) {
                int node = pair.first;
                const list<int>& neighbors = pair.second;

                                    // Print the node and its neighbors
                EV << "Node " << node << " { ";
                for (int neighbor : neighbors) {
                    EV << neighbor << ",";
                }
                EV << "}";
                EV << endl;
            }


            // Traverse Graph, compute circuits, paths ...
            graphStructure.traverseGraph();


            // get Circuits List, and Paths
            list<list<int>> circuitList = graphStructure.getCircuitList();
            // Iterate over each circuit in the circuit list
            for (const auto& circuit : circuitList) {
                                    // Print the circuit
                EV << "Circuit: ";
                for (int node : circuit) {
                    EV << node << "-";
                }
                EV << endl;
            }


            // get empty Adjacency List
            map<int, list<int>> adjacencyList2 = graphStructure.getAdjancyList();
            // Display empty adjacency list
            for (const auto& pair : adjacencyList2) {
                int node = pair.first;
                const list<int>& neighbors = pair.second;

                                    // Print the node and its neighbors
                EV << "Node " << node << " { ";
                for (int neighbor : neighbors) {
                    EV << neighbor << ",";
                }
                EV << "}";
                EV << endl;
            }


            // compute Quintuplets Emission
            list<list<int>> quintupletsList = graphStructure.buildQuintupletsFromCircuitsList(circuitList);

            EV << endl;
            EV << "****   Quintuplets Transmission   *****" << endl;
            EV << endl;

            // add slots

            int slot = 1;
            list<int> quintupletRecep;
            list<list<int>> quintupletsListRecep;

            for (auto& entry : adjacencyList2) {
                for (auto& quintuplet : quintupletsList) {
                    int firstNode = quintuplet.front();
                    if(firstNode == entry.first){
                        // change slot if multi channel
                        if (strcmp("Sending back List Quintuplets from sink1", msgSendExternalPackets) == 0) {
                            size_t position = 2;
                            int newVal = slot;
                            setValueAtListPosition(quintuplet, position, newVal);
                        }


                        // build reception quintuplet
                        quintupletRecep.push_back(getValueAtListPosition(quintuplet, 1));     //receiver node
                        quintupletRecep.push_back(getValueAtListPosition(quintuplet, 0));     //emitter node
                        if (strcmp("Sending back List Quintuplets from sink1", msgSendExternalPackets) == 0) {
                            quintupletRecep.push_back(slot);     //slot number for multi channel
                        }else{
                            quintupletRecep.push_back(getValueAtListPosition(quintuplet, 2));     //slot number
                        }

                        quintupletRecep.push_back(1);        // 1 for reception
                        quintupletRecep.push_back(getValueAtListPosition(quintuplet, 4));     // receiver channel
                        quintupletsListRecep.push_back(quintupletRecep);



                        slot++;
                        quintupletRecep.clear();
                    }
                }
                slot = 1;
            }

            for (const auto& quintuplet : quintupletsList) {
                                    // Print the circuit
                EV << "Quintuplet: {";
                for (int node : quintuplet) {
                    EV << node << ",";
                }
                EV << "}" << endl;
            }

            EV << endl;
            EV << "****   Quintuplets Reception   *****" << endl;
            EV << endl;

            for (const auto& quintuplet : quintupletsListRecep) {
                                    // Print the circuit
                EV << "Quintuplet Recep: {";
                for (int node : quintuplet) {
                    EV << node << ",";
                }
                EV << "}" << endl;
            }


            // send back Quintuplets to every node.
            for (auto& entry : adjacencyList) {
                list<list<int>> OneSinkquintupletsListEmission;
                list<list<int>> OneSinkquintupletsListReception;

                for (auto& quintuplet : quintupletsList) {
                    int firstNode = quintuplet.front();
                    if(firstNode == entry.first){
                        OneSinkquintupletsListEmission.push_back(quintuplet);
                    }
                }

                for (auto& quintuplet : quintupletsListRecep) {
                    int firstNode = quintuplet.front();
                    if(firstNode == entry.first){
                        OneSinkquintupletsListReception.push_back(quintuplet);
                    }
                }

                list<char> charList = structureListQuintuplets(OneSinkquintupletsListEmission, OneSinkquintupletsListReception);

                sendBackQuintToSinks(msgSendExternalPackets, charList, entry.first);

            }

        }


        void Server::receiveQuintupletsFromSink(ListQuintMsg *rcvListQuintMsg, list<char> carList, list<list<int>> quintupletsListEmission, list<list<int>> quintupletsListReception){

            int arraySize = rcvListQuintMsg->getDatasArraySize();

            // Read all values in the datas[] array and put it in a List
            for (int i = 0; i < arraySize; ++i) {
                carList.push_back(rcvListQuintMsg->getDatas(i));

            }

            // destructure the char List
            if(!carList.empty()){
                std::pair<list<list<int>>, list<list<int>>> pairQuintuplets = destructureListCharQuint(carList);

                // Extract the results
                quintupletsListEmission = pairQuintuplets.first;
                quintupletsListReception = pairQuintuplets.second;

                for (const auto& quintuplet : quintupletsListEmission) {
                    EV << "{";
                    for (const int node : quintuplet) {
                        EV << node << ",";
                    }
                    EV<< "}" << endl;
                }

                EV << "Reception *** " << endl;

                for (const auto& quintuplet : quintupletsListReception) {
                    EV << "{";
                    for (const int node : quintuplet) {
                        EV << node << ",";
                    }
                    EV<< "}" << endl;
                }

                EV << endl;
                EV << endl;
                EV << endl;

            }

        }

        void Server::mySchduleRountAPacketAt(simtime_t delay, char message[], int packetIdToSend, int sinkReceiverId){
            char datas[] = "Ids";
            PocMsg *msgToSend = generateMessage(message, getId(), datas);
            msgToSend->setSource(sinkReceiverId);
            msgToSend->setDestination(packetIdToSend);
            scheduleAt(simTime()  + delay, msgToSend->dup());
        }

        void Server::finish()
        {

            EV << "duration: " << simTime() << endl;

//            // adjacency List
//            map<int, list<int>> adjacencyList = graph.getAdjancyList();
//            // Iterate over each node in the adjacency list
//            for (const auto& pair : adjacencyList) {
//                int node = pair.first;
//                const list<int>& neighbors = pair.second;
//
//                                    // Print the node and its neighbors
//                EV << "Node " << node << " { ";
//                for (int neighbor : neighbors) {
//                    EV << neighbor << ",";
//                }
//                EV << "}";
//                EV << endl;
//            }
//

//            // Log the contents of HostsIdList using EV
//            EV << "Contents of HostsIdList in " << getName() << " : ";
//            for (auto it = HostsIdList.begin(); it != HostsIdList.end(); ++it) {
//                EV << *it << " -- "; // Log each element
//            }
//
//            EV << endl;
//
//            for (auto it = HostsIdMapExternal.begin(); it != HostsIdMapExternal.end(); ++it) {
//                int key = it->first;
//                int value = it->second;
//                // Do something with the key and value
//                EV << key << " :: " << value << endl;
//            }
//
//            EV << endl;
//            EV << endl;
//            EV << endl;
//            EV << endl;
            recordScalar("duration", simTime());
        }

}; //namespace
