#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define UDP_PORT 9090
#define BUFFER_SIZE 2048
using namespace std;
int main(){
    const char* SERVER_IP = "127.0.0.1";
    struct sockaddr_in server_addr{};
    server_addr.sin_port = htons(UDP_PORT);
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET , SERVER_IP , &server_addr.sin_addr);
    int client_socket = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP);
    while(true){
        string msg;
        cin>>msg;
        if(sendto(client_socket ,msg.c_str() , msg.length() (struct sockaddr*)(&server_addr) , sizeof(server_addr))<0) break;
        this_thread::sleep_for(chrono::milliseconds(800));
        char buf[256];
        int bytes = recvfrom(client_socket , buf , sizeof(buf)-1 , (struct sockaddr*)(&server_addr));
        if(bytes<=0) break;
        buf[bytes] = '\0';
        cout<<buf<<endl;
    }
    close(client_socket);
    return 0;

}
