//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 1992-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//



network pocSimulation
{
    parameters:
        int numHosts;  // number of hosts
        double txRate @unit(bps);  // transmission rate
        double slotTime @unit(ms);  // zero means no slots 
        double residualEnergy @unit(J);
        @display("bgi=background/terrain,s;bgb=1700,1200");

        double SbWidth @unit(m); // Width of the sub-zone
        double SbHeight @unit(m);
    submodules:

        sink1: Server;
        sink2: Server;
        sink3: Server;
        sink4: Server;
        
        host[numHosts]: Host {
            txRate = txRate;
            slotTime = slotTime;
        }

}

