#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>

#define PORT 8080
#define BUFFER_MAX 1024

void *handle_receive_broadcast(void *arg);
void handle_interrupt(int sig);

static int sock_fd;

int main(int argc, char *argv[]) {

  struct sockaddr_in server_addr;
  char buffer[BUFFER_MAX] = {0};
  pthread_t thread;

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

  // Signal handler for interrupt
  signal(SIGINT, handle_interrupt);

  if(pthread_create(&thread, NULL, handle_receive_broadcast, (void *)&sock_fd) < 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  // Client loop
  while(1) {
    printf("MSG: \n");
    scanf("%s", buffer);
    if(send(sock_fd, buffer, BUFFER_MAX, 0) < 0) {
      perror("send failed");
      exit(EXIT_FAILURE);
    }
  }

  close(sock_fd);

  return 0;
}

void *handle_receive_broadcast(void *arg) {
  int sock_fd = *(int *)arg;
  char buffer[BUFFER_MAX] = {0};

  while(1) {
    if(read(sock_fd, buffer, BUFFER_MAX) < 0) {
      perror("read failed");
      exit(EXIT_FAILURE);
    } 

    if(strcmp(buffer, "Room full") == 0) {
      printf("Room full, connection refused");
      exit(0);
    }

    printf("broadcast msg: %s\n", buffer);
  }

  close(sock_fd);
  pthread_exit(NULL);
}

void handle_interrupt(int sig) {
  if(send(sock_fd, "quit", BUFFER_MAX, 0) < 0) {
    perror("send failed");
  }

  close(sock_fd);
  exit(1);
}
