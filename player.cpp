#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sstream>
#include "helper.h"
#include <string>
#include <vector>


using namespace std;
class Player{
public:
    int recv_neigh_fd;
    int recv_master_fd;
    int send_neigh_fd;
    int id;
    Server server;
    unsigned int port_to_server;
    char hostname_to_server[512];
    fd_set readfds;
    int fd_max;
    int player_num;
    
    Player(){
        fd_max = 0;
        FD_ZERO(&readfds);
        server.CreateServer("0");
        //send_neigh_fd = server.socket_fd;
        struct sockaddr addr;
        socklen_t addr_len = sizeof(addr);

        int status = getsockname(server.socket_fd, &addr, &addr_len);
        if(status == -1){
            perror("ERROR: GETSOCKNAME");
        }
        port_to_server = ntohs(((struct sockaddr_in*)& addr)->sin_port);
        gethostname(hostname_to_server,sizeof(hostname_to_server));
    }
    
    int connect_with_other(const char* hostname, const char*  port){
        int socket_fd;
        int status;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;
        
        memset(&host_info, 0, sizeof(host_info));
        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;

        status = getaddrinfo(hostname, port, &host_info, &host_info_list);
        if (status != 0) {
          cerr << "Error: cannot get address info for host" << endl;
          cerr << "  (" << hostname << "," << port << ")" << endl;
        }

        socket_fd = socket(host_info_list->ai_family,
                   host_info_list->ai_socktype,
                   host_info_list->ai_protocol);
        if (socket_fd == -1) {
          cerr << "Error: cannot create socket" << endl;
          cerr << "  (" << hostname << "," << port << ")" << endl;
        }
        
        //cout << "Connecting to " << hostname << " on port " << port << "..." << endl;
        
        status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
          cerr << "Error: cannot connect to socket" << endl;
          cerr << "  (" << hostname << "," << port << ")" << endl;
        }
        freeaddrinfo(host_info_list);
        return socket_fd;
    }
    
    void connnet_with_master(const char* hostname, const char* port){
        //hostname is from argv[1], port is from argv[2]
        recv_master_fd = connect_with_other(hostname,port);
        char recv_data[1024];
        memset(recv_data,0,sizeof(recv_data));
        if(recv(recv_master_fd,recv_data,sizeof(recv_data),0) == -1){
            perror("ERROR: RECV DATA FORM SERVER ERROR");
        }
	//while(recv)
        //cout<<"data:"<<string(recv_data)<<endl;
        string addr_data = string(hostname_to_server) + " " + to_string(port_to_server);
        if(send(recv_master_fd, addr_data.c_str(), strlen(addr_data.c_str()),0)==-1){
            perror("ERROR: SEND MASTER MY ADDR INFO");
        }
        vector<string>data = packet(string(recv_data));
        if(data.size() == 2){
            //send my addr info
            //host name + " " + port
            player_num = stoi(data[0]);
            //cout<<"num of player:"<<player_num<<endl;
            id  = stoi(data[1]);
            
            send_neigh_fd = server.Connect();
//            cout<<"data size:"<<data.size()<<" id:"<<id<<endl;
        }
        else{
            player_num = stoi(data[0]);
            //cout<<"num of player:"<<player_num<<endl;
            id = stoi(data[1]);
            cout<<"Connected as player "<<id<<" out of "<<player_num<<" total players"<<endl;
            const char* host = data[2].c_str();
            const char* port = data[3].c_str();
//            cout<<"data size:"<<data.size()<<" id:"<<id<<endl;
            //connect_with_neigh()
            
            recv_neigh_fd = connect_with_other(host,port);
            send_neigh_fd = server.Connect();
        }
        //get the last client's ip and port number
        memset(recv_data,0,sizeof(recv_data));
        if(id == 0){
           memset(recv_data,0,sizeof(recv_data));
           if(recv(recv_master_fd,recv_data,sizeof(recv_data),0) == -1){
               perror("ERROR: RECV DATA FORM SERVER ERROR");
           }
           data = packet(string(recv_data));
            
           const char* host =data[0].c_str();
           const char* port = data[1].c_str();
            cout<<"Connected as player "<<id<<" out of "<<player_num<<" total players"<<endl;
           recv_neigh_fd = connect_with_other(host,port);
       }
       

    }
     
    void play(){
        srand((unsigned int)time(NULL) + id);
        
        
        fd_max = recv_neigh_fd;
        if(fd_max < recv_master_fd){
            fd_max = recv_master_fd;
        }
        if(fd_max < send_neigh_fd){
            fd_max = send_neigh_fd;
        }
        vector<int> fds;
        fds.push_back(recv_neigh_fd);
//        cout<<"recv_neigh_fd:"<<recv_neigh_fd<<endl;
        fds.push_back(recv_master_fd);
//        cout<<"recv_master_fd:"<<recv_master_fd<<endl;
        fds.push_back(send_neigh_fd);
        //cout<<"send_neigh_fd:"<<send_neigh_fd<<endl;
        for(int i = 0; i< 3; i++){
            FD_SET(fds[i],&readfds);
            
        }
        while(true){
            int random = rand() % 2;
            fd_set copy = readfds;
	    
            if(select(fd_max+1,&copy,nullptr,nullptr,nullptr) == -1){
                cerr<<"ERROR: SELECT"<<endl;
            }
	    //cout<<"stuck jre"<<endl;
            //cout<<"into while"<<endl;
           //send to neighbor
            if(FD_ISSET(recv_neigh_fd,&copy)){
	      char msg[2048] = {'\0'};
                memset(msg,0,sizeof(msg));
                if(recv(recv_neigh_fd,msg,sizeof(msg),0)==-1){
                    cerr<<"ERROR: RECEIVE MES"<<endl;
                }
		string test = (string)msg;
		if(test.length() == 0){
		  return;
		  //continue;
		  }
                //cout<<msg<<endl;
                stringstream ss(test);
                int hop;
                ss>>hop;
                string trace;
                ss>>trace;
                hop--;
                string message;
                if(hop ==0){
		  /*cout<<"trace:"<<trace<<endl;
		   if(trace.length() == 0){
		    continue;
		    }*/
                    message =  trace + to_string(id);
		    //cout<<"messaga:"<<message<<endl;
                    if(send(recv_master_fd,message.c_str(),strlen(message.c_str()),0)==-1){
                        cerr<<"ERROR: SEND MESSAGE TO MASTER"<<endl;
                    }
		    cout<<"I’m it"<<endl;
		    continue;
		    //return;
                }
                else{
                    message =  to_string(hop) + " "+trace + to_string(id) + ",";
		   if(trace.length() == 0){
		    continue;
		  }
                    if(random == 1){
                            if(send(recv_neigh_fd, message.c_str(),strlen(message.c_str()),0) ==-1){
                                cerr<<"ERROR: SEND MESSAGE TO NEIGHBOR"<<endl;
                            }
                            int recv_neigh_id;
                            if(id == 0){
                                recv_neigh_id = player_num -1;
                            }
                            else{
                                recv_neigh_id = id - 1;
                            }
                            cout<<"Sending potato to "<<recv_neigh_id<<endl;
                            
                        }
                        else{
                            if(send(send_neigh_fd, message.c_str(),strlen(message.c_str()),0) ==-1){
                                cerr<<"ERROR: SEND MESSAGE TO NEIGHBOR"<<endl;
                            }
                            int send_neigh_id;
                            if(id == player_num - 1){
                                send_neigh_id = 0;
                            }
                            else{
                                send_neigh_id = id + 1;
                            }
                            cout<<"Sending potato to "<<send_neigh_id<<endl;
                            
                        }

                }
                                
            }
            //send to neighbor
            if(FD_ISSET(send_neigh_fd,&copy)){
	      char msg[2048] = {'\0'};
                memset(msg,0,sizeof(msg));
                if(recv(send_neigh_fd,msg,sizeof(msg),0) ==-1){
                    cerr<<"ERROR: RECEIVE MES"<<endl;
                }
		string test = (string)msg;
		if(test.length() == 0){
		  //continue;
		  return;
		}
                //cout<<msg<<endl;
                stringstream ss(test);
                int hop;
                ss>>hop;
                string trace;
                ss>>trace;
                hop--;
                string message;
                if(hop == 0){
		  //cout<<"trace:"<<trace<<endl;
		  if(trace.length() == 0){
		    continue;
		  }
                    message =  trace + to_string(id);
		    //cout<<"message"<<message<<endl;
                    if(send(recv_master_fd,message.c_str(),strlen(message.c_str()),0) ==-1){
                        cerr<<"ERROR: SEND MESSAGE TO MASTER"<<endl;
                    }
		    cout<<"I’m it"<<endl;
		    //continue;
		    //return;
                }
                else{
		  if(trace.length() == 0){
		    continue;
		  }
                    message =  to_string(hop) +" "+ trace + to_string(id) + ",";
                    if(random == 1){
                        if(send(recv_neigh_fd, message.c_str(),strlen(message.c_str()),0) ==-1){
                            cerr<<"ERROR: SEND MESSAGE TO NEIGHBOR"<<endl;
                        }
                        int recv_neigh_id;
                        if(id == 0){
                            recv_neigh_id = player_num -1;
                        }
                        else{
                            recv_neigh_id = id - 1;
                        }
                        cout<<"Sending potato to "<<recv_neigh_id<<endl;
                        
                    }
                    else{
                        if(send(send_neigh_fd, message.c_str(),strlen(message.c_str()),0)==-1){
                            cerr<<"ERROR: SEND MESSAGE TO NEIGHBOR"<<endl;
                        }
                        int send_neigh_id;
                        if(id == player_num - 1){
                            send_neigh_id = 0;
                        }
                        else{
                            send_neigh_id = id + 1;
                        }
                        
                        cout<<"Sending potato to "<<send_neigh_id<<endl;
                    }
                }
                
            }
            //receive from master
            if(FD_ISSET(recv_master_fd,&copy)){
                char msg[2014];
                memset(msg,0,sizeof(msg));
                if(recv(recv_master_fd,msg,sizeof(msg),0) ==-1){
                    cerr<<"ERROR: RECEIVE MES"<<endl;
		    //return;
                }
                //cout<<"receive from master:"<<msg<<endl;
                if(string(msg) == "zero" || string(msg) == "endgame" || string(msg) == ""){
                    //cout<<msg<<endl;
                    return;
                }
               else{
                    stringstream ss((string(msg)));
                    int hop;
                    ss>>hop;
                    hop--;
		    if(hop == 0){
		      string message =  to_string(id);
		    //cout<<"message"<<message<<endl;
		      if(send(recv_master_fd,message.c_str(),strlen(message.c_str()),0) ==-1){
                        cerr<<"ERROR: SEND MESSAGE TO MASTER"<<endl;
		      }
		      cout<<"I’m it"<<endl;
		      continue;
		    }
		    //cout<<"hop:"<<hop<<endl;
                    string message;
                    message =  to_string(hop) + " " + to_string(id)+ ",";
                    //cout<<"after the first time potato:"<<message<<endl;
                    if(random == 1){
                        if(send(recv_neigh_fd, message.c_str(),strlen(message.c_str()),0)==-1){
                            cerr<<"ERROR: SEND MESSAGE TO NEIGHBOR"<<endl;
                        }
			int recv_neigh_id;
			if(id == 0){
                            recv_neigh_id = player_num -1;
                        }
                        else{
                            recv_neigh_id = id - 1;
                        }
                        cout<<"Sending potato to "<<recv_neigh_id<<endl;
                    }
                    else{
                        if(send(send_neigh_fd, message.c_str(),strlen(message.c_str()),0) ==-1){
                            cerr<<"ERROR: SEND MESSAGE TO NEIGHBOR"<<endl;
                        }

			 int send_neigh_id;
                        if(id == player_num - 1){
                            send_neigh_id = 0;
                        }
                        else{
                            send_neigh_id = id + 1;
                        }
                        
                        cout<<"Sending potato to "<<send_neigh_id<<endl;
                    }
                    
                }
            }
        }
        
    }
         
     vector<string> packet(string str){
        vector<string> ans;
        stringstream ss(str);
        string s;
        while(ss>>s){
            ans.push_back(s);
        }
            return ans;
     }
            
    ~Player(){
        close(recv_master_fd);
        close(recv_neigh_fd);
        close(send_neigh_fd);
    }
};


int main(int argc, const char *argv[])
{

  const char *hostname = argv[1];
  const char *port = argv[2];
  
  if (argc != 3) {
      cout << "Usage: player <machine_name> <hostname>\n" << endl;
      return 1;
  }
    Player p;
    p.connnet_with_master(hostname,port);
    p.play();
    // sleep(2);
  
  return 0;
}
