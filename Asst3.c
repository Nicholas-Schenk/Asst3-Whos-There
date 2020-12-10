#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <ctype.h>
//jokeList is a struct to store jokes from a file when provided
struct jokeList{
	int length;	//used to keep track of length of list so that random num can be generated in correct range. only set for head of list.
	char *setup;
	char *punchline;
	int setup_length;
	struct jokeList * next;

};
struct jokeList* file_read(char* file_name){
	struct jokeList *ret = NULL;
	struct jokeList *start = NULL;
	FILE *fp;
	fp = fopen(file_name, "r");
	if(fp == NULL){ //file couldn't be opened
		printf("File not found.\n");
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if(size ==0){ // length of file is 0
		printf("File was empty or unreadable.\n");
                return NULL;
        }
        char* file_string = malloc(sizeof(char)*size);
        while(fgets(file_string, size, fp)!=NULL){
		file_string[strlen(file_string)-1] = '\0';
		if(ret == NULL){ // if head of list is empty, make the head
			ret = malloc(sizeof(struct jokeList));
			start = ret;
			ret->length = 1;
			ret->next = NULL;
			ret->setup = malloc(sizeof(char)*(strlen(file_string)+1));
			memcpy(ret->setup, file_string, strlen(file_string));
			ret->setup[strlen(file_string)] = '\0'; //remove newline
			ret->setup_length = strlen(ret->setup);
			if(fgets(file_string, size, fp)!=NULL){
				ret->punchline = malloc(sizeof(char)*(strlen(file_string)+1));
				memcpy(ret->punchline, file_string, strlen(file_string)-1);
				ret->punchline[strlen(file_string)] = '\0'; //remove newline
			}else{
				printf("File formatting was bad.\n");
				return 0;
			}	
		}else{
			ret->next = malloc(sizeof(struct jokeList));
			ret = ret->next;
			ret->next = NULL;
			ret->setup = malloc(sizeof(char)*(strlen(file_string)+1));
			memcpy(ret->setup, file_string, strlen(file_string));
			ret->setup[strlen(file_string)] = '\0';
			ret->setup_length = strlen(ret->setup);
			if(fgets(file_string, size, fp)!=NULL){
				ret->punchline = malloc(sizeof(char)*(strlen(file_string)+1));
				memcpy(ret->punchline, file_string, strlen(file_string));
				ret->punchline[strlen(file_string)-1] = '\0';
				start->length++; // keep track of how many jokes there are by adding 1 to start->length
			}else{
				printf("File formatting was bad.\n");
				return 0;
			}	



		}
		fgets(file_string, size, fp);		
	}
	fclose(fp);
	return start;


}
int main(int argc, char** argv){
	struct jokeList *first, *iterate;
	int i = 1;
	if(argc < 2 || argc > 3){
		printf("Server should either be invoked as ./server PORT# or ./server PORT# JOKE_FILE\n");
		return 0;
	}
	if(argc == 3){
		first = file_read(argv[2]);
		if(first == NULL){
			printf("File was empty/unreadable.\n"); // if file can't be used(wrong format, can't be opened, etc.) just return
			return 0;
		}
	}
	int sockfd,  new_socket, valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	//Joke that will always be used if user does not specify useable joke file.
	char * start = "REG|13|Knock, knock.|";
	char * setup = "REG|4|Who.|";
	int digits = 1;
	char * punchline = "REG|30|I didn't know you were an owl!|";
	char* ret1 = "Who's there?"; // first message received should always equal this(with correct formatting of course)
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0))==0){
		printf("Error: socket creation failed\n");
		return 0;
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(atoi(argv[1]));
	if(bind(sockfd, (struct sockaddr*)&address, sizeof(address))<0){
		printf("Error: socket bind failed\n");
		return 0;
	}
	if(listen(sockfd, 3)<0){
		printf("Error: listen failed\n");
		return 0;
	}
	while(1){
	if(argc == 3){// we were given a joke file, so we need to get a random joke from that to send to the client
		i = 1;
		int num = 0;
		srand(time(0));
		num = (rand()%(first->length))+1; // generate number between 1 and length of list.
		iterate = first;
		while(i< num){   // go to jokeList object that is at the position of the random num selected
			iterate = iterate->next;
			i++;
		}
		//Joke file contains strings, not formatted messages. so we need to format it:
		//format setup 
		char* temp_setup = malloc(sizeof(char)*(strlen(iterate->setup)*2 +6));
		strcpy(temp_setup, "REG|");
		char* temp_length = malloc(sizeof(char)*strlen(iterate->setup));
		sprintf(temp_length, "%ld", strlen(iterate->setup)); 
		strcat(temp_setup, temp_length);
		strcat(temp_setup, "|");
		strcat(temp_setup, iterate->setup);
		strcat(temp_setup, "|");
		strcat(temp_setup, "\0");
		setup = temp_setup;


		//format punchline
		char* temp_punchline = malloc(sizeof(char)*(strlen(iterate->punchline)*2 +6));
		strcpy(temp_punchline, "REG|");
		char* temp_length2 = malloc(sizeof(char)*strlen(iterate->punchline));
		sprintf(temp_length2, "%ld", strlen(iterate->punchline)); 
		strcat(temp_punchline, temp_length2);
		strcat(temp_punchline, "|");
		strcat(temp_punchline, iterate->punchline);
		strcat(temp_punchline, "|");
		strcat(temp_punchline, "\0");
		punchline = temp_punchline;

		
	}
	if((new_socket = accept(sockfd, (struct sockaddr*)&address,(socklen_t*)&addrlen))<0){
		printf("Error: couldn't accept\n");	  
	     	return 0;
	}
	valread= send(new_socket,start,strlen(start), 0);
	int ret = -1; // keeps track of if there are errors in messages
	valread = read(new_socket, buffer, 4);
	char* temp = malloc(sizeof(char)*20); //freed
	char* num = malloc(sizeof(char)*3); //freed
	memcpy(temp, buffer, valread);
	while(valread < 4){ // if we got a chunk smaller than 4, loop through until we do
		int myvar = read(new_socket, buffer, 1);
		buffer[1] = '\0';
		valread += myvar;
		strcat(temp, buffer);
		if(buffer[0] == '|'){
			break;
		}
	}
	temp[valread] = '\0';
	if(strcmp(temp, "REG|")==0){
		valread = read(new_socket, buffer, 1);
		num[0] = buffer[0];
		if(num[0] == '|'){ // no length field
			ret = 1;
		}else{	
			valread = read(new_socket, buffer, 1);
			num[1] = buffer[0]; //first message should always have length = 12
		
		if(!(isdigit(num[0]))){ //if length isn't numerical, formatting error
			ret = 1;
		}else{	
			num[2] = '\0';
			int the_length = atoi(num);		
			if(the_length != 12){
				ret = 2; // length was wrong(either formatted wrong or just the wrong number)
			}
		}
		if(ret == -1){ //only want to read next if no errors already found.
			valread = read(new_socket, buffer, 1);	
		}
		}
		if(buffer[0] != '|' && ret ==-1){
			if(isdigit(buffer[0])){ //length had at least 3 digits, length error
				ret = 2;
			}else{ // length contained some other char where | should be
				ret = 1;
			}
			
		}else if(ret == -1){ // if we haven't found an error yet
			int z = 0;
			char* temp2 = malloc(sizeof(char)*13); //freed
			while(z < 12){ // checking length takes precedence over content, so check for bars
				valread = read(new_socket, buffer, 1);
				if(buffer[0] == '|'){
					ret = 2; //not enough characters
					break;
				}
				temp2[z] = buffer[0];
				z++;
			}
			temp2[z] = '\0'; // to guarantee string functions work
			if(ret!=2){
				valread = read(new_socket, buffer, 1);
				if(buffer[0] != '|'){
					ret = 2; //  too many characters sent
				}				
			}	
			if(ret == -1){
				z=0;
				while(z< 12){	// now we can check if content is correct
					if(temp2[z] != ret1[z]){ //characters that should match didn't
						ret = 3; // therefore message content is wrong
					}
					z++;	
				}	
			}
			free(temp2);			
		}
	
	}else if(strcmp(temp, "ERR|")==0){ // code to print error message if we were sent one, assumes error is formatted right
		int s=0, barcount=0;
		while(s<15&&barcount<1){
			read(new_socket, buffer, 1);
			buffer[1] = '\0';
			if(buffer[0]=='|'){
				barcount++;
			}
			strcat(temp, buffer);
			s++;
		}
		printf("Received the error message: %s from the client when we sent them: %s\n\n", temp, start);
		close(new_socket);
		continue;
	}else{
		//code was neither ERR nor REG (or was missing |, but both are the same error code);
		ret = 1;
	}

	if(ret == 1){  //format
		setup = "ERR|M1FT|";
		printf("The client sent us a message with the following error: %s\n\n", setup);
	}else if(ret == 2){ //length
		setup = "ERR|M1LN|";
		printf("The client sent us a message with the following error: %s\n\n", setup);
	}else if(ret == 3){ //content
		setup = "ERR|M1CT|";
		printf("The client sent us a message with the following error: %s\n\n", setup);
	}else{
		printf("M1 was correctly formatted.\n");
	}
	free(temp);
	free(num);
	valread=send(new_socket,setup,strlen(setup), 0);
	if(ret != -1){ // we recieved a message that was wrong, close the connection
		close(new_socket);
		continue;
	}
	valread = read(new_socket, buffer, 4);
	char* tem = malloc(sizeof(char)*20);//freed
	memcpy(tem, buffer, valread);
	while(valread < 4){ // if we didn't get 4 bytes (message code + |) in one, keep reading until we have
		int myvar = read(new_socket, buffer, 1);
		buffer[1] = '\0'; //ignore characters after first
		valread += myvar;
		strcat(tem, buffer);
	}
	tem[valread] = '\0';
	int digits_in_length = 0;//track #digits in the length
	int k = 1;
	int the_setup_length = 0;//track length of setup
	//finding out how many digits the length of the message will be. Only need to do if given joke file as length is known value otherwise.
	if(argc==3){
		while(k <(iterate->setup_length+5)){ //string we get should have length (punchline + " who?")
			digits_in_length++;
			k = k*10; 
		}
		the_setup_length = iterate->setup_length;
	}else{
		digits_in_length = 1;
		the_setup_length = 4;
	}
	char* temp2 = malloc(sizeof(char)*(digits_in_length+1));// will store length field from message freed
	if(strcmp(tem, "REG|")==0){
		k=0;
		while(k < digits_in_length){ 
			valread = read(new_socket, buffer, 1);
			temp2[k] = buffer[0];
			if(temp2[k] == '|'){
				if(k ==0){
					ret =1; // no length, format error
				}else{
					ret = 2; // length value was too few digits
				}
				break;	
			}else if(!isdigit(temp2[k])){
				ret = 1; // length field contain non numerical characters
				break;
			}
			k++;	
		}
		if(ret == -1){ // if we haven't found an error yet
			temp2[k] = '\0';
			valread = read(new_socket, buffer, 1);
			if(buffer[0] != '|'){
				if(isdigit(buffer[0])){
					//at least one extra digit in length.
					ret = 2;
				}else{
					ret = 1; // missing |
				}
				
			}else{
				int the_length = atoi(temp2);
				if(the_length != (the_setup_length+5)){ 
					ret = 2; // length field was wrong
				}else{
					int z = digits_in_length+5; // start of the real response
					char* who = ", who?"; // needs to have added to end of setup in the clients response
					while(z < (the_setup_length+digits_in_length+10)){
						valread = read(new_socket, buffer, 1);
						if(buffer[0] == '|'){
							ret = 2; //message was too short
							break;
						}
						if(z < 5+digits_in_length+the_setup_length-1){
							if(buffer[0] != setup[z-digits_in_length+digits]){
								ret = 3; // first part of response was not the same as the setup
							}
						}else{
							if(buffer[0] != who[z-digits_in_length-4-the_setup_length]){
								ret = 3; // second part of response was not ", who?"	
							}
						}
						z++;	
					}
					if(ret != 2){ // length error takes precedence over content error. if no length error up to this point, check for one
						valread = read(new_socket, buffer, 1);
						if(buffer[0] != '|'){
							ret = 2; // had characters where | should have been found
						}
					}
		
				}
			}
		}
	}else if(strcmp(tem, "ERR|")==0){	// prints error message out if we were sent one
		int s=0, barcount=0;
		while(s<15&&barcount<1){
			read(new_socket, buffer, 1);
			buffer[1] = '\0';
			if(buffer[0]=='|'){
				barcount++;
			}
			strcat(tem, buffer);
			s++;
		}
		printf("Received the error message: %s from the client when we sent them: %s\n\n", tem, setup);
		close(new_socket);
		continue;
	}else{
		//message didn't even have starting code right.
		ret = 1;
	}
	if(ret == 1){ // format
		punchline = "ERR|M3FT|";
		printf("The client sent us a message with the following error: %s\n\n", punchline);
	}else if(ret == 2){ //length
		punchline = "ERR|M3LN|";
		printf("The client sent us a message with the following error: %s\n\n", punchline);
	}else if(ret == 3){ //content
		punchline = "ERR|M3CT|";
		printf("The client sent us a message with the following error: %s\n\n", punchline);
	}else{
		printf("M3 was formatted correctly.\n");
	}
	free(tem);
	free(temp2);
	valread=send(new_socket,punchline,strlen(punchline), 0);
	if(ret != -1){ // we were sent a bad message, close the connection
		close(new_socket);
		continue;
	}	
	valread = read(new_socket, buffer, 4);
	char* type = malloc(sizeof(char)*20); //freed
	memcpy(type, buffer, valread);
	while(valread < 4){
		int myvar = read(new_socket, buffer, 1);
		buffer[1] = '\0';
		valread += myvar;
		strcat(type, buffer);
	}
	type[valread] = '\0';
	int expression_digits=0; // the A/D/S can have a completely variable length, unlike other messages
	if(strcmp(type, "REG|")==0){	
		read(new_socket, buffer, 1);
		char* len_size = malloc(sizeof(char)*13); //freed
		while(buffer[0]!='|'){ // read until | to find number of digits in expression length
			if(!(isdigit(buffer[0])) || expression_digits > 12){  // if the length is greater than 12, i.e 1000000000000 or greater, give formatting error
				ret = 1;
				break;
			}
			len_size[expression_digits] = buffer[0];
			expression_digits++;
			read(new_socket, buffer, 1);
		}
		if(ret != 1){
			if(expression_digits == 0){
				//No length value
				ret = 1;
			}else{
				read(new_socket, buffer, 1);
				int messageLength = atoi(len_size);
				free(len_size);
				if(messageLength <= 0){
					//atoi failed or length is 0. formatting error regardless
					ret = 1;
				}else {
					i = 0;
					while(i < messageLength){// basically just go for messagelength unless we find a |, also check last char to be punctuation
						if(i == messageLength - 1){
							if(strchr(".?!", buffer[0]) == NULL){
								// trailing punctuation missing
								ret = 3;
							}
						}else if(buffer[0] == '|'){
							// "|" appeared before we should hit end of string.
							ret = 2;
						}
						i++;
						read(new_socket, buffer, 1);
						buffer[1] = '\0';
					}
					if(ret != 2){ //again, length error takes precedence over content, so if we had a content error or no error we go into this check
						if(buffer[0] != '|'){
							//too many characters were sent
							ret = 2;
						}
					}
				
				}
			}
		}
	}else if (strcmp(type, "ERR|")==0){	//print error message if we were sent one
		int s=0, barcount=0;
		while(s<15&&barcount<1){
			read(new_socket, buffer, 1);
			buffer[1] = '\0';
			if(buffer[0]=='|'){
				barcount++;
			}
			strcat(type, buffer);
			s++;
		}
		printf("Received the error message: %s from the client when we sent them: %s\n\n", type, punchline);
		close(new_socket);
		continue;
	}else{ // beginning did not have REG| or ERR|
		ret = 1;
	}
	free(type);
	if(ret == 1){
		punchline = "ERR|M5FT|";
		printf("The client sent us a message with the following error: %s\n\n", punchline);
	}else if(ret == 2){
		punchline = "ERR|M5LN|";
		printf("The client sent us a message with the following error: %s\n\n", punchline);
	}else if(ret == 3){
		punchline = "ERR|M5CT|";
		printf("The client sent us a message with the following error: %s\n\n", punchline);
	}else{// else, format was right! we can exit now.
		printf("M5 was formatted correctly. Closing connection.\n\n");
		close(new_socket);
		continue;
	}	
	// send message after M5 only if error was present.
	send(new_socket,punchline,strlen(punchline), 0);
	
	close(new_socket);
	}
	return 0;



}
