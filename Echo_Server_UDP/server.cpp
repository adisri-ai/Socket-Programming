#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define UDP_PORT 9090
#define BUFFER_SIZE 2048
using namespace std;
int main(){
    struct sockaddr_in server_addr{};
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int server_socket = socket(AF_INET , SOCK_DGRAM , IPPROTO_TCP);
    bind(server_socket , (struct sockaddr*)(&server_addr), sizeof(server_addr));
    listen(server_socket , 5);
    while(true){
        struct sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        char buf[256];
        int bytes = recvfrom(server_socket , buf , sizeof(buf)-1 , &len, (struct sockaddr*)(&client_addr) );
        if(bytes<=0) break;
        buf[bytes] = '\0';
        string msg = "";
        int port = ntohs(client_addr.sin_port);
        char server_ip[64];
        inet_ntop(AF_INET , client_addr.sin_family , &server_ip , sizeof(server_ip));
        msg+=to_string(port);
        msg+=string(buf);
        sendto(server_socket , msg.c_str() , msg.length() , (struct sockaddr*)(&client_addr) , len);
    }
    return 0;
}
