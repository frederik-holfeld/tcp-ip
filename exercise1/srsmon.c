// by Frederik Holfeld

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8080
#define COMMAND_SIZE 1024
#define BUFF_SIZE 1000000

int main(int argc, char* const argv[]) {
  // Check if command is executed properly.
  if (argc < 3) {
    printf("Invalid command line input!\n");
    printf("Usage: program-name server-ip remote-command [remote-command-parameters].\n");
    printf("Bye bye.\n");
    return 1;
  }

  // Create struct.
  struct sockaddr_in sock_addr = {
    .sin_family = AF_INET,
    .sin_port = htons(PORT),
    .sin_addr.s_addr = inet_addr(argv[1])
  };

  // Create socket for connection request.
  int sock = socket(AF_INET, SOCK_STREAM, 0);

  // Handle failure to establish connection.
  if (connect(sock, (struct sockaddr*)&sock_addr, sizeof(struct sockaddr))) {
    printf("Couldn't connect to host. Are you connected to the network?\n");
    printf("Aborting.\n");
    return 2;
  }

  // Create command-buffer and copy command into it.
  // Commands mentioned on assignment sheet are replaced by actual linux commands.
  char command[COMMAND_SIZE];
  if (!strcmp(argv[2], "users")) {
    strcpy(command, "who");
  } else if (!strcmp(argv[2], "procs")) {
    strcpy(command, "ps -ef");
  } else if (!strcmp(argv[2], "nets")) {
    strcpy(command, "netstat");
  } else if (!strcmp(argv[2], "disks")) {
    strcpy(command, "df -h");
  } else {
    strcpy(command, argv[2]);
  }

  // Append potential command parameter(s) to command-buffer.
  for (int i = 3; i < argc; i++) {
    strcat(command, " ");
    strcat(command, argv[i]);
  }

  // Send command to server.
  // Sending \0 isn't really necessary here, because server checks input.
  send(sock, command, strlen(command) + 1, 0);
  printf("Sent command \"%s\" to server.\n", command);

  // Create buffer for server-reply, read and display it, close socket and exit.
  char buff[BUFF_SIZE];
  recv(sock, &buff, BUFF_SIZE, 0);
  printf("\n%s\n", buff);
  close(sock);
  printf("Connection closed, done. Good bye!\n");
  return 0;
}
