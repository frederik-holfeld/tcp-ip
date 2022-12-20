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
    char command[COMMAND_SIZE];
    int command_bytes = recv(sock2, &command, COMMAND_SIZE, 0);

    // Append \0 to command, so previously executed commands aren't read from
    // command buffer due to faulty client input without \0.
    if (command_bytes < COMMAND_SIZE) command[command_bytes] = '\0';

    // Shut down daemon when command "exit" is sent.
    if (!strcmp(command, "exit")) {
      send(sock2, "srsmond successfully shut down!\n", sizeof("srsmond successfully shut down!\n"), 0);
      printf("Client %s requested to shut down srsmond. Shutting down, bye!\n", inet_ntoa(client_addr.sin_addr));
      close(sock);
      close(sock2);
      return 0;
    }

    printf("Client %s sent command \"%s\".\n", inet_ntoa(client_addr.sin_addr), command);

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
