#include <iostream>
#include <receiver.h>
#include "boost/asio.hpp"

bool terminatedSignal = false;

// Signal handler for Ctrl+C
// void signalHandler(const boost::system::error_code& error, int signal_number) {
//     std::cout << "Ctrl+C received." << std::endl;   
//     terminatedSignal = true;
// }

void signalHandler(int signum) {
    std::cout << "Ctrl+C received." << std::endl;
    terminatedSignal = true;
}

int main() {
    
    // boost::asio::io_context io_context;
    // boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    // signals.async_wait(signalHandler);
    signal(SIGINT, signalHandler);

    Receiver receiver;    
    receiver.start();
    while (true){
        if (terminatedSignal){
            std::cout << "Stopping everything" << std::endl; 
            receiver.stop();
            break;
        }
    }
    return 0;
}

