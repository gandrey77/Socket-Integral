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

#define PORT 5000

#define LEN 64

#define SERVER_ADDR "127.0.0.1"

double calculateIntegral( double interval, double discretization){
    double total = 0;
    double point1 = 0;
    double point2 = 0;

    // Regra do trapezio
    point1 = sqrt(pow(100, 2) - pow(interval,2));
    point2 = sqrt(pow(100, 2) - pow((interval + discretization), 2));
       
    total = (point1 + point2)*discretization/2;

    return total;
}


int main(int argc, char *argv[]) {

    struct sockaddr_in server;

    int local_socket_file_descriptor;

    int len = sizeof(server);
    int slen;

    char buffer_in[LEN];
    char buffer_out[LEN];

    fprintf(stdout, "Starting Slave ...\n");

    // Cria o socket para o slave
    if ((local_socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error on slave socket creation:");
        
        return EXIT_FAILURE;
    }

    // Propriedades da conexao
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    
    memset(server.sin_zero, 0x0, 8);


    // Tenta se conectar ao master
    if (connect(local_socket_file_descriptor, (struct sockaddr*) &server, len) == -1) {
        perror("Can't connect to server");
        return EXIT_FAILURE;
    }

    double interval = 0;
    double discretization = 0;
    double result = 0;
    
    // Recebe a mensagem de apresentacao do master
    if ((slen = recv(local_socket_file_descriptor, buffer_in, LEN, 0)) > 0)
         sscanf(buffer_in, "%lf", &discretization);

    strcpy(buffer_out, "ready");
    
    send(local_socket_file_descriptor, buffer_out, strlen(buffer_out), 0);

    // Mantem a comunicacao com o master aberta enquanto a mensagem "exit"
    // nao chegar
    while (true) {
        // Zerando os buffers
        memset(buffer_in, 0x0, LEN);
        memset(buffer_out, 0x0, LEN);

        // Recebe a resposta do servidor
        slen = recv(local_socket_file_descriptor, buffer_in, LEN, 0);
        
        sscanf(buffer_in, "%lf", &interval);

        result = calculateIntegral(interval, discretization);

        // A mensagem de "bye" fecha a conexao
        if(strcmp(buffer_in, "bye") == 0)
            break;
        
        sprintf(buffer_out, "%lf", result);

        // Envia a mensagem para o servidor por meio do socket
        send(local_socket_file_descriptor, buffer_out, strlen(buffer_out), 0);
    }

    // Encerra a conexao com o master
    close(local_socket_file_descriptor);

    return 0;
}   