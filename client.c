#include <stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <string.h>
#include <stdlib.h>
int main(int argc, char ** argv){
	int sock, valread;
	struct sockaddr_in serv_addr;
	char * start = "REG|12|Who's there?|";
	char * response = "REG|9|Who, who?|";
	char* disgust = "REG|4|Ugh.|";
	char buffer[1024] = {};
	if((sock = socket(AF_INET, SOCK_STREAM, 0))<0){
		printf("\n Socket creation error \n");
		return -1;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){
		printf("\nInvalid address/ not supported\n");
		return -1;
	}

	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))<0){
		printf("\n Connection failed\n");
		return -1;
	}

	
	read(sock, buffer, 1024);
	printf("%s\n", buffer);
	//start = "REG|||";
	send(sock, start, strlen(start), 0);
	valread = read(sock, buffer, 1024);
	buffer[valread] = '\0';
	printf("%s\n", buffer);
	// All this commented out code makes is respond correctly to variable jokes, i.e when we want to send the right response to whatever joke the server tells. It makes it much harder to test the servers error handling ability however.
	/*if(valread <= 0){
		return 0;
	}
	buffer[valread] = '\0';	
	printf("%s\n", buffer);
	char *temp_setup = malloc(sizeof(char)*(strlen(buffer)+7));
	strcpy(temp_setup, "REG|");
	int the_length = strlen(buffer)-7+5;
	int check = 0;
	if(strlen(buffer) > 18){
		check = 1;
		the_length--;
	}
	
	char* temp_length = malloc(sizeof(char)*5);
	sprintf(temp_length, "%d", the_length);
	//strcat(temp_setup, "TEN");
	strcat(temp_setup, temp_length);
	strcat(temp_setup, "|");
	char* mytemp = malloc(sizeof(char)*the_length);
	if(check == 1){
		memcpy(mytemp, &buffer[7], the_length-6);
	}else{
		memcpy(mytemp, &buffer[6], the_length-6);
	}
	strcat(temp_setup, mytemp);
	//strcat(temp_setup, mytemp);
	strcat(temp_setup, ", who?|");
	//printf("%s\n", temp_setup);
	//i
	//printf("%s\n", buffer);
	response = temp_setup;
	//response = "ERR|M2CT|";
	//printf("RESPONSE: %s\n", response);
	*/
	send(sock, response, strlen(response), 0);
	valread = read(sock, buffer, 1024);
	buffer[valread] = '\0';
	printf("%s\n", buffer);
	send(sock, disgust, strlen(disgust), 0);
	valread = read(sock, buffer, 1024);
	if(valread>0){
		buffer[valread] = '\0';
		printf("%s\n", buffer);
	}
	close(sock);

	return 0;	


}
