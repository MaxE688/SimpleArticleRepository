#include "sharedcode2.h"

#define MAXPENDING 5    /* Maximum outstanding connection requests */

void DieWithError(char *errorMessage);   /* TCP client handling function */
void handleTCPClient(int clntSocket, char *date);


typedef struct ArticleNode{
	char *title;
	char *text;
	struct ArticleNode *next;
};


int main(int argc, char *argv[])
{
	char *file;
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in servAddr; /* Local address */
    struct sockaddr_in clntAddr; /* Client address */
    unsigned short servPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */

    if (argc != 3)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port> <The days date>\n", argv[0]);
        exit(1);
    }

    servPort = atoi(argv[1]);  /* First arg:  local port */
	file = argv[2];

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(servPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");
	

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(clntAddr);
		
		
        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr,
                               &clntLen)) < 0)
            DieWithError("accept() failed");
		

        /* clntSock is connected to a client! */

        printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));

        handleTCPClient(clntSock, file);
    }
    /* NOT REACHED */
}

void handleTCPClient(int clntSocket, char *date)
{
   
	struct ClientMessage cm;
	struct ServerMessage sm;
	int numOfArticles, len;
	char  *buff, *buff2, *title, *text;
	FILE *fp;
	FILE *fp2;
	
	//Counts number of articles in the file
	fp = fopen(date,"r");
	numOfArticles = 0;
	while(!feof(fp)){
		
		fscanf(fp,"%s",&buff);
		if(strcmp(&buff, "#item") == 0){
			numOfArticles++;
		}
	}
	
	fclose(fp);
	
	
	while(TRUE){
		/* Receive message from client */
		if (recv(clntSocket, &cm, sizeof(cm), 0) < 0){
			DieWithError("recv() failed");
		}
		
		//request type from client
		if(cm.requestType == Request){
			printf("News request\n");
			
			//sends number of articles to the client
			sm.numOfArticles = numOfArticles;
			if (send(clntSocket, &sm, sizeof(sm), 0) != sizeof(sm)){ 
				DieWithError("send() failed");
			}
			
			
			fp = fopen(date,"r");
			fp2 = fopen(date,"r");
			
			while(!feof(fp)){
				
				//looks for "#item" tag
				fscanf(fp,"%s",&buff);
				fscanf(fp2,"%s",&buff2);
				
				//counts number of characters in the title
				if(strcmp(&buff, "#item") == 0){
					len = 0;
					
					while(fgetc(fp2) != '\n'){
						
						len++;	
						
						if(feof(fp2)){
							break;
						}
					}
				}
				
				//allocates space for the title
				title = (char *)malloc((len+1)*sizeof(char));
				
				//Gives the title from the file to the title variable
				int i;
				for(i = 0; i < len; i++){
					title[i] = fgetc(fp);
				}
				
				//appends null character to the end
				title[len] = '\0';
				
				
				//counts length of the article
				len = 0;
				while(fgetc(fp2) != '#'){
					len++;
					if(feof(fp2)){
						break;
					}
				}
				
				//allocates memory for the article text
				text = (char *)malloc((len+1)*sizeof(char));
				
				//reads article into program
				for(i = 0; i < len; i++){
					text[i] = fgetc(fp);
				}
				text[len] = '\0';
				
				
				//initializes length variables
				sm.titlelen = strlen(title);
				sm.textlen = strlen(text);
				//sends lengths to client to count messages
				if (send(clntSocket, &sm, sizeof(sm), 0) != sizeof(sm)){ 
					DieWithError("send() failed");
				}
				
				//sends title one character at a time
				for(i = 1; i < sm.titlelen; i++){
					sm.ch = title[i];
					if (send(clntSocket, &sm, sizeof(sm), 0) != sizeof(sm)){ 
						DieWithError("send() failed");
					}
				}
				//sends article one character at a time
				for(i = 0; i < sm.textlen; i++){
					sm.ch = text[i];
					if (send(clntSocket, &sm, sizeof(sm), 0) != sizeof(sm)){ 
						DieWithError("send() failed");
					}
				}
				
				
				//frees the memory to be reused
				free(text);
				free(title);
			}
			fclose(fp);
			fclose(fp2);
			
			
			
			
			
			
		}
		else if(cm.requestType == Search){
			
			int artNum = 0;
			int matches[15];
			int hasMatch = FALSE;
			
			printf("Search request\n");
			
			//will vary depending on matches
			sm.numOfArticles = 0;
			
			
			
			fp = fopen(date,"r");
			
			int i = 0;
			while(!feof(fp)){
				
				//looks for start of article
				fscanf(fp, "%s", &buff);
				if(strcmp(&buff, "#item") == 0){
					artNum++;
				}
				else if(strcmp(&buff, cm.keyword) == 0){
					
					//if match is found, checks to see if article has already matched
					int j; 
					for(j = 0; j < 15; j++){
						if(matches[j] == artNum){
							hasMatch = TRUE;
							break;
						}
					}
					
					//if article has not already matched it's added to the list of 
					//matched articles
					if(!hasMatch){
						matches[i] = artNum;
						i++;
					}
					hasMatch = FALSE;
					
				}
			}
			sm.numOfArticles = i+1;
			
			//sends number of matched articles
			if (send(clntSocket, &sm, sizeof(sm), 0) != sizeof(sm)){
				DieWithError("send() failed");
			}
			
			fclose(fp);
			
			
			
			fp = fopen(date,"r");
			fp2 = fopen(date,"r");
			
			i = 0;
			artNum = 0;
			while(!feof(fp2)){
				
				fscanf(fp,"%s",&buff);
				fscanf(fp2,"%s",&buff2);
				
				//looks for start of article
				if(strcmp(&buff, "#item") == 0){
					artNum++;
				}
				else if(artNum == matches[i]){
					
					//if article being read was matched then it is sent
					len = 0;
					while(fgetc(fp2) != '#'){
						len++;
						if(feof(fp2)){
							break;
						}
					}
					
					//length of text is sent to client to sync send, and recv function
					sm.textlen = len;
					
					if (send(clntSocket, &sm, sizeof(sm), 0) != sizeof(sm)){
						DieWithError("send() failed");
					}
					
					//sends article one character at a time
					int j;
					for(j = 0; j < len; j++){
						sm.ch = fgetc(fp);
						if (send(clntSocket, &sm, sizeof(sm), 0) != sizeof(sm)){
							DieWithError("send() failed");
						}
					}
					
					
				}
				
				
			}
			
			fclose(fp);
			fclose(fp2);
			
		}
		else if(cm.requestType == Quit){
			printf("Closing connection...\n");
			break;
		}
	
	
	}

    close(clntSocket);    /* Close client socket */
}


