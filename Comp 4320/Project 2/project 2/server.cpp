#include "unp.h"
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>


using namespace std;


int main(){
	int n, m;
  int sd;
	struct sockaddr_in server,client;
  fd_set readfds, masterfds;
  struct timeval timeout;

	char request[26];
	char ACK[6];
	char result[513];
  char lastResult[513];
	char* fileName;


	sd = socket(AF_INET, SOCK_DGRAM, 0);

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(8080);
	
	bind(sd, (struct sockaddr *)&server, sizeof(server));

	socklen_t clen = sizeof(client);
	socklen_t slen = sizeof(server);

  timeout.tv_sec = 0;
  timeout.tv_usec = 55;


	for(;;){
		n = recvfrom(sd, request, sizeof(request), 0, (struct sockaddr*) &client, &clen);
    cout << request << endl;

		if (request[0] != '\0'){
			printf("request received: %s\r\n", request);
		}
		break;
	}

  //set receive timeout
	int opt = setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&timeout,sizeof(struct timeval));
	

  //open File
	ifstream inputfile("TestFile.html", ios::binary);
	inputfile.seekg(0, ios::end);
	int content_length = inputfile.tellg();
	inputfile.seekg(0, ios::beg);
	inputfile.close();

	cout << "sending packets\r\n";

  //put all file contents into char array
	char ch;
  char wholeFile[content_length];
	fstream inputFile("TestFile.html", fstream::in);
  int fileCounter = 0;
  while (inputFile >> noskipws >> ch){
    wholeFile[fileCounter] = ch;
    fileCounter ++;
  }
  



  int sequenceNum = 0;
	int headerSize = 33;
  int lastSuccessful = 0;
  int toRead = 1; //used so the send loop only loops once when resending a particular packet 
  int index = 0;

	//send loop
	for(int i = 0; i < floor(content_length / (512-headerSize)); i++){
    int checksum = 0;
    char packetChar[512];
    string snString;
    string csString;

    for(int j = 0; j < 512-headerSize; j++){
      packetChar[j] = wholeFile[j+(i*(512-headerSize))];
      checksum += wholeFile[j+(i*(512-headerSize))];
      index = j+(i*(512-headerSize));
    }
    

    string packetString = string(packetChar);

    if(sequenceNum < 10){
      snString = "sequence Number: " + to_string(0) + to_string(0) + to_string(sequenceNum);
    }
    else if(sequenceNum < 100){
      snString = "sequence Number: " + to_string(0) + to_string(sequenceNum);
    }
    else{
      snString = "sequence Number: " + to_string(sequenceNum);
    }

    csString = "checksum: "+ to_string(checksum);

    string smallHeader =  snString;
    smallHeader.append("\r\n");
    smallHeader.append(csString);
    smallHeader.append("\r\n");

    string resultString;

    resultString = smallHeader + packetString;

    copy(resultString.begin(), resultString.end(), result);
    result[resultString.size()] = '\0';

    sendto(sd, result, 518, 0, (struct sockaddr *) &client, sizeof(client));//send packet
    cout << "packet " << sequenceNum << " sent" << endl;

    //cout << result << endl;
    sequenceNum++;


    for(;;){
		  //recieve ACK
		  m = recvfrom(sd, ACK, sizeof(ACK), 0, (struct sockaddr*) &client, &clen);

      if(m == -1){
        cout << "falied to receive ACK or NAK " << endl << "resending packet" << endl;
        sendto(sd, result, 518, 0, (struct sockaddr *) &client, sizeof(client));
        continue;
      }
      else if (ACK[0] == 'N'){
        cout << "packet error" << endl << "resending packet" << endl;
        sendto(sd, result, 518, 0, (struct sockaddr *) &client, sizeof(client));
        continue;
      }

      int ackSN = stoi(string(1,ACK[3])+string(1,ACK[4])+ACK[5]);//sequence number attatched to ack

      if(ACK[0] == 'A'){
        cout << "ACK " << ackSN << " received" << endl;
        break;
      }
    }
	}
	//end of for loop




	//last packet
	char lastPacket[512];
  char lastPacketData [512-headerSize];
	int checksum = 0;
  int lastCounter = 0;
	for(int z = index+1; z < content_length; z++){
		lastPacketData[lastCounter] = wholeFile[z];
		checksum += wholeFile[z];
    lastCounter++;
	}

	//cout << lastCounter << endl;

	string finalHeader = "sequence Number: " + to_string(sequenceNum) + "\r\nchecksum: "+ to_string(checksum) + "\r\n";
	if(checksum < 10000){
		finalHeader = "sequence Number: " + to_string(sequenceNum) + "\r\nchecksum: "+ to_string(0)+ to_string(checksum)+"\r\n";
	}

	finalHeader.append(lastPacketData);

	copy(finalHeader.begin(), finalHeader.end(), lastPacket);
	lastPacket[finalHeader.size()] = '\0';

  //cout << lastPacket << endl;

	sendto(sd, lastPacket, sizeof(lastPacket), 0, (struct sockaddr *) &client, clen);

  for(;;){
    //recieve ACK
    m = recvfrom(sd, ACK, sizeof(ACK), 0, (struct sockaddr*) &client, &clen);

    if(m == -1){
      cout << "falied to receive ACK or NAK" << endl << "resending packet" << endl;
      sendto(sd, lastPacket, 518, 0, (struct sockaddr *) &client, sizeof(client));
      continue;
    }
    else if(ACK[0] == 'N'){
      cout << "packet error" << endl << "resending packet" << endl;
      sendto(sd, lastPacket, 518, 0, (struct sockaddr *) &client, sizeof(client));
      continue;
    }

    int ackSN = stoi(string(1,ACK[3])+string(1,ACK[4])+ACK[5]);//sequence number attatched to ack

    if(ACK[0] == 'A'){
      cout << "ACK " << ackSN << " received" << endl;
      break;
    }
  }
  
	inputFile.close();

	sendto(sd, "\0", 1, 0, (struct sockaddr *) &client, sizeof(client));



	close(sd);
	return 0;
}