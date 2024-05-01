
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


                EV << getName() << " : Received from : " << rcvListHostMsg->getMsgContent() << endl;

                if (rcvListHostMsg) {
                   // Get the size of the datas[] array
                   int arraySize = rcvListHostMsg->getDatasArraySize();

                   // Read all values in the datas[] array and put them in HostsIdMap
                   for (int i = 0; i < arraySize; ++i) {
                       int hostId = rcvListHostMsg->getDatas(i);
                       EV << hostId << " -- ";
                       HostsIdMap[hostId] = rcvListHostMsg->getMsgContent();
                   }
                   EV << endl;
                } else {
                   // Handle the case where the message is not of type ListHostMsg
                   EV << "Received message is not of type ListHostMsg" << endl;
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

        void Server::finish()
        {
            EV << "duration: " << simTime() << endl;

            // Log the contents of HostsIdList using EV
            EV << "Contents of HostsIdList in " << getName() << " : ";
            for (auto it = HostsIdList.begin(); it != HostsIdList.end(); ++it) {
                EV << *it << " -- "; // Log each element
            }

            EV << endl;

            for (auto it = HostsIdMap.begin(); it != HostsIdMap.end(); ++it) {
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
