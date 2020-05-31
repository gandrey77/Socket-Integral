#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>  
#include <arpa/inet.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h>
     
#define TRUE   1  
#define FALSE  0  

#define PORT 5000

#define BUFFER_LENGTH 64

char buffer[BUFFER_LENGTH];
char buffer_in[BUFFER_LENGTH];

int main(int argc , char *argv[]) {
    int opt = TRUE;

    int slave_socket[10],
        max_clients = 10,
        closed_slaves = 0,
        socket_descriptor,
        master_socket,
        addrlen,
        new_socket,
        activity,
        valread,
        i;

    int max_socket_descriptor;   
    struct sockaddr_in address;   
    int executing = TRUE;     
        
    // Conjunto de socket descriptors
    fd_set read_file_descriptors;   
         
    // Inicializa o array de endereçamento dos sockets
    for (i = 0; i < max_clients; i++)
        slave_socket[i] = 0;   
         
    // Cria o master socket
    if ( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {   
        perror("Socket failed");   

        return EXIT_FAILURE;   
    }

    fprintf(stdout, "[+]Server socket created with fd: %d\n", master_socket);   
    
    // Trata o erro de porta ja utilizada
    // Permite que o master socket aceite multiplas conexoes
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,  sizeof(opt)) < 0) {   
        perror("[-]Socket option error:");

        return EXIT_FAILURE;   
    }
     
    // Tipo de socket criado
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    // Conecta o socket ao localhost
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {   
        perror("[-] Socket bind failed");   
        
        return EXIT_FAILURE;   
    }

    fprintf(stdout, "[+]Listening on port %d\n", PORT); 
    
    // Inicializa as conexões em espera para os clientes. Além disso, especifica
    // o maximo de tres conexoes pendentes para o master socket.
    if (listen(master_socket, 3) < 0) {   
        perror("[-]Listen error:");

        return EXIT_FAILURE;   
    }
         
    // Aceita conexoes a caminho
    addrlen = sizeof(address);   

    puts("[+]Waiting for connections ...");   
        
    
    // Valores para resolucao da integral
    double total = 0;
    double  intervalo = 0;     
    double descrization = 0;
    double aux = 0;
    int slave = 0;
   
    printf("Digite o valor da discretizacao: ");
    scanf("%lf", &descrization);

    printf("Digite o numero de slaves necessario: ");
    scanf("%d", &slave);
    
    system("gcc slave.c -o slave -lm ");

    for(int i = 0 ; i < slave; i++){
        system("./slave&");
    }

    while(executing){   
        // Limpa o conjunto de sockets
        FD_ZERO(&read_file_descriptors);   
        
        // Adiciona o master socket ao conjunto de sockets
        FD_SET(master_socket, &read_file_descriptors);   

        max_socket_descriptor = master_socket;   
             
        // Adiciona os sockets dos slaves ao conjunto de sockets  
        for ( i = 0 ; i < max_clients ; i++) {
            socket_descriptor = slave_socket[i];   
                 
            // Se o descritor do socket for valido, o adicionamos a lista de leitura 
            if(socket_descriptor > 0)   
                FD_SET( socket_descriptor , &read_file_descriptors);   
                 
            if(socket_descriptor > max_socket_descriptor)   
                max_socket_descriptor = socket_descriptor;   
        }

        // Aguarda por uma atividade nos sockets
        // O recebimento de null significa a ocorrencia de um timeout
        activity = select( max_socket_descriptor + 1 , &read_file_descriptors , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR)){   
            printf("select error");   
        }   

        // Caso algo ocorra no master socket, entao temos uma conexao a caminho.
        if (FD_ISSET(master_socket, &read_file_descriptors)) {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                perror("accept");

                return EXIT_FAILURE;   
            }

            // Informa o usuario a respeito do numero do socket
            // Utilizado para o envio e recebimento de comandos
            printf("[+]New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs 
                  (address.sin_port));   
           
            // Envia a discretizacao
            sprintf(buffer_in, "%lf", descrization);

            if( send(new_socket, buffer_in, strlen(buffer_in), 0) != strlen(buffer_in))
                perror("send");
               
            // Adiciona um novo socket ao array de sockets
            for (i = 0; i < max_clients; i++)
                if(slave_socket[i] == 0 ) {   
                    slave_socket[i] = new_socket;
                    break;   
                }
        }

        // Caso contrario, temos uma operacao de IO em algum outro socket
        for (i = 0; i < max_clients; i++) {
            memset(buffer_in, 0x0, BUFFER_LENGTH);
            
            memset(buffer, 0x0, BUFFER_LENGTH);
            
            socket_descriptor = slave_socket[i];
                 
            if (FD_ISSET(socket_descriptor , &read_file_descriptors)){
                // Le a mensagem enviada pelo socket e verifica se esta
                // trata-se de seu fechamento
                if ((valread = read( socket_descriptor , buffer, 1024)) == 0) {
                    getpeername(socket_descriptor , (struct sockaddr*)&address ,  
                        (socklen_t*)&addrlen);

                    printf("Host disconnected , ip %s , port %d \n" ,
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                         
                    // Fecha o socket
                    close( socket_descriptor );

                    FD_CLR (socket_descriptor, &read_file_descriptors);
                    FD_CLR (master_socket, &read_file_descriptors);
                    
                    slave_socket[i] = 0;

                    closed_slaves++;

                    if(closed_slaves == slave) 
                        executing  = FALSE;
                }   
                     
                // Recebe a mensagem do slave
                else {
                    // Caso o intervalo seja maior que 100, envia um bye para o slive                                          
                    if(intervalo + descrization >= 100) {
                        strcpy(buffer, "bye");

                        send(socket_descriptor, buffer, strlen(buffer), 0);
                    }

                    // Calcula o intervalo
                    else {
                        if (strcmp(buffer, "ready") == 0) {  
                            sprintf(buffer, "%lf", intervalo);

                            send(socket_descriptor , buffer , strlen(buffer) , 0 );                    
                        }
                        else {
                            sscanf(buffer, "%lf", &aux);
                            
                            total += aux;
                            
                            sprintf(buffer, "%lf", intervalo);
                            
                            send(socket_descriptor , buffer , strlen(buffer) , 0 );
                        
                        }
                        intervalo += descrization;   
                    }
                }   
            }
        } 

    }
    printf("\nResultado: %lf\n", total);   
         
    return 0;   
}   