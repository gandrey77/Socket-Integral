
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>


/* Defines the server port */
#define PORT 5000

/* Sockets buffers length */
#define LEN 64


/* Server address */
#define SERVER_ADDR "127.0.0.1"



double Integral_Calculate( double intervalo, double discretization){
    
    double total = 0;
    double point1 = 0;
    double point2 = 0;


    point1 = sqrt(pow(100, 2) - pow(intervalo,2));
    point2 = sqrt(pow(100, 2) - pow((intervalo + discretization), 2));
       
    total = (point1 + point2)*discretization/2;

    return total;
}




/*
 * Main execution of the client program of our simple protocol
 */
int main(int argc, char *argv[]) {

    /* Server socket */
    struct sockaddr_in server;
    /* Client file descriptor for the local socket */
    int sockfd;

    int len = sizeof(server);
    int slen;

    /* Receive buffer */
    char buffer_in[LEN];
    /* Send buffer */
    char buffer_out[LEN];


    fprintf(stdout, "Starting Slave ...\n");


    /*
     * Creates a socket for the client
     */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error on slave socket creation:");
        return EXIT_FAILURE;
    }


    /* Defines the connection properties */
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    
    memset(server.sin_zero, 0x0, 8);


    /* Tries to connect to the server */
    if (connect(sockfd, (struct sockaddr*) &server, len) == -1) {
        perror("Can't connect to server");
        return EXIT_FAILURE;
    }

    double intervalo = 0;
    double discretization = 0;
    double resultado = 0;
    
    /* Receives the presentation message from the server */
    if ((slen = recv(sockfd, buffer_in, LEN, 0)) > 0) {
        
         sscanf(buffer_in, "%lf", &discretization);
    }

    strcpy(buffer_out, "ready");
    send(sockfd, buffer_out, strlen(buffer_out), 0);

/*

    /*
     * Commuicate with the server until the exit message come
     */
    while (true) {

        /* Zeroing the buffers */
        memset(buffer_in, 0x0, LEN);
        memset(buffer_out, 0x0, LEN);

        /* Receives an answer from the server */
        slen = recv(sockfd, buffer_in, LEN, 0);
        
        sscanf(buffer_in, "%lf", &intervalo);
        resultado = Integral_Calculate(intervalo, discretization);

            
        
        /* 'bye' message finishes the connection */
        if(strcmp(buffer_in, "bye") == 0)
            break;
        
        sprintf(buffer_out, "%lf", resultado);

        /* Sends the read message to the server through the socket */
        send(sockfd, buffer_out, strlen(buffer_out), 0);


    }


    /* Close the connection whith the server */
    close(sockfd);

    //fprintf(stdout, "\nConnection closed\n\n");

    return 0;
}   