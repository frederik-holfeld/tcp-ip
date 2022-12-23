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
  // Checked at beginning of programm to avoid useless server load.
  if (argc < 3) {

    // Command needs 3 arguments in all cases.
    printf("Invalid command line input!\n");
    printf("Usage: program-name server-ip remote-command [remote-command-parameters].\n");
    printf("Bye bye.\n");
    return 1;
  } else if ((!strcmp(argv[2], "get") || !strcmp(argv[2], "put")) && argc < 4) {

    // If command is get or put, 4th parameter is needed.
    printf("Command \"%s\" requires you to specify the file you want to %s! Aborting.\n", argv[2], argv[2]);
    return 1;
  } else if (!strcmp(argv[2], "get")) {

    // Prevent user from accidentally overriding local file.
    FILE* test_file;
    if (argc > 4) {
      test_file = fopen(argv[4], "r");
    } else {
      test_file = fopen(argv[3], "r");
    } if (test_file != 0) {
      printf("The file already exists in the current directory! Choose another name for the file!\n");
      return 1;
    }
  } else if (!strcmp(argv[2], "put")) {

    // Can't put file on server that doesn't exist.
    FILE* test_file = fopen(argv[3], "r");
    if (test_file == 0) {
      printf("File \"%s\" does not exist on this computer, can't put it onto server!\n", argv[3]);
      printf("Aborting.\n");
      return 1;
    }
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
    printf("Couldn't connect to host! Are you connected to the network?\n");
    printf("Aborting.\n");
    return 2;
  }

  // Create command-buffer and copy command into it.
  if (!strcmp(argv[2], "dir")) {
    send(sock, "ls", 3, 0);
  } else {
    send(sock, argv[2], strlen(argv[2]) + 1, 0);
  }

  // Send command to server.
  // Sending \0 isn't really necessary here, because server checks input.
  printf("Sent command \"%s\" to server.\n", argv[2]);

  // First checked case is get command.
  if (!strcmp(argv[2], "get")) {

    // Send file name to server.
    send(sock, argv[3], strlen(argv[3]) + 1, 0);

    // Reserve status-flag to judge whether file can be retrieved or not.
    char status;
    recv(sock, &status, 1, 0);
    if (status == '0') {

      // If status is 0, file can't be retrieved from server.
      printf("File \"%s\" does not exist on server! Aborting.\n", argv[3]);
      close(sock);
      return 3;
    }

    // Otherwise create buffer for requested file.
    char buff[BUFF_SIZE];
    int buff_bytes = recv(sock, buff, BUFF_SIZE, 0);

    FILE* new_file;

    // Set file-name accordingly.
    if (argc > 4) {
      new_file = fopen(argv[4], "w");
      printf("New file with name \"%s\" created.\n", argv[4]);
    } else {
      new_file = fopen(argv[3], "w");
      printf("New file \"%s\" created.\n", argv[3]);
    }

    // Write buffer to file and exit.
    write(fileno(new_file), buff, buff_bytes);
    fclose(new_file);
    printf("Successfully written file from server to local file!\n");

  // Second case is to check for put command.
  } else if (!strcmp(argv[2], "put")) {

    // Send file-name so server can check if it already exists in server directory.
    if (argc > 4) {
      send(sock, argv[4], strlen(argv[4]) + 1, 0);
    } else {
      send(sock, argv[3], strlen(argv[3]) + 1, 0);
    }

    // Again create status flag to evaluate success.
    char status;
    recv(sock, &status, 1, 0);

    // If status is 1, file doesn't exist on server and local file can be but there.
    if (status == '1') {
      char buff[BUFF_SIZE];
      FILE* put_file = fopen(argv[3], "r");

      // Read file contents into buffer and send buffer to server.
      int buff_bytes = read(fileno(put_file), buff, BUFF_SIZE);
      close(fileno(put_file));
      send(sock, buff, buff_bytes, 0);

    // If status is 0, file already exist and won't be overridden.
    } else {
      close(sock);
      printf("File already exists on server! Please choose another name!\n");
      return 4;
    }

  // Otherwise just do as in assignment 1 and read output of popen() into buffer for printing.
  } else {

    // Cd additionally needs to send directory to chdir to.
    if (!strcmp(argv[2], "cd")) {
      send(sock, argv[3], strlen(argv[3]) + 1, 0);
    }
    char buff[BUFF_SIZE];
    recv(sock, buff, BUFF_SIZE, 0);
    printf("\n%s\n", buff);
  }
  close(sock);
  printf("Connection closed, done. Good bye!\n");
  return 0;
}
