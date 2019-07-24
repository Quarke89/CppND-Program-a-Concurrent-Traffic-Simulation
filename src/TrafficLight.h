#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;

template <class T>
class MessageQueue
{
public:
    MessageQueue() {}

    void send(T &&msg);
    T receive();

private:
    std::deque<T> _queue;
    std::mutex _mutex;
    std::condition_variable _cond;
    
};


enum TrafficLightPhase {red, green};

class TrafficLight  : public TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();
    ~TrafficLight(){};

    // getters / setters
    TrafficLightPhase getCurrentPhase();    

    // typical behaviour methods
    void simulate();
    void waitForGreen();
    

private:
    // typical behaviour methods
    void cycleThroughPhases();
    TrafficLightPhase _currentPhase;
                        
    MessageQueue<TrafficLightPhase> _msgQ;  // message queue that holds the notifications for phase changes

    std::condition_variable _condition;
    std::mutex _mutex;
};

#endif