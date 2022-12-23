// by Frederik Holfeld

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>

#define PORT 8080
#define HEADER 21
#define BUFF_SIZE 1024

int main(int argc, char const *argv[]) {

  // Handle wrong amount of arguments.
  if (argc < 3) {
    printf("Bad input!\n");
    printf("Usage: smbsubscribe broker(ip) topic(string).\n");
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
  char msg[HEADER + strlen(argv[2])];

  // The header contains the role, in this case 's' for subscriber.
  *msg = 's';

  // Then the size of the entire message is given, so the broker can dynamically
  // allocate enough space for it.
  sprintf(msg + 1, "%10u", sizeof(msg));

  // Append the topic to subscribe to.
  strcat(msg, argv[2]);

  // Send message to broker.
  sendto(sock, msg, sizeof(msg), 0, (struct sockaddr*) &addr, sizeof(addr));
  printf("Sent subscription request \"%s\" to broker.\n", msg);

  // The subscriber listens for messages on the port PORT + 1 to avoid sending
  // itself messages when running a subscriber on localhost.
  addr.sin_port = htons(PORT + 1);
  addr.sin_addr.s_addr = INADDR_ANY;

  // Get another socket descriptor. Otherwise messages aren't received.
  sock = socket(AF_INET, SOCK_DGRAM, 0);

  // Bind socket for listening.
  bind(sock, (struct sockaddr*) &addr, sizeof(addr));
  printf("Bound to port %d.\n", ntohs(addr.sin_port));

  // Create variables for time measurment.
  time_t now;
  struct tm *print_time;

  // Create necessary variables.
  char buff[BUFF_SIZE + 1];
  socklen_t addr_size = sizeof(addr);
  uint bytes_received;

  // Enter infinite loop for displaying subscribed messages.
  while (1) {

    // Receive data and measure how many bytes were read.
    bytes_received = recvfrom(sock, buff, sizeof(buff), 0, NULL, NULL);

    // Set end of string for proper display.
    buff[bytes_received] = '\0';

    // Set time to display when the message was received.
    time(&now);
    print_time = localtime(&now);

    // Display message.
    printf("[%d:%02d:%02d] %s\n", print_time->tm_hour, print_time->tm_min, print_time->tm_sec, buff);
  }

  return 0;
}
