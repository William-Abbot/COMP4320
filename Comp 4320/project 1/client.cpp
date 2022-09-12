#include "unp.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <chrono>
#include <random>
#include<bits/stdc++.h>

using namespace std;

void gremlin(char *packet, double P, double P2) {
	double prob = P * 100;
	double prob2 = P2 * 100;

	unsigned seed = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator (seed);

	uniform_real_distribution<float> dist(0,100);

	int drop = dist(generator);

	int n = dist(generator);
	int n1 = dist(generator);
	int n2 = dist(generator);
	int n3 = dist(generator);

	if(drop < prob2){
		packet[1] = 0;
		return;
	}

	if(n < prob) {
		if(n1 < 50){
			packet[0] = packet[0] + 1;
			return;
		}
		if(n2 < 30){
			for(int i = 0; i < 2; i++){
				packet[i] = packet[i] + 1;
			}
			return;
		}
		if(n3 < 20){
			for(int i = 0; i < 3; i++){
				packet[i] = packet[i] + 1;
			}
			return;
		}
	}
}

int main(int argc, char **argv){
	int sd, n;
	struct sockaddr_in server, client;
	struct hostent *hp;
	char packet[512];
	char greeting[2];
	char ACK[6];
	int headerSize = 33;
	int counter = 0;


	FILE *file = fopen("result.txt", "w");
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sd < 0){
		cerr << "ERROR opening socket" << endl;
  }
  
	server.sin_family = AF_INET;
	server.sin_port = htons(8080);
	hp = gethostbyname(argv[1]);
	bcopy(hp->h_addr,&(server.sin_addr),hp->h_length);

	socklen_t clen = sizeof(client);
	socklen_t slen = sizeof(server);

	double p;
	double p2;
	cout << "Enter gremlin probabilies seperated by a space: ";
	cin >> p >> p2;
	cout << "\r\n";
	
	sendto(sd, "GET TestFile.html HTTP/1.0", 26, 0, (struct sockaddr *) &server, slen);
	
	cout << "ready to receive" << endl;




	char buffer[51235];

	//receive loop
	for(;;){
		n = recvfrom(sd, packet, 518, 0, (struct sockaddr*) &server, &clen);

    //cout << packet << endl;

		if (n==1){
			break;
		}

		if(packet[0]=='\0'){
			break;
		}

		char newPacket[512-headerSize];

		int packet_Num = stoi(string(1,packet[17])+string(1,packet[18])+packet[19]);
		int checkSum = stoi(string(1,packet[32])+string(1,packet[33])+string(1,packet[34])+string(1,packet[35])+string(1,packet[36]));



		for(int s = 0; s < 512-headerSize; s++){
			newPacket[s] = packet[s+headerSize+6];
		}


    //cout << "new Packet = " << newPacket << endl;


		gremlin(newPacket, p, p2);


		int sum = 0;
    if(packet_Num != 106){
      for(int k = 0; k < sizeof(newPacket); k++){
        sum += newPacket[k];
      }
    }
    else{
      for(int k = 0; k < sizeof(newPacket)-17; k++){
        //cout << "new packet[k] = " << newPacket[k] << endl;
        sum += newPacket[k];
      }
    }


		if(newPacket[1] == 0){
			cout << "packet " << packet_Num << " was lost" << endl;
			continue;
		}
		else if(sum != checkSum){
			cout << "error in packet: " << packet_Num << endl;
      string ACKString = "NAK" + to_string(packet_Num);
			copy(ACKString.begin(), ACKString.end(), ACK);
			ACK[ACKString.size()] = '\0';
		}
		else{
			cout << "Packet " << packet_Num << " Received"<< endl;
			string ACKString = "ACK" + to_string(packet_Num);
			copy(ACKString.begin(), ACKString.end(), ACK);
			ACK[ACKString.size()] = '\0';

      fputs(newPacket, file);
      
      counter++;
    }
    //cout << ACK << endl;
    
    sendto(sd, ACK, sizeof(ACK), 0, (struct sockaddr *) &server, sizeof(server));
    
	}
	//end of loop



	printf("Finished Recieving Packets\r\n");
	
	close(sd);
	fclose(file);
	return 0;
}