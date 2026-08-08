#include <bacnet-client.hpp>
using namespace std;

BACnetClient::BACnetClient()
{
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd > 0)
    {
        setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));

        // Allow Address Reuse (So you can run multiple tools on 47808)

        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

        sockaddr_in localAddr{};
        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(BACNET_PORT);
        localAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(sockfd, reinterpret_cast<sockaddr *>(&localAddr), sizeof(localAddr)) < 0)
        {
            cerr<<"BACnet Client Init: Bind failed";
            close(sockfd);
            return;
        }

        // 4. Configure Broadcast Address (Optionally use your subnet broadcast like 10.42.0.255)
        sockaddr_in broadcastAddr{};
        broadcastAddr.sin_family = AF_INET;
        broadcastAddr.sin_port = htons(BACNET_PORT);
        inet_pton(AF_INET, "10.42.0.255", &broadcastAddr.sin_addr); // Match bacnet-stack broadcast
    }
}
