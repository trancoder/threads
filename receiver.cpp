#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "boost/property_tree/ptree.hpp"
#include "boost/regex.hpp"
#include <boost/json.hpp>

struct timespec32 
{
    uint32_t tv_sec;
    uint32_t tv_nsec;
};

int main() {
    bool mLaunchTimeReceived = false;

    // Prepare boost::asio objects
    boost::asio::io_context io_context_afatds;
    boost::asio::io_context io_context_weaponServerSim;

    // To handle launch time from AFATDS
    std::atomic<double> launchTimeFromAFATDS; //in milliseconds

    std::cout << "Start AFATDSThread" << std::endl;
    
    auto AFATDSThread = std::thread([&launchTimeFromAFATDS, &io_context_afatds, &io_context_weaponServerSim, &mLaunchTimeReceived](){
        bool AfatdsConnected = false;
        
        while (!AfatdsConnected && !mLaunchTimeReceived){
            try {
                std::string AfatdsIp = "192.168.1.220";
                std::string AfatdsPort = "1234";
                std::string AfatdsPath = "/launch-event-service/ws";
                                
                boost::asio::ip::tcp::resolver resolver(io_context_afatds);
                boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws(io_context_afatds);

                // Resolve the WebSocket domain name
                auto const results = resolver.resolve(AfatdsIp, AfatdsPort);
                
                printf("Waiting for the websocket Connection...\n");
                // Connect to the WebSocket server
                boost::asio::connect(ws.next_layer(), results.begin(), results.end());                        
            
                printf("Websocket Connection is established succesfully\n");        

                // Read and print data from the server asynchronously
                boost::beast::flat_buffer buffer;

                // Perform WebSocket handshake asynchronously
                ws.async_handshake(AfatdsIp, AfatdsPath,[&](const boost::beast::error_code& ec) {
                    if (ec) {
                        std::cout << "Handshake failed: " << ec.message() << std::endl;
                        return;
                    }
                    printf("Handshake successful!\n");
                
                    // Start asynchronous read operation        
                    ws.async_read(buffer, [&](boost::beast::error_code read_ec, std::size_t bytes_transferred) {
                        if (!read_ec) {                                                                       
                            try{
                                //Parse the received message as JSON                        
                                std::string message = boost::beast::buffers_to_string(buffer.data());
                                std::istringstream iss(message);
                                boost::property_tree::ptree pt;
                                boost::property_tree::json_parser::read_json(iss, pt);

                                // Display content
                                printf("Received JSON message:");
                                for (const auto& pair : pt) {                                            
                                    if (pair.first == "launchTime") {
                                        launchTimeFromAFATDS = pair.second.get_value<double>();
                                        if (mLaunchTimeReceived){
                                            std::cout << "Already received launch time from WeaponServer Sim, ignore this timestamp" << launchTimeFromAFATDS << std::endl;
                                            launchTimeFromAFATDS = 0;
                                        }
                                        else{
                                            std::cout << "launchTimeFromAFATDS =" << launchTimeFromAFATDS << std::endl;
                                            mLaunchTimeReceived = true;
                                            // stop weaponServerSim
                                            io_context_weaponServerSim.stop();
                                            printf("Stop weaponServerSim connection\n");
                                        }                                            
                                        break;
                                    }
                                }
                            }                                                     
                            catch (const std::exception& e) {
                                std::cerr << "Exception: " << e.what() << std::endl;
                            }                                    
                        } 
                        else {
                            std::cout << "WebSocket read error: " << read_ec.message();
                        }                                                                                               
                    });
                });    
            
                // Run the io_context to start processing asynchronous operations
                io_context_afatds.run();
                AfatdsConnected = true;
                printf("Finish websocket asynchronous read operation\n");
                                                                                    
            }
            catch (const std::exception& e) {
                std::cout << e.what() << std::endl;
                sleep(1); // check connection every 1 sec until it is established
            }                  
        }
    });

    // To handle launch time from WeaponServerSim
    timespec32 launchTimeFromWeaponServerSim;
    auto weaponServerSimThread = std::thread([&launchTimeFromWeaponServerSim, &io_context_afatds, &io_context_weaponServerSim,  &mLaunchTimeReceived](){
        bool weaponServerSimConnected = false;
        boost::system::error_code ec;
        
        std::string weaponServerSimIp = "192.168.1.220";
        short unsigned int weaponServerSimPort = 1235;
        boost::asio::ip::tcp::endpoint weaponServerSimEndpoint{boost::asio::ip::address::from_string(weaponServerSimIp), weaponServerSimPort};

        // Create a TCP socket
        boost::asio::ip::tcp::socket socket(io_context_weaponServerSim);

        // Attempt to connect. If error, wait before attempting again.
        while(!weaponServerSimConnected && !mLaunchTimeReceived)
        {                        
            socket.connect(weaponServerSimEndpoint, ec);            
            if (ec)
            {
                std::cout << "Failed to connect to weapon server: " << ec.message() << std::endl;
                std::cout << "Attempting to reconnect." << std::endl;
                // Close before reattempting to connect.
                socket.close();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            else
            {
                // Once connected. Wait for timestamp message sent from the message server.
                // Custom timespec struct to get timestamp from HIMARS. Only 32 bits since
                // that is the current architecture of the vehicle's software.
                std::cout << "Waiting for timestamp from the Weapon Server..." << std::endl;

                std::vector<char> buffer(8); // Assuming we are receiving 2 uint32_t values (8 bytes for total)

                // Receive data asynchronously
                boost::asio::async_read(socket, boost::asio::buffer(buffer),
                    [&](const boost::system::error_code &error, std::size_t bytes_transferred) {
                    if (!error) {
                        // Assuming the data received is 2 uint32_t values
                        if (bytes_transferred > 0) {
                            weaponServerSimConnected = true;
                            
                            if (mLaunchTimeReceived){
                                std::cout << "Already received launch time from AFATDS, ignore this timestamp" << std::endl;
                                launchTimeFromWeaponServerSim.tv_nsec = 0;
                                launchTimeFromWeaponServerSim.tv_sec = 0;
                            }   
                            else{
                                mLaunchTimeReceived = true;

                                uint32_t sec, nsec;
                                std::memcpy(&sec, buffer.data(), sizeof(uint32_t));
                                std::memcpy(&nsec, buffer.data() + sizeof(uint32_t), sizeof(uint32_t));
                                launchTimeFromWeaponServerSim.tv_sec = sec;
                                launchTimeFromWeaponServerSim.tv_nsec  = nsec;
                                
                                printf("launchTimeFromWeaponServerSim s = %f, ns = %f \n", launchTimeFromWeaponServerSim.tv_sec, launchTimeFromWeaponServerSim.tv_nsec);            
                            }
                            // stop AFATDS connection
                            io_context_afatds.stop();
                            std::cout << "Stop AFATDS connection" << std::endl; 
                        } else {
                                printf("Received unexpected data size: %d", bytes_transferred);
                        }
                    } 
                    else {
                        if (error == boost::asio::error::operation_aborted) {
                            std::cout << "Async read operation aborted." << std::endl;
                        } else {
                            std::cout << "Error receiving data: " << error.message() << std::endl;
                        }
                    }
                });

                // Run the io_context_weaponServerSim to start processing asynchronous operations
                io_context_weaponServerSim.run();   
                std::cout << "Finish TCP asynchronous read operation" << std::endl;
            }
        }
        socket.close();
    });

    AFATDSThread.join();
    weaponServerSimThread.join();
    return 0;
}

