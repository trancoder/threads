#include <thread>
#include <boost/asio.hpp>

#ifndef __RECEIVER_H__
#define __RECEIVER_H__

class Receiver {
public:    
    void start();
    void stop();
    void startReceiving( boost::asio::ip::udp::socket& recvSocket);
private:
    std::thread mPlotThread;
    std::thread mainThread;

    boost::asio::io_context ioContext;

    
    bool mShutdown = false;

};

#endif