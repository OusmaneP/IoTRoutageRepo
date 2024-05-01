simple Host
{
    parameters:
        @signal[state](type="long");
        @statistic[radioState](source="state";title="Radio state";enum="IDLE=0,TRANSMIT=1";record=vector);
        @signal[arrival](type="long");
        @statistic[hopCount](title="hop count"; source="arrival"; record=vector,stats; interpolationmode=none);
        double txRate @unit(bps);          // transmission rate
        volatile int pkLenBits @unit(b);   // packet length in bits
        volatile double iaTime @unit(s);   // packet interarrival time
        double slotTime @unit(s);          // zero means no slots (pure Aloha)
        double x @unit(m);                 // the x coordinate of the host
        double y @unit(m);                 // the y coordinate of the host
        double idleAnimationSpeed;         // used when there is no packet being transmitted
        double transmissionEdgeAnimationSpeed; // used when the propagation of a first or last bit is visible
        double midTransmissionAnimationSpeed; // used during transmission
        bool controlAnimationSpeed = default(true);
        double residualEnergy @unit(J);                 // the residual energy of nodes
        int numHosts;  // number of hosts 
        int numbPacketToSend;
        
        
        int sinkID; // the id of the sink in my cluster
        
//        int hostMemorySize; // max number of items the node can hold at a time
//        int itemsToSend[hostMemorySize]; // an array of those items to be transmited, int is the object to receive
//        
//        int receivedItems[hostMemorySize];
//        
//        int hostsNeighbors[numHosts] // list of nodes in my cluster
//        
        
        
        
        
        // sink/ Server;
        // liste des noeuds voisins;
        
        
        @display("i=device/smallrouter_s");
        
        
     gates:
        input in @directIn;
//        output out @directOut;
}

