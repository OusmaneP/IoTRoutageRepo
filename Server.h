
#ifndef __ALOHA_SERVER_H_
#define __ALOHA_SERVER_H_

#include <omnetpp.h>

#include "pocMsg_m.h"
#include "ListHostMsg_m.h"
#include "GraphStructure.h"
#include <list>
#include <map>
#include "ListQuintMsg_m.h"
#include <iostream>
#include <vector>
#include <sstream>

using namespace omnetpp;

namespace pocSimulation {


class Server : public cSimpleModule
{
  private:
    // state variables, event pointers
    bool channelBusy;
    simtime_t radioDelay;
    double txRate;
    cPar *iaTime;
    cPar *pkLenBits;
    cMessage *endRxEvent;
    std::list<int> HostsIdList; // ids of hosts in the same cluster of that Sink
    int numberOfHosts;
    int numberOfMessagesForInitialization;

    // my params    *********
    std::string sinkName;
    ListHostMsg *listHostMsg; // hosts Ids to be broadcasted by each sink for all nodes in its cluster and to other sinks in the network
    std::map<int, int> HostsIdMapExternal; // external nodes ids saved along side their sink ids.
    std::map<int, std::string> packetsToSendMap;
    std::list<int> listExternalPackets; // List of external packets (ids)
    GraphStructure graph; // graph for internal routing
    GraphStructure graphMultiCanalSink1; // graph for external routing sink1

    list<char> carListFromSB; // list structured by base station to concat list quintuplets Em & Recep to send to each sink in the network fo external routing
    list<list<int>> quintupletsListEmExternalP;
    list<list<int>> quintupletsListRecepExternalP;
    std::list<char> carListFromMySink; // list structured by my cluster sink to concat list quintuplets Em & Recep to send to each sink in the network fo external routing
    std::list<std::list<int>> quintupletsListEmission;
    std::list<std::list<int>> quintupletsListReception;


    cModule *host;
    PocMsg *newMessage;
    double aoiWidth;
    double aoiheight;
    double sinkX;
    double sinkY;
    long currentCollisionNumFrames;
    long receiveCounter;
    simtime_t recvStartTime;
    enum { IDLE = 0, TRANSMISSION = 1, COLLISION = 2 };
    simsignal_t channelStateSignal;

    // statistics
    simsignal_t receiveBeginSignal;
    simsignal_t receiveSignal;
    simsignal_t collisionLengthSignal;
    simsignal_t collisionSignal;

    const double propagationSpeed = 299792458.0;

  public:
    Server();
    virtual ~Server();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual PocMsg *generateMessage(char msgname[], int dest, char  items[]);
    virtual void finish() override;
    virtual void refreshDisplay() const override;
//    my methods
    virtual bool isIdInList(int id);
    virtual void mySchduleAt(simtime_t delay, char message[]);
    void setValueAtListPosition(std::list<int>& list, size_t position, int newVal);
    int getValueAtListPosition(std::list<int>& list, size_t position);
    list<char> structureListQuintuplets(list<list<int>> quintupletsList, list<list<int>> quintupletsListRecep);
    void sendBackQuintToSinks(char msgSendExternalPackets[], list<char> charList, int sinkReceiverId);
    std::pair<std::list<std::list<int>>, std::list<std::list<int>>> destructureListCharQuint(const std::list<char>& listChar);
    void sendOnePacketToSink(int packetIdToSend, int sinkReceiverId);
    void computeGraphCircuitsPaths(char msgSendExternalPackets[], GraphStructure graphStructure);
    void receiveQuintupletsFromSink(ListQuintMsg *rcvListQuintMsg, list<char> carList, list<list<int>> quintupletsListEmission, list<list<int>> quintupletsListReception);
    void mySchduleRountAPacketAt(simtime_t delay, char message[], int packetIdToSend, int sinkReceiverId);

};

}; //namespace

#endif

