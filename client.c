#include "sharedcode2.h"     /* for close() */

#define RCVBUFSIZE 32   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */
int mainMenu();

int main(int argc, char *argv[])
{
	int reqType, articleCount = 0;
	struct ClientMessage cm;
	struct ServerMessage sm;
	
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short echoServPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */   
    int totalBytesRcvd;   /* Bytes read in single recv()
                                        and total bytes read */

    if ((argc < 2) || (argc > 3))    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Echo Port>\n",
               argv[0]);
       exit(1);
    }

    servIP = argv[1];             /* First arg: server IP address (dotted quad) */    

    if (argc == 3)
        echoServPort = atoi(argv[2]); /* Use given port, if any */
    else
        echoServPort = 7;  /* 7 is the well-known port for the echo service */

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port        = htons(echoServPort); /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");

    /* Receive the same string back from the server */
    totalBytesRcvd = 0;
    printf("Received: ");                /* Setup to print the echoed string */
    while (TRUE)
    {
		//runs main menu
		reqType = mainMenu();
		
		
		//users choice
		if(reqType == 1){
			
			//send request type to server
			cm.requestType = Request;
			if(send(sock, &cm, sizeof(cm), 0) != sizeof(cm)){
				DieWithError("Request send() failed");
			}
			printf("Message sent\n");
			
			//recieves number of articles to be recieved
			if ( recv(sock, &sm, sizeof(sm), 0) <= 0){
				DieWithError("recv() failed or connection closed prematurely");
			}
			
			articleCount = sm.numOfArticles;
			
			int i;
			for(i = 0; i < articleCount; i++){
				
				//recieves character count to be sent by server
				if (recv(sock, &sm, sizeof(sm), 0) <= 0){
						DieWithError("recv() failed or connection closed prematurely");
					}
				
				printf("Article #%d:\n\n",i+1 );
				
				//recieves one character at a time from server
				int j;
				for(j = 1; j < sm.titlelen; j++){
					if (recv(sock, &sm, sizeof(sm), 0) <= 0){
						DieWithError("recv() failed or connection closed prematurely");
					}
					printf("%c", sm.ch);
				}
				printf("\n");
				for(j = 0; j < sm.textlen; j++){
					if (recv(sock, &sm, sizeof(sm), 0) <= 0){
						DieWithError("recv() failed or connection closed prematurely");
					}
					printf("%c", sm.ch);
				}
				printf("\n\n");
				
			}
			
		}
		else if(reqType == 2){
			
			//user prompt for keyword
			cm.requestType = Search;
			printf("Enter your keyword: ");
			scanf("%s", &cm.keyword);
			printf("Showing all articles matching: %s\n\n", cm.keyword);
			
			//sends keyword to server
			if(send(sock, &cm, sizeof(cm), 0) != sizeof(cm)){
				DieWithError("Request send() failed");
			}
			
			//recieves number of articles the server will send
			if (recv(sock, &sm, sizeof(sm), 0) <= 0){
				DieWithError("recv() failed or connection closed prematurely");
			}
			
			
			//recieves one character at a time for each article sent by server
			int i;
			for(i = 0; i < sm.numOfArticles; i++){
				
				if (recv(sock, &sm, sizeof(sm), 0) <= 0){
					DieWithError("recv() failed or connection closed prematurely");
				}
				
				printf("Article #%d:\n\n",i+1 );
				int j;
				for(j = 0; j < sm.textlen; j++){
					
					if (recv(sock, &sm, sizeof(sm), 0) <= 0){
						DieWithError("recv() failed or connection closed prematurely");
					}
					printf("%c", sm.ch);
				}
				printf("\n");
				
			}	
			
		}
		else if(reqType == 3){
			
			//close connection
			cm.requestType = Quit;
			if(send(sock, &cm, sizeof(cm), 0) != sizeof(cm)){
				DieWithError("Request send() failed");
			}
			break;
		}
		else{
			printf("Invalid selection\n");
		}
		
    }

    printf("\n");    /* Print a final linefeed */
	
	//close connection
    close(sock);
    exit(0);
}

int mainMenu(){
	int choice;
	char c;
	
	//propmts user for input
	printf("\nPlease choose an option: \n");
	printf("1. Request today's news\n");
	printf("2. Search today's news\n");
	printf("3. Quit\n");
	
	//reads input, and returns result 
	scanf("%d", &choice);
	while ( (c = getchar()) != EOF && c != '\n') { }
	
	return choice;
}
