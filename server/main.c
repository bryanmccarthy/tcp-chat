#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define MAX_CLIENTS 50

int main(int argc, char *argv[]) {

  int server_fd, client_fd, opt = 1;
  struct sockaddr_in server_addr;
  int addrlen = sizeof(server_addr);
  char buffer[1024] = {0};

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

    printf("Connection made, client address: %s\n", inet_ntoa(server_addr.sin_addr));
    
  }

  close(server_fd);

  return 0;
}
