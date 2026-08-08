#ifndef _BACNET_CLIENT_HPP
#define _BACNET_CLIENT_HPP

#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// BIND TO PORT 0xBAC0 (Sets UDP Source Port to 47808)
#define BACNET_PORT         0xBAC0
#define BACNET_PORT_REUSE   1
#define BROADCAST_ENABLE    1

class BACnetClient
{
    private:
        void transmit_message();
        void receive_message();
        int sockfd;
        int reuse = 1;
        int broadcastEnable = 1;

    public:
        BACnetClient() = default;
        ~BACnetClient() = default;
        

    
};

#endif