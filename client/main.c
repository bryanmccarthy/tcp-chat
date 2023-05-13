#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int main(int argc, char *argv[]) {

  int sock_fd, valread;
  struct sockaddr_in server_addr;
  char buffer[1024] = {0};

  // Create socket file descriptor
  if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Set server address
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  // Connect to server
  if(connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("connect failed");
    exit(EXIT_FAILURE);
  }

  // Client loop
  while(1) {
    printf("MSG: \n");
    scanf("%s", buffer);
    printf("buffer contents: %s\n", buffer);
    send(sock_fd, buffer, 1024, 0);
  }

  close(sock_fd);

  return 0;
}
