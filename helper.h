#include <string>
#include <cstring>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <assert.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace std;


class Server{
public:
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname;
    const char *port;
    int yes = 1;
        
    
    void CreateServer(const char *cin_port){
        hostname = NULL;
        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        host_info.ai_flags    = AI_PASSIVE;
        
        
        status = getaddrinfo(hostname, cin_port, &host_info, &host_info_list);
        if (status != 0) {
          cerr << "Error: cannot get address info for host" << endl;
          cerr << "  (" << hostname << "," << port << ")" << endl;
          //return -1;
        } //if
        //set port
        port = cin_port;
        
        socket_fd = socket(host_info_list->ai_family,
                   host_info_list->ai_socktype,
                   host_info_list->ai_protocol);
          //cout<<"socket_fd:"<<socket_fd<<endl;
        if (socket_fd == -1) {
          cerr << "Error: cannot create socket" << endl;
          cerr << "  (" << hostname << "," << port << ")" << endl;
          //return -1;
        } //if
        
        
        if( setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&yes,
                sizeof(yes)) < 0 )
          {
              perror("setsockopt");
              exit(EXIT_FAILURE);
          }
        
        status = ::bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
          cerr << "Error: cannot bind socket" << endl;
          cerr << "  (" << hostname << "," << port << ")" << endl;
          //return -1;
        } //if

        status = listen(socket_fd, 100);
        if (status == -1) {
          cerr << "Error: cannot listen on socket" << endl;
          cerr << "  (" << hostname << "," << port << ")" << endl;
          //return -1;
        }
    }
    
    
    int Connect(){
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        int client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        
        if (client_connection_fd == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            exit(EXIT_FAILURE);
            return -1;
        }
        return client_connection_fd;
    }
    
    ~Server(){
        freeaddrinfo(host_info_list);
        close(socket_fd);
    }
    
};
