
#ifndef __ALOHA_SERVER_H_
#define __ALOHA_SERVER_H_

#include <omnetpp.h>

#include "pocMsg_m.h"
#include "ListHostMsg_m.h"
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
    std::list<int> HostsIdList;
    int numberOfHosts;
    int numberOfMessagesForInitialization;

    // my params    *********
    std::string sinkName;
    ListHostMsg *listHostMsg;
    std::map<int, std::string> HostsIdMap;
    std::map<int, std::string> packetsToSendMap;

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
    virtual bool isIdInList(int id);
};

}; //namespace

#endif

