#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <ctype.h>
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
	if(fp == NULL){
		printf("File not found.\n");
		return 0;
	}
	fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if(size ==0){
		printf("File was empty or unreadable.\n");
                return 0;
        }
        char* file_string = malloc(sizeof(char)*size);
	//int i = 0;
        while(fgets(file_string, size, fp)!=NULL){
		file_string[strlen(file_string)-1] = '\0';
		if(ret == NULL){
			ret = malloc(sizeof(struct jokeList));
			start = ret;
			ret->length = 1;
			ret->next = NULL;
			ret->setup = malloc(sizeof(char)*(strlen(file_string)+1));
			memcpy(ret->setup, file_string, strlen(file_string));
			ret->setup[strlen(file_string)] = '\0';
			if(ret->setup[strlen(file_string)-1] == '\0'){
				ret->setup[strlen(file_string)-1] = '\0';
			}
			ret->setup_length = strlen(ret->setup);
		//	printf("setup: %s\n", ret->setup);
			if(fgets(file_string, size, fp)!=NULL){
				ret->punchline = malloc(sizeof(char)*(strlen(file_string)+1));
				memcpy(ret->punchline, file_string, strlen(file_string)-1);
				ret->punchline[strlen(file_string)] = '\0';
				if(ret->punchline[strlen(file_string)-1] == '\0'){
					ret->punchline[strlen(file_string)-1] = '\0';
				}
				//ret->punchline_length = strlen(ret->punchline);
		//		printf("punchline: %s\n", ret->punchline);
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
		//	printf("setup: %s\n", ret->setup);
			ret->setup_length = strlen(ret->setup);
			if(fgets(file_string, size, fp)!=NULL){
				ret->punchline = malloc(sizeof(char)*(strlen(file_string)+1));
				memcpy(ret->punchline, file_string, strlen(file_string));
				ret->punchline[strlen(file_string)-1] = '\0';
		//		printf("punchline: %s\n", ret->punchline);
				//ret->punchline_length = strlen(ret->punchline);
				start->length++;
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
			printf("File was empty/unreadable.\n");
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
	if(argc == 3){
		i = 1;
		int num = 0;
	//	srand(time(0));
		num = (rand()%(first->length))+1; // generate number between 1 and length of list.
		printf("NUM: %d\n", num);
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
		//printf("Message: %s\n", temp_setup);
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
		//printf("Message: %s\n", temp_punchline);
		punchline = temp_punchline;

		
	}
	if((new_socket = accept(sockfd, (struct sockaddr*)&address,(socklen_t*)&addrlen))<0){
		printf("Error: couldn't accept\n");	  
	     	return 0;
	}
	valread= send(new_socket,start,strlen(start), 0);
	/*valread = read(new_socket, buffer, 1024);
	buffer[valread] = '\0';
	*/
	int ret = -1;
	valread = read(new_socket, buffer, 4);
	char* temp = malloc(sizeof(char)*5);
	memcpy(temp, buffer, valread);
	while(valread < 4){
		int myvar = read(new_socket, buffer, 1);
		buffer[1] = '\0';
		valread += myvar;
		strcat(temp, buffer);
	}
	temp[valread] = '\0';
	if(strcmp(temp, "REG|")==0){
		valread = read(new_socket, buffer, 1);
		temp[0] = buffer[0];	
		valread = read(new_socket, buffer, 1);
		temp[1] = buffer[0];	
		valread = read(new_socket, buffer, 1); //we know that this message should always have length 12 aka a number with 2 digits
		if(buffer[0] != '|'){
			if(isdigit(buffer[0])){ //length had at least 3 digits, error
				ret = 2;
			}else{ // length contained some other char where | should be
				ret = 1;
			}
			
		}else{
			temp[2] = '\0';
			int the_length = atoi(temp);
			if(the_length != 12){
				ret = 2; // length was wrong
			}else{
				int z = 0;
				//char* temp2 = malloc(sizeof(char)*12);
				while(z < 12){
					valread = read(new_socket, buffer, 1);
					if(buffer[0] != ret1[z]){ //characters that should match didn't
						ret = 3; // therefore message content is wrong
					}
					z++;	
				}
				if(ret != 3){
					valread = read(new_socket, buffer, 1);
					if(buffer[0] != '|'){
						ret = 1; // missing last |
					}else{
						ret = 0; //formatted right
					}
				}
	
			}
		}
	}else if(strcmp(temp, "ERR|")==0){

	}else{
		printf("formatting error at beginning");
		ret = 1;
	}

	free(temp);
	if(ret == 1){  //format
		setup = "ERR|4|M1FT|";
	}else if(ret == 2){ //length
		setup = "ERR|4|M1LN|";
	}else if(ret == 3){ //content
		setup = "ERR|4|M1CT|";
	}
	valread=send(new_socket,setup,strlen(setup), 0);
	ret = -1;
	valread = read(new_socket, buffer, 4);
	char* tem = malloc(sizeof(char)*5);
	memcpy(tem, buffer, valread);
	while(valread < 4){ // if we didn't get 4 bytes (message code + |) in one, keep reading until we have
		int myvar = read(new_socket, buffer, 1);
		buffer[1] = '\0'; //ignore characters after first
		valread += myvar;
		strcat(tem, buffer);
	}
	tem[valread] = '\0';
	int digits_in_length = 0;
	int k = 1;
	//finding out how many digits the length of the message will be.
	while(k <(iterate->setup_length+5)){ //string we get should have length (punchline + " who?")
		digits_in_length++;
		k = k*10; 
	}
	char* temp2 = malloc(sizeof(char)*(digits_in_length+1));// will store length field from message
	if(strcmp(tem, "REG|")==0){
		k=0;
		while(k < digits_in_length){ 
			valread = read(new_socket, buffer, 1);
			temp2[k] = buffer[0];
			k++;	
		}
		temp2[k] = '\0';
		valread = read(new_socket, buffer, 1);
		if(buffer[0] != '|'){
			if(isdigit(buffer[0])){
				//at least one extra digit in length.
				ret = 2;
			}else{
				ret = 1; // some sort of formatting error
			}
			
		}else{
			int the_length = atoi(temp2);
			if(the_length != (iterate->setup_length+5)){
				ret = 2; // length field was wrong
			}else{
				int z = digits_in_length+5; // start of the real response
				char* who = ", who?"; // needs to have added to end of setup in the clients response
				while(z < (iterate->setup_length+digits_in_length+10)){
					valread = read(new_socket, buffer, 1);
					if(z < 5+digits_in_length+iterate->setup_length-1){
						if(buffer[0] != setup[z-digits_in_length+digits]){
							ret = 3; // first part of response was not the same as the setup
						}
					}else if(z>=iterate->setup_length+digits_in_length+4){
						if(buffer[0] != who[z-digits_in_length-4-iterate->setup_length]){
							ret = 3; // second part of response was not ", who?"
						}

					}
					z++;	
				}
				if(ret != 3){ // message was right, check for final |
					valread = read(new_socket, buffer, 1);
					if(buffer[0] != '|'){
						ret = 1; // missing final |
					}else{
						ret = 0; //formatted right!
					}
				}
	
			}
		}
	}else if(strcmp(temp, "ERR|")==0){
		printf("ERROR RECEIVED\n");
		return 0;
	}else{
		//message didn't even have starting code right.
		ret = 1;
	}
	if(ret == 1){ // format
		punchline = "ERR|4|M3FT|";
	}else if(ret == 2){ //length
		punchline = "ERR|4|M3LN|";
	}else if(ret == 3){ //content
		punchline = "ERR|4|M3CT|";
	}
	// else, no error!
	ret = -1;
	valread=send(new_socket,punchline,strlen(punchline), 0);
	valread = read(new_socket, buffer, 4);
	char* type = malloc(sizeof(char)*5);
	memcpy(type, buffer, valread);
	while(valread < 4){
		int myvar = read(new_socket, buffer, 1);
		buffer[1] = '\0';
		valread += myvar;
		strcat(type, buffer);
	}
	type[valread] = '\0';
	int expression_digits=0;
	if(strcmp(type, "REG|")==0){	
		read(new_socket, buffer, 1);
		char* len_size = malloc(sizeof(char)*13);
		while(buffer[0]!='|'){ // read until | to find length
			len_size[expression_digits] = buffer[0];
			expression_digits++;
			read(new_socket, buffer, 1);
		}
		if(expression_digits == 0){
			//No length value
			ret = 2;
		}else{
			read(new_socket, buffer, 1);
			int messageLength = atoi(len_size);
			free(len_size);
			if(messageLength < 0){
				//atoi failed. either length is 0 or formatting is wrong. either way, format error
				ret = 2;
			}else {
				i = 0;
				while(i < messageLength){
					if(i == messageLength - 1){
						if(strchr(".,?!", buffer[0]) == NULL){
							// trailing punctuation missing
							ret = 3;
						}
					}else if(buffer[0] == '|'){
						// "|" appeared before we should hit end of string.
						ret = 2;
					}
					i++;
					read(new_socket, buffer, 1);
				}
				if(ret == -1){
					if(buffer[0] != '|'){
						//NO CLOSING |
						ret = 1;
					}
				}
			
			}


		}
	}else if (strcmp(type, "ERR")==0){
		ret = 0;
		return 0;
	}else{
		ret = 1;
	}
	if(ret == 1){
		punchline = "ERR|4|M5FT|";
	}else if(ret == 2){
		punchline = "ERR|4|M5LN|";
	}else if(ret == 3){
		punchline = "ERR|4|M5CT|";
	}// else, format was right! we can exit now.
	close(new_socket);
	}
	return 0;



}
