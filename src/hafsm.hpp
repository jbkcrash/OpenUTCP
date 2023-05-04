#pragma once
#include "arphandler.hpp"

// Two processes
// Each starts as standby... The process starts a timer, waiting to see that there is no ARP for the Virtual IP.
// When the process sees a gratuitous arp from another it stops the timer and enters into standby
// If the timer expires, then the process will assume it is the only one or isolated and it becomes active.
// The number of millis is recorded as the "epoc", this is used in quorum and active/active conflict resolution.


class HAFSM
{
private:
    bool m_Active = false;
    ArpHandler *m_arpHandler;
public:
    HAFSM(ArpHandler *arpHandler)
        : m_arpHandler(arpHandler){};
    enum HaState
    {
        OUT_OF_SERVICE,
        STANDBY,
        GOING_ACTIVE,
        GOING_STANDBY,
        ACTIVE
    };
    // Called to start the current application active
    void go_active(ArpHandler *arpHandler);
    // Called to put the current application in standby
    void go_standby(ArpHandler *arpHandler);

    void transition_state(HaState newState);
private:
    HaState m_arpState = OUT_OF_SERVICE;
};