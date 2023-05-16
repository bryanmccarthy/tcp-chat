#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_MAX 1024
#define MAX_CLIENTS 50

void *handle_client(void *arg);

int clients[50] = {0};
int num_clients = 0;

int main(int argc, char *argv[]) {
  int server_fd, client_fd, opt = 1;
  struct sockaddr_in server_addr;
  int addrlen = sizeof(server_addr);
  char buffer[BUFFER_MAX] = {0};
  char room_full[32] = "Room full";

  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Set option at protocol level (reuse port at sol_socket level)
  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // Set server address
  server_addr.sin_family = AF_INET;  
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // Bind socket to address
  if(bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Listen for client connections
  if(listen(server_fd, 3) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Listening on port %d\n", PORT);

  // Server loop
  while(1) {

    if((client_fd = accept(server_fd, (struct sockaddr *) &server_addr, (socklen_t *) &addrlen)) < 0) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    // Refuse connection if room full
    if(num_clients == MAX_CLIENTS) {
      printf("Room is full, refusing client %d\n", client_fd);
      if(send(client_fd, room_full, strlen(room_full), 0) < 0) {
        perror("send failed");
        exit(EXIT_FAILURE);
      }
      close(client_fd);
      continue;
    }

    // Add client to list of clients
    for(int i = 0; i < MAX_CLIENTS; i++) {
      if(clients[i] == 0) {
        clients[i] = client_fd;
        num_clients++;
        break;
      }
    }

    printf("Connection made, client address: %s\n", inet_ntoa(server_addr.sin_addr));

    // Display the current number of clients
    printf("Clients connected: %d\n", num_clients);

    // Handle client in thread
    pthread_t thread;
    if(pthread_create(&thread, NULL, handle_client, (void *)&client_fd) < 0 ) {
      perror("pthread_create failed"); 
      exit(EXIT_FAILURE);
    }
  }

  close(server_fd);

  return 0;
}

void *handle_client(void *arg) {
  int client_fd = *(int *)arg;
  char buffer[BUFFER_MAX] = {0};
  char message[BUFFER_MAX + 100] = {0};

  // Beginning of message will include client fd
  sprintf(message, "(client %d): ", client_fd);

  if(num_clients == MAX_CLIENTS) {
    printf("Client %d refused, room is full", client_fd);
    close(client_fd);
    pthread_exit(NULL);
    return NULL;
  }

  // Thread loop
  while(1) {
    if(read(client_fd, buffer, BUFFER_MAX) < 0) {
      perror("read failed");
      exit(EXIT_FAILURE);
    }

    if(strcmp(buffer, "quit") == 0) {
      printf("Client %d has disconnected\n", client_fd);
      num_clients--;

      // Remove client from list
      for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] == client_fd) {
          clients[i] = 0;
        }
      }

      break;
    } else {
      // Broadcast Message
      strcat(message, buffer);
      printf("MSG: %s", message);
      for(int i = 0; i < MAX_CLIENTS; i++) {
        if(clients[i] != 0) {
          if(send(clients[i], message, BUFFER_MAX, 0) < 0) {
            perror("send failed");
            exit(EXIT_FAILURE);
          }
        }
      }
    }
  }

  close(client_fd);
  pthread_exit(NULL);
}
