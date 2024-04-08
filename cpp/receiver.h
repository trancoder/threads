#include <thread>


#ifndef __RECEIVER_H__
#define __RECEIVER_H__

class Receiver {
public:
    void start();
    void stop();
private:
  std::thread mPlotThread;
  std::thread mainThread;

};

#endif