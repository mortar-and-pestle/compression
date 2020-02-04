#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void fireman(int)
{
    //Referenced cs.cmu.edu
    while(waitpid(-1,NULL,WNOHANG) > 0);
        //std::cout << "A child has ended\n";

}

void generateBinaryStr(char str [], char binChar)
{
    for (size_t i = 0; i < strlen(str); ++i)
    {
        if (str[i] == char(binChar))//static_cast<char>
        {
            str[i] = '1';
        }
        else
        {
            str[i] = '0';
        }
    }
}


char getSymbol(char combinedStr[])
{
    char symbol;

    symbol = combinedStr[strlen(combinedStr)-1];
    combinedStr[strlen(combinedStr) -1] = '\0';

    return symbol;

}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0)
        error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");

     listen(sockfd,50); //
     clilen = sizeof(cli_addr);
     signal(SIGCHLD,fireman);

     while(1)
     {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (socklen_t *)&clilen);
         pid_t pid;

         if((pid = fork())==0)
         {
             if (newsockfd < 0)
                  error("ERROR on accept");

             bzero(buffer,256);
             n = read(newsockfd,buffer,255);

             if (n < 0) error("ERROR reading from socket");

             char symbol = getSymbol(buffer);
             generateBinaryStr(buffer,symbol);

             n = write(newsockfd,buffer,256);

             if (n < 0) error("ERROR writing to socket");

             close(newsockfd);
             _exit(0);
         }

     }

     return 0;
}
