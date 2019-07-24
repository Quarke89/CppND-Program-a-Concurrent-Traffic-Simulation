#include <iostream>
#include <random>
#include "TrafficLight.h"
#include <future>


/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{ 
    std::unique_lock<std::mutex> uLock(_mutex);

    // wait until queue is not empty
    _cond.wait(uLock, [this] { return !_queue.empty(); });

    // get message from back of queue
    T msg = std::move(_queue.back());

    // clear the queue as we just want the latest phase change        
    _queue.clear();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> uLock(_mutex);
    
    // add message to queue
    _queue.push_back(std::move(msg));

    // unblock thread waiting on the condition variable
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{

    // wait until a green notification is received from the message queue
    // receive will block the thread until a message is available
    while(true){
        if(_msgQ.receive() == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // launch thread of cycling traffic light phases
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{

    // cycle duration is picked at random between 4 - 6 seconds
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4000, 6000);
    double cycleDuration = distr(eng);

    // init stop watch
    std::chrono::time_point<std::chrono::system_clock> lastUpdate = std::chrono::system_clock::now();

    while(true){

        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // compute time difference to stop watch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {

            // toggle traffic light phase
            if(_currentPhase == TrafficLightPhase::green){
                _currentPhase = TrafficLightPhase::red; 
            }else{
                _currentPhase = TrafficLightPhase::green;
                
            }
            
            // send notification of phase change to message queue
            TrafficLightPhase msg = _currentPhase;            
            _msgQ.send(std::move(msg));

            // reset stop watch for next cycle
            cycleDuration = distr(eng);
            lastUpdate = std::chrono::system_clock::now();
        }
        
    }
}

