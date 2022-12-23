// by Frederik Holfeld

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define PORT 8080
#define HEADER 21

int main(int argc, char const *argv[]) {

  // Handle wrong amount of arguments.
  if (argc < 4) {
    printf("Bad input!\n");
    printf("Usage: smbpublish broker(ip) topic(string) message(string).\n");
    printf("Try again! Exiting now.\n");
    return 1;
  }

  // Create socket, address-structure and fill in port and borker-address.
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = inet_addr(argv[1]);

  // The message to be sent to the broker consists of a header and the message
  // itself.
  char msg[HEADER + strlen(argv[2]) + strlen(argv[3])];

  // The header contains the role, in this case 'p' for publisher.
  *msg = 'p';

  // Then the size of the entire message is given, so the broker can dynamically
  // allocate enough space for it.
  sprintf(msg + 1, "%10u", sizeof(msg));

  // After that follows the size of the topic, so broker knows when the actual
  // message starts.
  sprintf(msg + 11, "%10u", strlen(argv[2]));

  // Finally the topic and the message are appended.
  strcat(msg, argv[2]);
  strcat(msg, argv[3]);

  // Send message to broker and terminate.
  sendto(sock, msg, sizeof(msg), 0, (struct sockaddr*) &addr, sizeof(addr));
  printf("Published message \"%s\" to broker.\n", msg);

  return 0;
}
