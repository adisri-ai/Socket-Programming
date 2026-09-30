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
#define SEVER_PORT 5432
using namespace std;
int main(int argc , char*argv[]){
    string addr = "127.0.0.1";
    if(argc!=1){
        const char* c = argv[1];
        addr = string(c);
    }
    char buf[256];
    const char* host = addr;
    struct sockaddr_in server_addr{};
    server_addr.sin_port = SERVER_PORT;
    server_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET , host ,  &server_addr.sin_addr) < 0){
        cout<<"Issue in resolving server addr"<<endl;
        return 1;
    }
    int client_socket = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
    if(connect(client_socket , (struct sockaddr*)(&server_addr) , sizeof(server_addr)) < 0){
        cout<<"Error conncecting server"<<endl;
        return 1;
    }
    while(1){
        string s;
        cout<<"Enter the string"<<endl;
        getline(cin , s);
        if(send(client_socket , s.c_str() , sizeof(s.c_str()) , 0) < 0){
            cout<<"Error in sedning"<<endl;
            continue;
        }
        int bytes = recv(client_socket , buf , sizeof(buf)-1 , 0);
        if(bytes<=0){
            cout<<"Termianting server connection"<<endl;
            break;
        }
        buf[bytes] = "\0";
        cout<<"Received: "<<buf<<endl;
    }
    close(client_socket);
    return 0;
}
