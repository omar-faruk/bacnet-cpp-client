#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iomanip>
using namespace std;
constexpr uint16_t BACNET_PORT = 47808; // 0xBAC0
#define BUFFER_SIZE 1024
constexpr int TIMEOUT_SECONDS = 3;

// Corrected 8-byte BACnet Who-Is packet (NPDU control = 0x00)
const uint8_t WHO_IS_PACKET[] = {
    0x81, 0x0B,   // BVLC: Type (BACnet/IP), Function (Original-Broadcast)
    0x00, 0x08,   // BVLC Length: 8 bytes
    0x01, 0x00,   // NPDU: Version (1), Control (0x00 = Local Broadcast, NO DNET)
    0x10, 0x08    // APDU: Unconfirmed-Req, Service: Who-Is
};


void parseIAmResponse(const uint8_t* buffer, ssize_t length, const std::string& senderIP) {
    if (length < 13 || buffer[0] != 0x81) return;

    size_t apduOffset = 6; // Standard non-routed NPDU payload offset
    
    // Check APDU Type (0x10 = Unconfirmed Request) & Service Choice (0x00 = I-Am)
    if (buffer[apduOffset] != 0x10 || buffer[apduOffset + 1] != 0x00) {
        return;
    }

    size_t tagOffset = apduOffset + 2;

    // Check for BACnet Application Tag 12 (Object Identifier) -> 0xC4
    if (buffer[tagOffset] == 0xC4) {
        uint32_t objData = (static_cast<uint32_t>(buffer[tagOffset + 1]) << 24) |
                           (static_cast<uint32_t>(buffer[tagOffset + 2]) << 16) |
                           (static_cast<uint32_t>(buffer[tagOffset + 3]) << 8)  |
                           (static_cast<uint32_t>(buffer[tagOffset + 4]));

        uint32_t objectType = (objData >> 22) & 0x03FF;
        uint32_t deviceInstance = objData & 0x003FFFFF;

        if (objectType == 8) { // 8 = Device Object Type
            std::cout << "[Device Discovered] IP: " << std::left << std::setw(16) << senderIP 
                      << " | Device ID: " << deviceInstance << std::endl;
        }
    }
}

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) return 1;

    // 1. Enable Broadcast
    int broadcastEnable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

    // 2. Allow Address Reuse (So you can run multiple tools on 47808)
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 3. BIND TO PORT 47808 (Sets UDP Source Port to 0xBAC0)
    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(BACNET_PORT);
    localAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, reinterpret_cast<sockaddr*>(&localAddr), sizeof(localAddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    // 4. Configure Broadcast Address (Optionally use your subnet broadcast like 10.42.0.255)
    sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(BACNET_PORT);
    inet_pton(AF_INET, "10.42.0.255", &broadcastAddr.sin_addr); // Match bacnet-stack broadcast

    // 5. Send Packet
    sendto(sockfd, WHO_IS_PACKET, sizeof(WHO_IS_PACKET), 0,reinterpret_cast<sockaddr*>(&broadcastAddr), sizeof(broadcastAddr));

            // Receive responses
    uint8_t recvBuffer[BUFFER_SIZE];
    socklen_t addrLen = sizeof(broadcastAddr);

        while (true) {
        ssize_t recvLen = recvfrom(sockfd, recvBuffer, BUFFER_SIZE, 0,
                                   reinterpret_cast<sockaddr*>(&broadcastAddr), &addrLen);

        if (recvLen < 0) {
            break; // Timeout reached or standard socket error
        }

        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(broadcastAddr.sin_addr), ipStr, INET_ADDRSTRLEN);

        parseIAmResponse(recvBuffer, recvLen, std::string(ipStr));
    }


    close(sockfd);
    return 0;
}