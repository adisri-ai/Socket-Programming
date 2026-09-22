#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <termios.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

atomic<bool> running(true);
string current_input = "";
mutex display_mutex;
struct termios orig_termios;

void enable_raw_mode(){
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

void disable_raw_mode(){
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void redraw_prompt(){
    cout << "\r\033[KYou: " << current_input << flush;
}

void print_incoming_message(const string& msg){
    lock_guard<mutex> lock(display_mutex);
    cout << "\r\033[K" << msg;
    // Fix: Add newline ONLY if the incoming message doesn't end with one
    if(!msg.empty() && msg.back() != '\n') cout << "\n";
    redraw_prompt();
}

void handle_receive(int server_socket){
    char buf[BUFFER_SIZE];
    while(running){
        memset(buf, 0, sizeof(buf));
        int bytes = recv(server_socket, buf, sizeof(buf) - 1, 0);
        if(bytes <= 0){
            if (running) {
                print_incoming_message("[Disconnected from server]");
                running = false;
            }
            break;
        }
        buf[bytes] = '\0';
        print_incoming_message(string(buf));
    }
}

int main(){
    const char* SERVER_IP = "127.0.0.1";
    struct sockaddr_in server_addr{};
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;

    // Fix 1: Replaced broken gethostname/bcopy calls with inet_pton
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        cout << "Invalid address / Address not supported" << endl;
        return 1;
    }

    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_socket < 0){
        cout << "Unable to create socket" << endl;
        return 1;
    }

    if(connect(server_socket, (struct sockaddr*)(&server_addr), sizeof(server_addr)) < 0){
        cout << "Unable to connect to server socket" << endl;
        return 1;
    }

    enable_raw_mode();
    atexit(disable_raw_mode);

    thread receive_thread(handle_receive, server_socket);

    {
        lock_guard<mutex> lock(display_mutex);
        cout << "Connected to server\n";
        redraw_prompt();
    }

    while(running){
        char ch = getchar();
        if(ch == EOF) break;

        lock_guard<mutex> lock(display_mutex);

        if(ch == '\n' || ch == '\r'){
            if(!current_input.empty()){
                // Fix 2: Replaced sizeof(current_input.c_str()) with current_input.length()
                send(server_socket, current_input.c_str(), current_input.length(), 0);
                
                // Fix 3: Clear draft buffer after sending
                current_input.clear();
            }
            cout << "\n";
            redraw_prompt();
        }
        else if(ch == 127 || ch == '\b'){
            if(!current_input.empty()){
                current_input.pop_back();
            }
            redraw_prompt();
        }
        else{
            current_input += ch;
            redraw_prompt();
        }
    }

    running = false;
    if(receive_thread.joinable()) {
        receive_thread.join();
    }

    close(server_socket);
    disable_raw_mode();
    return 0;
}
