// by Frederik Holfeld

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>

#define PORT 8080
#define STD_CHARS 1024
#define BUFF_SIZE 1000000

int main(int argc, char* const argv[]) {

  // Create structs.
  struct sockaddr_in server_addr = {
    .sin_family = AF_INET,
    .sin_port = htons(PORT),
    .sin_addr.s_addr = htonl(INADDR_ANY)
  }, client_addr;

  // Create socket to listen for connections on defined port.
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  bind(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
  listen(sock, 32);
  int client_length = sizeof(client_addr);

  // Daemon loops indefinitely to serve requests until client sends "exit" command.
  while (1) {
    printf("Server listening on port %d.\n", PORT);
    int sock2 = accept(sock, (struct sockaddr*)&client_addr, &client_length);

    // Reads command that client sent into command-buffer.
    char command[STD_CHARS];
    int command_bytes = recv(sock2, &command, STD_CHARS, 0);

    // Append \0 to command, so previously executed commands aren't read from
    // command buffer due to faulty client input without \0.
    if (command_bytes < STD_CHARS) command[command_bytes] = '\0';

    // Shut down daemon when command "exit" is sent.
    if (!strcmp(command, "exit")) {
      send(sock2, "Vlftpd successfully shut down!\n", sizeof("Vlftpd successfully shut down!\n"), 0);
      printf("Client %s requested to shut down vlftpd. Shutting down, bye!\n", inet_ntoa(client_addr.sin_addr));
      close(sock);
      close(sock2);
      return 0;
    }

    printf("Client %s sent command \"%s\".\n", inet_ntoa(client_addr.sin_addr), command);

    // New functionality of assignment 2 starts here.
    // First handle get request.
    if (!strcmp(command, "get")) {

      // Read file name into buffer file_name.
      char file_name[STD_CHARS];
      recv(sock2, file_name, STD_CHARS, 0);
      printf("Got request to get file \"%s\" from client.\n", file_name);
      FILE* get_file = fopen(file_name, "r");

      // Check whether requested file exists.
      // If it doesn't, send 0 to host, so he knows he can't get the file.
      if (get_file == 0) {
        send(sock2, "0", 1, 0);
        printf("File does not exist! Aborting.\n\n");
        close(sock2);

      // Otherwise send 1, so client can prepare buffer and read file contents.
      } else {
        send(sock2, "1", 1, 0);
        char buff[BUFF_SIZE];
        int buff_bytes = read(fileno(get_file), buff, BUFF_SIZE);
        fclose(get_file);

        send(sock2, buff, buff_bytes, 0);

        close(sock2);

        printf("Sent file \"%s\" to client.\n\n", file_name);
      }

    // Handle put request.
    } else if (!strcmp(command, "put")) {

      // Receive file name and open new file to be read from.
      char file_name[STD_CHARS];
      recv(sock2, file_name, STD_CHARS, 0);
      FILE* test_file = fopen(file_name, "r");

      // If test_file is 0, file already exists in server directory. Don't override and send flag accordingly.
      if (test_file) {
        close(fileno(test_file));
        send(sock2, "0", 1, 0);
        close(sock2);
        printf("The file already exists! Won't override existing file. Aborting.\n\n");

      // Otherwise send flag 1, open file for writing, read into buffer and write buffer to file.
      } else {
        send(sock2, "1", 1, 0);
        FILE* put_file = fopen(file_name, "w");
        char buff[BUFF_SIZE];
        int buff_bytes = recv(sock2, buff, BUFF_SIZE, 0);
        write(fileno(put_file), buff, buff_bytes);
        close(fileno(put_file));
        printf("Successfully written client data to new file!\n\n");
      }

    // Handle cd request.
    } else if (!strcmp(command, "cd")) {

      // Get directory name.
      char dir_name[STD_CHARS];
      recv(sock2, dir_name, STD_CHARS, 0);

      // Mimick normal popen() behaviour and send msg as faked command output.
      char* msg;

      // Handle successful chdir or failure.
      if (chdir(dir_name)) {
        msg = "Couldn't cd to given directory! Please check for typos.\n";
        printf("Could not cd to %s! Aborting.\n\n", dir_name);
      } else {
        msg = "Successfully changed working directory.\n";
        printf("Changed working directory to %s.\n\n", dir_name);
      }
      send(sock2, msg, strlen(msg) + 1, 0);

    // In all other cases just execute command normally as in assignment 1.
    } else {
      // Execute command given by client via popen() and read its outputs into buff.
      // Save how many bytes were read, so that many bytes may be sent to client.
      // Only sending required number of bytes doesn't waste bandwitdh and prevents
      // buffer contents of previous commands to be displayed.
      char buff[BUFF_SIZE];
      FILE* command_file = popen(command, "r");
      int buff_bytes = read(fileno(command_file), buff, BUFF_SIZE);
      fclose(command_file);

      // Send command output buff to client.
      printf("Sending output of \"%s\" to client.\n", command);
      send(sock2, buff, buff_bytes, 0);

      // Close socket.
      close(sock2);
      printf("Sent output of \"%s\" to client.\n\n", command);
    }
  }
}
