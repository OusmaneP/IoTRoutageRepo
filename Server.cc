
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

                    EV << "Im ready to send the hosts Ids : " << sinkName << endl;
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
                            EV << "Module name starts with 'sink': " << moduleName << endl;

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




                }
                else if (strcmp("WakeUp and Send External Ids to Sink 1", ttmsg->getMsgContent()) == 0) {
                    EV << "Waking Up and Send External Ids to Sink 1" << endl;

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

                if (strcmp("sendingId to server", ttmsg->getMsgContent()) == 0){
                    if (numberOfMessagesForInitialization <= numberOfHosts) {
                        HostsIdList.push_back(ttmsg->getSource());
                        numberOfMessagesForInitialization = numberOfMessagesForInitialization+1;
                        EV << "Received packet from " << ttmsg->getSource() << "\n";
                    }
                }
            }

            // Check if msg is of type ListHostMsg
            else if (dynamic_cast<ListHostMsg *>(msg) != nullptr) {
                ListHostMsg *rcvListHostMsg = check_and_cast<ListHostMsg *>(msg);

                // when sink receive external packets from hosts.
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
                    EV << rcvListHostMsg->getSenderModule()->getName() << " Sent for External :: ";

                    // Read all values in the datas[] array and put them in HostsIdMapExternal
                    for (int i = 0; i < arraySize; ++i) {
                        int hostId = rcvListHostMsg->getDatas(i);
                        EV << hostId << " -- ";

                        if(isIdInList(hostId)){
                            graph.addEdge(getId(), hostId);
                            graphMultiCanalSink1.addEdge(rcvListHostMsg->getSource(), getId());
                        }else{
                            auto it = HostsIdMapExternal.find(hostId);

                            if(it != HostsIdMapExternal.end()){

                                const char *second = it->second.c_str();

                                cModule *module = getParentModule()->getSubmodule(second);
                                graphMultiCanalSink1.addEdge(rcvListHostMsg->getSource(), module->getId());

                            }

                        }

                    }


                    EV << endl;
                }
                else{

                    EV << getName() << " : Received from : " << rcvListHostMsg->getMsgContent() << endl;

                    if (rcvListHostMsg) {
                        // Get the size of the datas[] array
                        int arraySize = rcvListHostMsg->getDatasArraySize();

                        // Read all values in the datas[] array and put them in HostsIdMapExternal
                        for (int i = 0; i < arraySize; ++i) {
                            int hostId = rcvListHostMsg->getDatas(i);
                            EV << hostId << " -- ";
                            HostsIdMapExternal[hostId] = rcvListHostMsg->getMsgContent();
                        }
                        EV << endl;
                    } else {
                        // Handle the case where the message is not of type ListHostMsg
                        EV << "Received message is not of type ListHostMsg" << endl;
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

        void Server::finish()
        {
            if (strcmp("sink1", getName()) == 0) {


                map<int, list<int>> adjacencyList = graphMultiCanalSink1.getAdjancyList();

                    // Iterate over each node in the adjacency list
                    for (const auto& pair : adjacencyList) {
                        int node = pair.first;
                        const list<int>& neighbors = pair.second;

                        // Print the node and its neighbors
                        EV << "Node " << node << " neighbors: ";
                        for (int neighbor : neighbors) {
                            EV << neighbor << " ";
                        }
                        EV << endl;
                    }

                    graphMultiCanalSink1.DFS(2);
                    graphMultiCanalSink1.DFS(3);
                    graphMultiCanalSink1.DFS(4);
                    graphMultiCanalSink1.DFS(5);

                    list<list<int>> circuitList = graphMultiCanalSink1.getCircuitList();

                    // Iterate over each circuit in the circuit list
                    for (const auto& circuit : circuitList) {
                        // Print the circuit
                        EV << "Circuit: ";
                        for (int node : circuit) {
                            EV << node << "-";
                        }
                        EV << endl;
                    }


            }

            EV << "duration: " << simTime() << endl;

            // Log the contents of HostsIdList using EV
            EV << "Contents of HostsIdList in " << getName() << " : ";
            for (auto it = HostsIdList.begin(); it != HostsIdList.end(); ++it) {
                EV << *it << " -- "; // Log each element
            }

            EV << endl;

            for (auto it = HostsIdMapExternal.begin(); it != HostsIdMapExternal.end(); ++it) {
                int key = it->first;
                std::string value = it->second;
                // Do something with the key and value
                EV << key << " :: " << value << endl;
            }

            EV << endl;
            EV << endl;
            EV << endl;
            EV << endl;
            recordScalar("duration", simTime());
        }

}; //namespace
