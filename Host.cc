
#include <algorithm>

#include "Host.h"

namespace pocSimulation {

Define_Module(Host);

Host::Host()
{
    newMessage = nullptr;
}

Host::~Host()
{
    delete lastPacket;
    cancelAndDelete(newMessage);
}

void Host::initialize()
{
    EV << "parent module " << getParentModule()->getSubmodule("numHosts") << endl;
    arrivalSignal = registerSignal("arrival");
    stateSignal = registerSignal("state");
//    server = getModuleByPath("server");

    //int n = getVectorSize();  // module vector size
    //int dest = intuniform(0, n-2);
    //host = getModuleByPath("^.host[2]");
    host = getParentModule()->getSubmodule("host", 2);  //same as the previous line
//    if (!server)
//        throw cRuntimeError("server not found");

    txRate = par("txRate");
    iaTime = &par("iaTime");
    pkLenBits = &par("pkLenBits");
    numberOfHosts = par("numHosts");
    residualEnergy = par("residualEnergy");

    slotTime = par("slotTime");
    isSlotted = slotTime > 0;

    //endTxEvent = new cMessage("send/endTx");
    char msgname[] = "sendingId to server";
    sprintf(msgname, "sendingId to server");
//    newMessage = generateMessage(msgname,server->getId());
    state = IDLE;
    emit(stateSignal, state);
    pkCounter = 0;


    x = par("x").doubleValue();
    y = par("y").doubleValue();

//    double serverX = server->par("x").doubleValue();
//    double serverY = server->par("y").doubleValue();

    idleAnimationSpeed = par("idleAnimationSpeed");
    transmissionEdgeAnimationSpeed = par("transmissionEdgeAnimationSpeed");
    midtransmissionAnimationSpeed = par("midTransmissionAnimationSpeed");

//    double dist = std::sqrt((x-serverX) * (x-serverX) + (y-serverY) * (y-serverY));
//    radioDelay = dist / propagationSpeed;

    //To display where the circles will start going (the center of the circle, which is the node emitting the message)
    getDisplayString().setTagArg("p", 0, x);
    getDisplayString().setTagArg("p", 1, y);

//    scheduleAt(getId(), newMessage);

    // my Params
    numbPacketToSend = par("numbPacketToSend");
    getAllHostsIdList();
    generatePacketsToSend();
    showPacketsToSend();


}

void Host::handleMessage(cMessage *msg)
{

    // Check if msg is of type PocMsg
    if (dynamic_cast<PocMsg *>(msg) != nullptr) {
        getParentModule()->getCanvas()->setAnimationSpeed(transmissionEdgeAnimationSpeed, this);
        PocMsg *ttmsg = check_and_cast<PocMsg *>(msg);
        if (strcmp("Sending my LeaderID", ttmsg->getMsgContent()) == 0){
            EV<< "I received this ID from " << ttmsg->getSource() << "\n";

            // save sink Id and Name
            mySinkId = ttmsg->getSource();
            cModule *module = getSimulation()->getModule(mySinkId);
            if(module){
                sinkName = module->getFullName();
            }

            // schedule wake up to send ACK
            char msgname[] = "wakeUP and Send ACK";
            PocMsg *msgToSend = generateMessage(msgname, getId());
            simtime_t delay = 0.000002;
            scheduleAt(simTime()  + delay, msgToSend);

        }
        else if (strcmp("wakeUP and Send ACK", ttmsg->getMsgContent()) == 0){
            EV << "Sending my ACK to :: " << sinkName << endl;

            cModule *target = getParentModule()->getSubmodule(sinkName);

            char msgname[] = "Receiving ACK";
            PocMsg *msgACK = generateMessage(msgname, mySinkId);

            sendDirect(msgACK, target, "in");

            mySchduleAt(0.000002088425, "WakeUp and Send External Packets");

        }
        else if (strcmp("WakeUp and Send External Packets", ttmsg->getMsgContent()) == 0){
            EV << "Waking up to send external packets " << endl;

            cModule *target = getParentModule()->getSubmodule(sinkName);

            ListHostMsg *listHostMsg;
            char msgSendExternalPackets[] = "Send External Packets";
            listHostMsg = new ListHostMsg(msgSendExternalPackets);
            listHostMsg->setMsgContent(msgSendExternalPackets);
            listHostMsg->setSource(getId());

            listHostMsg->setDatasArraySize(listOfPacketToSend.size());
            int indexArray = 0;
            for (auto it = listOfPacketToSend.begin(); it != listOfPacketToSend.end(); ++it) {
                 listHostMsg->setDatas(indexArray, *it);
                 indexArray++;
            }

            listHostMsg->setDestination(mySinkId);
            sendDirect(listHostMsg->dup(), target, "in");
        }
        else {
            throw cRuntimeError("invalid state");
        }

    }
        // Check if msg is of type ListHostMsg
    else if (dynamic_cast<ListHostMsg *>(msg) != nullptr) {
        ListHostMsg *rcvListHostMsg = check_and_cast<ListHostMsg *>(msg);

        if (rcvListHostMsg) {
           // Get the size of the datas[] array
           int arraySize = rcvListHostMsg->getDatasArraySize();

           // Read all values in the datas[] array and put them in List Neighbor HostsId
           for (int i = 0; i < arraySize; ++i) {
               int hostId = rcvListHostMsg->getDatas(i);
               hostNeighborsIdList.push_back(hostId);
           }
        } else {
            // Handle the case where the message is not of type ListHostMsg
            EV << "Received message is not of type ListHostMsg" << endl;
        }
    }
    //ASSERT(msg == endTxEvent);
}

simtime_t Host::getNextTransmissionTime()
{
    simtime_t t = simTime() + iaTime->doubleValue();

    if (!isSlotted)
        return t;
    else
        // align time of next transmission to a slot boundary
        return slotTime * ceil(t/slotTime);
}


PocMsg *Host::generateMessage(char msgname[], int dest)
{
    // Produce source and destination addresses.
    int src = getId();  // our module index
    /*int n = getVectorSize();  // module vector size
    int dest = intuniform(0, n-2);
    if (dest >= src)
        dest++;*/

    //char msgname[20];
    //sprintf(msgname, "message from -%d-to-%d", src, dest);

    // Create message object and set source and destination field.
    PocMsg *msg = new PocMsg(msgname);
    msg->setSource(src);
    msg->setDestination(dest);
    msg->setMsgContent(msgname);
    return msg;
}

void Host::refreshDisplay() const
{
    cCanvas *canvas = getParentModule()->getCanvas();
    const int numCircles = 2;
    const double circleLineWidth = 10;

    // create figures on our first invocation
    if (!transmissionRing) {
        auto color = cFigure::GOOD_DARK_COLORS[getId() % cFigure::NUM_GOOD_DARK_COLORS];

        transmissionRing = new cRingFigure(("Host" + std::to_string(getIndex()) + "Ring").c_str());
        transmissionRing->setOutlined(false);
        transmissionRing->setFillColor(color);
        transmissionRing->setFillOpacity(0.25);
        transmissionRing->setFilled(true);
        transmissionRing->setVisible(false);
        transmissionRing->setZIndex(-1);
        canvas->addFigure(transmissionRing);

        for (int i = 0; i < numCircles; ++i) {
            auto circle = new cOvalFigure(("Host" + std::to_string(getIndex()) + "Circle" + std::to_string(i)).c_str());
            circle->setFilled(false);
            circle->setLineColor(color);
            circle->setLineOpacity(0.75);
            circle->setLineWidth(circleLineWidth);
            circle->setZoomLineWidth(true);
            circle->setVisible(false);
            circle->setZIndex(-0.5);
            transmissionCircles.push_back(circle);
            canvas->addFigure(circle);
        }
    }

    if (lastPacket) {
        // update transmission ring and circles
        if (transmissionRing->getAssociatedObject() != lastPacket) {
            transmissionRing->setAssociatedObject(lastPacket);
            for (auto c : transmissionCircles)
                c->setAssociatedObject(lastPacket);
        }

        simtime_t now = simTime();
        simtime_t frontTravelTime = now - lastPacket->getSendingTime();
        simtime_t backTravelTime = now - (lastPacket->getSendingTime() + lastPacket->getDuration());

        // conversion from time to distance in m using speed
        double frontRadius = std::min(ringMaxRadius, frontTravelTime.dbl() * propagationSpeed);
        double backRadius = backTravelTime.dbl() * propagationSpeed;
        double circleRadiusIncrement = circlesMaxRadius / numCircles;

        // update transmission ring geometry and visibility/opacity
        double opacity = 1.0;
        if (backRadius > ringMaxRadius) {
            transmissionRing->setVisible(false);
            transmissionRing->setAssociatedObject(nullptr);
        }
        else {
            transmissionRing->setVisible(true);
            transmissionRing->setBounds(cFigure::Rectangle(x - frontRadius, y - frontRadius, 2*frontRadius, 2*frontRadius));
            transmissionRing->setInnerRadius(std::max(0.0, std::min(ringMaxRadius, backRadius)));
            if (backRadius > 0)
                opacity = std::max(0.0, 1.0 - backRadius / circlesMaxRadius);
        }

        transmissionRing->setLineOpacity(opacity);
        transmissionRing->setFillOpacity(opacity/5);

        // update transmission circles geometry and visibility/opacity
        double radius0 = std::fmod(frontTravelTime.dbl() * propagationSpeed, circleRadiusIncrement);
        for (int i = 0; i < (int)transmissionCircles.size(); ++i) {
            double circleRadius = std::min(ringMaxRadius, radius0 + i * circleRadiusIncrement);
            if (circleRadius < frontRadius - circleRadiusIncrement/2 && circleRadius > backRadius + circleLineWidth/2) {
                transmissionCircles[i]->setVisible(true);
                transmissionCircles[i]->setBounds(cFigure::Rectangle(x - circleRadius, y - circleRadius, 2*circleRadius, 2*circleRadius));
                transmissionCircles[i]->setLineOpacity(std::max(0.0, 0.2 - 0.2 * (circleRadius / circlesMaxRadius)));
            }
            else
                transmissionCircles[i]->setVisible(false);
        }

        // compute animation speed
        double animSpeed = idleAnimationSpeed;
        if ((frontRadius >= 0 && frontRadius < circlesMaxRadius) || (backRadius >= 0 && backRadius < circlesMaxRadius))
            animSpeed = transmissionEdgeAnimationSpeed;
        if (frontRadius > circlesMaxRadius && backRadius < 0)
            animSpeed = midtransmissionAnimationSpeed;
        canvas->setAnimationSpeed(animSpeed, this);
    }
    else {
        // hide transmission rings, update animation speed
        if (transmissionRing->getAssociatedObject() != nullptr) {
            transmissionRing->setVisible(false);
            transmissionRing->setAssociatedObject(nullptr);

            for (auto c : transmissionCircles) {
                c->setVisible(false);
                c->setAssociatedObject(nullptr);
            }
            canvas->setAnimationSpeed(idleAnimationSpeed, this);
        }
    }

    // update host appearance (color and text)
    getDisplayString().setTagArg("t", 2, "#808000");
    if (state == IDLE) {
        getDisplayString().setTagArg("i", 1, "");
        getDisplayString().setTagArg("t", 0, "");
    }
    else if (state == TRANSMIT) {
        getDisplayString().setTagArg("i", 1, "yellow");
        getDisplayString().setTagArg("t", 0, "TRANSMIT");
    }
}

void Host::showPacketsToSend(){
    EV << getName() << getIndex() << " : " << getId() << " :Packets Ill send :: ";
    for (auto it = listOfPacketToSend.begin(); it != listOfPacketToSend.end(); ++it) {
        EV << *it << " -- "; // Log each element
    }
}

void Host::generatePacketsToSend(){

    for (int i = 0; i <= numbPacketToSend -1; i++){
        int indexToPick = intuniform(0, allHostsIdList.size() -1);

        listOfPacketToSend.push_back(getValueAtIndex(indexToPick));
    }
}

void Host::getAllHostsIdList(){
    // Get a pointer to the system module
    cModule *systemModule = getSystemModule();
    // Iterate over all submodules of the system module
    cModule::SubmoduleIterator iter(systemModule);
    int indexOfHostId = 0;
    while (!iter.end()) {
        cModule *module = iter();

        // Check if the module name starts with "sink"
        const char *moduleName = module->getName();
        int moduleId = module->getId();
        int moduleIndex = module->getIndex();
        if ((strncmp(moduleName, "host", 4) == 0) && (moduleId != getId())) {
            // Module name starts with "host"
            allHostsIdList.push_back(moduleId);
            indexOfHostId++;
        }

        // Move to the next submodule
        ++iter;
    }
}

int Host::getValueAtIndex(int index){
    auto id = allHostsIdList.begin();
    std::advance(id, index);

    return *id;
}

void Host::mySchduleAt(simtime_t delay, char message[]){
    PocMsg *msgToSend = generateMessage(message, getId());
    scheduleAt(simTime()  + delay, msgToSend);
}

void Host::finish()
{
    EV << "Contents of Hosts Neighbors IdList in " << getName() << getIndex() << " : ";
    for (auto it = hostNeighborsIdList.begin(); it != hostNeighborsIdList.end(); ++it) {
        EV << *it << " -- "; // Log each element
    }

    EV << endl;
}



}; //namespace
