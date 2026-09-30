#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <bits/stdc++.h>
#define SERVER_PORT 5432;
using namespace std;
int main(){
    struct sockaddr_in server_addr{};
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    int server_socket = socket(AF_INET , SOCK_STREAM , 0);
    bind(server_socket , (struct sockaddr*)(&server_addr) , sizeof(server_addr));
    listen(server_socket , 5);
    char buf[256];
    while(1){
        struct sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)(&client_addr) , &len);
        while(true){
            int bytes = recv(client_socket , buf , sizeof(buf)-1 , 0);
            if(bytes<=0){
                cout<<"Client discconnected";
                close(client_socket);
                break;
            }
            buf[bytes] = '\0';
            cout<<"Received: "<<buf<<endl;
            for(int i = 0 ; i<bytes ; ++i){
                buf[i] = toupper(buf[i]);
            }
            cout<<"Sedning "<<buf<<endl;
            send(client_socket , buf , bytes , 0);
        }
    }
    close(server_socket);
    return 0;
}
