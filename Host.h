
#ifndef __ALOHA_HOST_H_
#define __ALOHA_HOST_H_
#include "pocMsg_m.h"
#include <omnetpp.h>
#include "ListHostMsg_m.h"

using namespace omnetpp;

namespace pocSimulation {

/**
 * PocSimulation host; see NED file for more info.
 */
class Host : public cSimpleModule
{
  private:
    // parameters
    simtime_t radioDelay;
    double txRate;
    cPar *iaTime;
    cPar *pkLenBits;
    simtime_t slotTime;
    bool isSlotted;
    int numberOfHosts;
    double residualEnergy;

    // state variables, event pointers etc
    cModule *server;
    cModule *host;
    //cMessage *endTxEvent;
    PocMsg *newMessage;
    enum { IDLE = 0, TRANSMIT = 1 } state;
    simsignal_t stateSignal;
    simsignal_t arrivalSignal;
    int pkCounter;

    // position on the canvas, unit is m
    double x, y;

    // speed of light in m/s
    const double propagationSpeed = 299792458.0;

    // animation parameters
    const double ringMaxRadius = 2000; // in m
    const double circlesMaxRadius = 1000; // in m
    double idleAnimationSpeed;
    double transmissionEdgeAnimationSpeed;
    double midtransmissionAnimationSpeed;

    // figures and animation state
    //cPacket *lastPacket = nullptr; // a copy of the last sent message, needed for animation
    PocMsg *lastPacket = nullptr; // a copy of the last sent message, needed for animation
    mutable cRingFigure *transmissionRing = nullptr; // shows the last packet
    mutable std::vector<cOvalFigure *> transmissionCircles; // ripples inside the packet ring

    // my Params
    int mySinkId;
    const char *sinkName;
    std::list<int> hostNeighborsIdList;
    int numbPacketToSend;
    std::list<int> allHostsIdList;
    std::list<int> listOfPacketToSend;

  public:
    Host();
    virtual ~Host();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;
    virtual PocMsg *generateMessage(char msgname[], int dest);
    simtime_t getNextTransmissionTime();
    virtual void generatePacketsToSend();
    virtual void getAllHostsIdList();
    virtual int getValueAtIndex(int index);
    virtual void showPacketsToSend();
    virtual void mySchduleAt(simtime_t delay, char message[]);
};

}; //namespace

#endif

