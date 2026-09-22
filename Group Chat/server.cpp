#include <iostream>
#include <string>
#include <set>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

mutex client_mutex;
set<int> clients;

void broadcast(int client_socket, const char* buf, int bytes){
    // Fix: Acquire lock before reading from the global clients set
    lock_guard<mutex> lock(client_mutex);
    for(int client : clients){
        if(client != client_socket){
            if(send(client, buf, bytes, 0) < 0){
                cout << "Unable to send to client: " << client << endl;
            }
        }
    }
}

// Fix: Pass client_addr by value to avoid thread race conditions
void handle_client(int client_socket, struct sockaddr_in client_addr){
    char buf[BUFFER_SIZE];
    int port = ntohs(client_addr.sin_port);
    // Fix: Replaced invalid intoa() with inet_ntoa()
    string addr = inet_ntoa(client_addr.sin_addr);

    cout << "Client connected from " << addr << ":" << port << endl;

    while(true){
        memset(buf, 0, sizeof(buf));
        int bytes = recv(client_socket, buf, sizeof(buf) - 1, 0);
        if(bytes <= 0){
            cout << "Terminating connection with client " << addr << ":" << port << endl;
            break;
        }
        buf[bytes] = '\0';
        broadcast(client_socket, buf, bytes);
    }

    {
        lock_guard<mutex> lock(client_mutex);
        // Fix: Corrected variable name from st to clients and fixed typo cleint_socket
        clients.erase(client_socket);
    }
    close(client_socket);
}

int main(){
    struct sockaddr_in server_addr{};
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Fix: Corrected IPPROCO_TCP typo to IPPROTO_TCP
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_socket < 0){
        cout << "Error in making socket" << endl;
        return 1;
    }

    // Set SO_REUSEADDR to allow rapid server restarts on port 8080
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(server_socket, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0){
        cout << "Error in binding" << endl;
        // Fix: Replaced invalid continue statement with return 1
        return 1;
    }

    // Fix: Replaced invalid server.listen() call with standard listen()
    if (listen(server_socket, 5) < 0) {
        cout << "Error in listening" << endl;
        return 1;
    }

    cout << "Server listening on port " << PORT << "..." << endl;

    while(true){
        struct sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        // Fix: Passed pointer &len instead of value len
        int client_socket = accept(server_socket, (struct sockaddr*)(&client_addr), &len);
        if(client_socket < 0){
            cout << "Error in connecting to client" << endl;
            continue;
        }
        {
            lock_guard<mutex> lock(client_mutex);
            clients.insert(client_socket);
        }
        thread client_thread(handle_client, client_socket, client_addr);
        client_thread.detach();
    }

    close(server_socket);
    return 0;
}