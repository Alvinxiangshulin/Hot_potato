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
#include <stdlib.h>
#include "helper.h"
using namespace std;

class ringmaster{
public:
    vector<int> clients;
    Server master;
    fd_set readfds;
    int hop_num;
    int player_num;
    int fd_max;
    char addr_buffer[1024];
    
    
    ringmaster(const char* port){
        master.CreateServer(port);
        FD_ZERO(&readfds);
        fd_max = 0;
    }
    
    void CreateListen(){
        int client_connection_fd;
        for(int i = 0 ; i< player_num; i++){
            client_connection_fd = master.Connect();
            if(client_connection_fd == -1){
                perror("ERROR: CONNECT");
            }
            //set fd_max
            if(fd_max < client_connection_fd){
                fd_max = client_connection_fd;
            }
            //add fd t0 client
            clients.push_back(client_connection_fd);
            //fd set the client_fd
            FD_SET(client_connection_fd,&readfds);
            if(i == 0){
                //send his id
                string num_id = to_string(player_num) + " "+to_string(i)+ '\n';
                if(send(client_connection_fd,num_id.c_str(),strlen(num_id.c_str()),0) == -1){
                    perror("ERROR: SEND ID");
                }
            }
            else{
                memset(addr_buffer,0,sizeof(addr_buffer));
                if(recv(clients[i-1], &addr_buffer, sizeof(addr_buffer), 0) == -1){
                    perror("ERROR: RECEIVE LAST ADDR INFO");
                    
                }
                string data = to_string(player_num) + " " + to_string(i) + " " + (string)addr_buffer;
               
                if(send(client_connection_fd, data.c_str(), strlen(data.c_str()), 0) == -1){
                    perror("ERROR: SEND ID + ADDR INFO");
                    
                }
            }
        }
        
        memset(addr_buffer,0,sizeof(addr_buffer));
        if(recv(client_connection_fd, &addr_buffer, sizeof(addr_buffer), 0) == -1){
            perror("ERROR: RECEIVE LAST ADDR INFO");
        }
        
        //send id and last client addr info to the first client
        if(send(clients[0], &addr_buffer, sizeof(addr_buffer), 0) == -1){
            perror("ERROR: SEND ADDR LAST TIME");
        }
    }
    
    void game(){
        if(hop_num == 0){
            for(int i = 0 ; i<player_num; i++){
                string zero = "zero";
                if(send(clients[i],zero.c_str(), strlen(zero.c_str()),0) ==-1){
                    cerr<<"ERROR: SEND POTATO WHEN HOP IS ZERO"<<endl;
                }
            }
            return;
        }
        else{
            string potato = to_string(hop_num);
            srand((unsigned int)time(NULL) + player_num);
            int random = rand() % player_num;
            cout<<"Ready to start the game, sending potato to player "<<random<<endl;
            if(send(clients[random],potato.c_str(),strlen(potato.c_str()),0) == -1){
                cerr<<"ERROR SENDING POTATO"<<endl;
            }
            for(int i = 0; i < player_num; i++){
                if(fd_max < clients[i]){
                    fd_max = clients[i];
                }
                FD_SET(clients[i], &readfds);
            }
            if(select(fd_max+1,&readfds,nullptr,nullptr,nullptr) == -1){
                cerr<<"ERROR: SELECT"<<endl;
            }
           
            fd_set copy = readfds;
            for(int i = 0; i<player_num; i++){
                if(FD_ISSET(clients[i], &copy)){
                    //cout<<"received last potato"<<endl;
                    char after_game[1024];
                    memset(after_game,0,sizeof(after_game));
                    if(recv(clients[i],&after_game,sizeof(after_game),0) == -1){
                        cerr<<"ERROR: RECEIVE"<<endl;
                    }

                    for(int j = 0; j < player_num; j++){
                        string end = "endgame";
                        if(send(clients[j],end.c_str(),sizeof(end.c_str()),0) == -1){
                            cerr<<"ERROR SENDING ENDING POTATO"<<endl;
                        }
                    }
		                        cout<<"Trace of potato:"<<endl;
                    cout<<after_game<<endl;
                    break;
                }
                
            }
        }
    }
    
    ~ringmaster(){
    }
};

int main(int argc, const char *argv[]){
    if (argc != 4) {
         cout << "Usage: ringmaster <port_num> <num_players> <num_hops>" << endl;
         return 1;
    }
    if(atoi(argv[2]) <=1 ){
        cout<<"Player should be greater than 1"<<endl;
        return 1;
    }
    if(atoi(argv[3]) <0 || atoi(argv[3]) >512){
        cout<<"Hops should be no more than 512 and no less than 0"<<endl;
        return 1;
    }
    cout<<"Potato Ringmaster"<<endl;
    cout<<"Players = "<<atoi(argv[2])<<endl;
    cout<<"Hops = "<<atoi(argv[3])<<endl;
    const char* port = argv[1];
    ringmaster rm = ringmaster(port);
    rm.player_num = atoi(argv[2]);
    rm.hop_num = atoi(argv[3]);
    rm.CreateListen();
    rm.game();
    return 0;
}
