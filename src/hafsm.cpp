#include "hafsm.hpp"

void HAFSM::go_active() {
    // TransitionState to Going_Active.

}

void HAFSM::transition_state(HaState newState) {
    switch (m_arpState) {
        case OUT_OF_SERVICE:
            if (newState == GOING_ACTIVE) {
                //Need to do a
            }
            if (newState == STANDBY) {
                // Perform actions to transition to standby
                m_arpState = STANDBY;
            }
            break;
        case STANDBY:
            if (newState == GOING_ACTIVE) {
                // Perform actions to transition to going active
                m_arpState = GOING_ACTIVE;
            }
            break;
        case GOING_ACTIVE:
            if (newState == ACTIVE) {
                // Perform actions to transition to active
                m_arpState = ACTIVE;
            } else if (newState == GOING_STANDBY) {
                // Perform actions to transition to going standby
                m_arpState = GOING_STANDBY;
            }
            break;
        case GOING_STANDBY:
            if (newState == STANDBY) {
                // Perform actions to transition to standby
                m_arpState = STANDBY;
            } else if (newState == ACTIVE) {
                // Perform actions to transition to active
                m_arpState = ACTIVE;
            }
            break;
        case ACTIVE:
            if (newState == GOING_STANDBY) {
                // Perform actions to transition to going standby
                m_arpState = GOING_STANDBY;
            }
            break;
    }
}
