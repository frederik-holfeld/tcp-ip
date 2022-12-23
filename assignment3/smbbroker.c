// by Frederik Holfeld

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define PORT 8080
#define HEADER 21
#define INITIAL_LIST_SIZE 2 // Low for demonstration purposes.

// Base list structure.
// Allows dynamic growing of list but can also be accessed quickly via index.
struct list {
  uint count;
  uint max;
  void* *arr;
};

// A topic has a unique name and stores its messages and who is subscribed to it.
struct topic {
  char *name;
  struct list subs;
  struct list msgs;
};

// A wildcard only needs to store its name and its subscribers,
// as it only pertains to subscribers who don't publish any messages.
struct wildcard {
  char *name;
  struct list subs;
};

// Function searches for a topic or wildcard in a list of topics or wildcards.
void* search_name(struct list *list, char *name) {
  printf("Searching list for name \"%s\".\n", name);

  // Cast the list's void* array to a wildcard array.
  // For this purpose, wildcards and topics are interchangeable.
  struct wildcard* *name_arr = (struct wildcard**) list->arr;

  // Loop through every entry and compare names.
  for (size_t i = 0; i < list->count; i++) {
    printf("Comparing to name \"%s\" in list.\n", name_arr[i]->name);

    // Return topic or wildcard if found.
    if (!strcmp(name_arr[i]->name, name)) return name_arr[i];
  }

  // Else return NULL.
  printf("Name \"%s\" not contained in list.\n", name);
  return NULL;
}

// Conveniance function for creating a new topic.
struct topic* new_topic(char *name) {
  printf("Creating new topic called \"%s\".\n", name);

  // Allocate space and fill in values.
  struct topic *topic = malloc(sizeof(struct topic));
  topic->subs.count = 0;
  topic->subs.max = INITIAL_LIST_SIZE;
  topic->subs.arr = malloc(sizeof(void*) * INITIAL_LIST_SIZE);
  topic->msgs.count = 0;
  topic->msgs.max = INITIAL_LIST_SIZE;
  topic->msgs.arr = malloc(sizeof(void*) * INITIAL_LIST_SIZE);
  topic->name = name;

  // Return filled in topic.
  return topic;
}

// Conveniance function for creating a new wildcard.
struct wildcard* new_wildcard(char* name) {
  printf("Creating new wildcard called \"%s\".\n", name);

  // Allocate space and fill in values.
  struct wildcard *wildcard = malloc(sizeof(struct wildcard));
  wildcard->subs.count = 0;
  wildcard->subs.max = INITIAL_LIST_SIZE;
  wildcard->subs.arr = malloc(sizeof(void*) * INITIAL_LIST_SIZE);
  wildcard->name = name;

  // Return new wildcard.
  return wildcard;
}

// Universal function to expand a list.
void expand_list(struct list *list) {
  printf("Expanding list from %d elements to %d elements.\n", list->max, list->max * 2);

  // Double list capacity every time the list is full.
  // Could be tuned more intelligently to avoid
  // wasting memory or too frequent expansion.
  list->max *= 2;

  // Allocate space for new pointer array and copy old values into it.
  void* *new_elements = malloc(list->max * sizeof(void*));
  memcpy(new_elements, list->arr, list->max / 2 * sizeof(void*));

  // Free old array and make the list's array the new array.
  free(list->arr);
  list->arr = new_elements;
}

// Function to reduce code duplication in main().
// Not implemented yet.
void serve_subs(struct topic *topic, char *msg) {

}

int main(int argc, char const *argv[]) {

  // Create a topic and a wildcard list.
  // Two lists needed so wildcards can be handled differently to normal topics.
  struct list topic_list = {
    .count = 0,
    .max = INITIAL_LIST_SIZE,
    .arr = malloc(sizeof(void*) * INITIAL_LIST_SIZE)
  };

  struct list wildcard_list = {
    .count = 0,
    .max = INITIAL_LIST_SIZE,
    .arr = malloc(sizeof(void*) * INITIAL_LIST_SIZE)
  };

  // Variable to count served requests. Maybe interesting for benchmarking.
  uint req_count = 0;

  // Create socket, address-structures and set values for listening-address.
  int sock = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in server_addr, client_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  // Bind socket to listen for incoming requests.
  bind(sock, (struct sockaddr*) &server_addr, sizeof(server_addr));

  // Enter infinite loop to take requests from publishers and subscribers.
  while (1) {

    // Allocate string for header information.
    // recvfrom() is set to MSG_PEEK so it can be read again further down.
    char header[HEADER];
    socklen_t client_size = sizeof(client_addr);
    recvfrom(sock, header, sizeof(header), MSG_PEEK, (struct sockaddr*) &client_addr, &client_size);
    printf("Got message from client %s.\n", inet_ntoa(client_addr.sin_addr));
    printf("Header: \"%s\".\n", header);

    // Copy the first ten header bytes into string
    // to determine total length of message with atoi().
    char total_size_char[10];
    strncpy(total_size_char, header + 1, sizeof(total_size_char));
    uint total_size = atoi(total_size_char);
    printf("Total size of received message: %d.\n", total_size);

    // Allocate space for the entire message including header and read into it.
    char *msg_long = malloc(total_size);
    recvfrom(sock, msg_long, total_size, 0, (struct sockaddr*) &client_addr, &client_size);


    // Now that the message is stored, it can be processed further
    // depending on what role the client had.

    // Handle requests from publishers.
    if (*header == 'p') {

      // The message of a publisher needs to be disected into the topic name
      // and the message itself. determine the length of the topic.
      char topic_size_char[10];
      strncpy(topic_size_char, header + 11, sizeof(topic_size_char));
      uint topic_size = atoi(topic_size_char);
      printf("Size of topic: %d.\n", topic_size);

      // Store the topic as separate string.
      // Put it on heap so it can be used later on in the lists.
      char *topic = malloc(topic_size + 1);
      strncpy(topic, msg_long + HEADER, topic_size);
      topic[topic_size] = '\0';
      printf("Topic: %s.\n", topic);

      // Now store only the message without header-information.
      uint msg_size = total_size - HEADER - topic_size;
      char *msg = malloc(msg_size + 1);
      strncpy(msg, msg_long + HEADER + topic_size, msg_size);
      msg[msg_size] = '\0';

      // Free original message with header.
      free(msg_long);
      printf("Message from publisher %s: \"%s\".\n", inet_ntoa(client_addr.sin_addr), msg);


      // Regardless of whether the topic, to which the publisher published,
      // exists or not, check the wildcards and serve subscribers
      // of potentially matching wildcards.

      // Raise port by one, as subscribers listen on PORT + 1.
      client_addr.sin_port = htons(PORT + 1);
      printf("Serving wildcard-subscribers now...\n");

      // Prefix the message for wildcard subscribers with the topic-name,
      // so they know what topic the message came from.
      uint wildcard_msg_size = topic_size + 2 + msg_size;
      char *wildcard_msg = malloc(wildcard_msg_size);
      strcpy(wildcard_msg, topic);
      strcat(wildcard_msg, ": ");
      strcat(wildcard_msg, msg);

      // Iterate through every wildcard in the wildcard-list.
      for (size_t i = 0; i < wildcard_list.count; i++) {
        struct wildcard *cur_wildcard = wildcard_list.arr[i];

        // Check for match of topic and wildcard.
        // Here the wildcard is compared only up to the '#',
        // so that "test/*" matches to wildcard "test/#".
        // This allows for completely userdefined topic-hierarchy and wildcards
        // still work as expected.
        if (!strncmp(cur_wildcard->name, topic, strlen(cur_wildcard->name))) {
          printf("Wildcard \"%s\" matches topic %s.\n", cur_wildcard->name, topic);

          // If wildcard matches,
          // send the message to every subscriber of that wildcard.
          for (size_t j = 0; j < cur_wildcard->subs.count; j++) {
            client_addr.sin_addr.s_addr = inet_addr(cur_wildcard->subs.arr[j]);
            sendto(sock, wildcard_msg, wildcard_msg_size, 0, (struct sockaddr*) &client_addr, sizeof(client_addr));
            printf("Sent message to wildcard-subscriber %s.\n", inet_ntoa(client_addr.sin_addr));
          }
        }
      }

      // All wildcards are served, don't need it anymore.
      free(wildcard_msg);

      // Now look for an exact match of topic in the topic list.
      struct topic *found_topic = search_name(&topic_list, topic);

      // If topic isn't found, create it.
      if (found_topic == NULL) {
        printf("Topic not found, creating it now...\n");
        struct topic *new_top = new_topic(topic);

        // Save pointer to msg in new topic's array and increase element count.
        new_top->msgs.arr[0] = msg;
        new_top->msgs.count++;

        // Make reference to new topic in the topic list.
        topic_list.arr[topic_list.count] = new_top;

        // Increase topic list counter and expand if full.
        topic_list.count++;
        if (topic_list.count == topic_list.max) expand_list(&topic_list);
      } else {
        printf("Topic already exists, sending message to subscribers...\n");

        // If the topic already exists, then serve its subscribers.
        // Again, send to port PORT + 1.
        client_addr.sin_port = htons(PORT + 1);

        // Iterate through the subscriber list and send message.
        for (size_t i = 0; i < found_topic->subs.count; i++) {
          client_addr.sin_addr.s_addr = inet_addr(found_topic->subs.arr[i]);
          sendto(sock, msg, msg_size, 0, (struct sockaddr*) &client_addr, sizeof(client_addr));
          printf("Sent message to subscriber %s\n", inet_ntoa(client_addr.sin_addr));
        }

        // Save message in the topic's message list and expand if it's full.
        found_topic->msgs.arr[found_topic->msgs.count] = msg;
        found_topic->msgs.count++;
        if (found_topic->msgs.count == found_topic->msgs.max) expand_list(&found_topic->msgs);

        // Topic string isn't needed anymore, as the topic already has one.
        free(topic);
      }
      printf("Served publishers request.\n");
    } else if (*header == 's') {

      // Requests from subscribers don't contain any message and
      // are comprised entirely of the topic. Store it.
      uint topic_size = total_size - HEADER;
      printf("Size of topic: %d.\n", topic_size);
      char *topic = malloc(topic_size + 1);
      strncpy(topic, msg_long + 11, topic_size);
      topic[topic_size] = '\0';
      printf("Topic: %s\n", topic);

      // Store the subscriber's address as a string for later use.
      uint addr_char_len = strlen(inet_ntoa(client_addr.sin_addr));
      char *addr_char = malloc(addr_char_len + 1);
      strcpy(addr_char, inet_ntoa(client_addr.sin_addr));
      addr_char[addr_char_len] = '\0';

      // Determine whether the topic contains a '#' and treat it accordingly
      // as a normal topic or as a wildcard.
      if (strchr(topic, '#') != NULL) {
        printf("The topic is a wildcard.\n");

        // Create wildcard string to use for comparison.
        // Neglects any characters from '#' onward.
        // Avoids creating new wildcards where none are needed.
        // For example "#42" is redundant, only create wildcard for "#".
        uint wildcard_len = strchr(topic, '#') - topic;
        char *wildcard = malloc(wildcard_len + 1);
        strncpy(wildcard, topic, wildcard_len);
        wildcard[wildcard_len] = '\0';
        printf("Wildcard: \"%s\".\n", wildcard);

        // If topic is a wildcard, serve subscriber the past history
        // of all matching topics.
        client_addr.sin_port = htons(PORT + 1);

        // Iterate through every topic and send messages of matching ones.
        for (size_t i = 0; i < topic_list.count; i++) {
          struct topic *cur_topic = topic_list.arr[i];

          // Again compare only up to the '#' to make the wildcard work.
          if (!strncmp(cur_topic->name, wildcard, wildcard_len)) {
            printf("Wildcard matches the topic \"%s\". Sending previous messages.\n", cur_topic->name);

            // Send all messages already contained in the topic.
            for (size_t j = 0; j < cur_topic->msgs.count; j++) {

              // Prefix the message for wildcard subscribers with the topic-name,
              // so they know what topic the message came from.
              uint wildcard_msg_size = strlen(cur_topic->name) + 2 + strlen(cur_topic->msgs.arr[j]);
              char *wildcard_msg = malloc(wildcard_msg_size);
              strcpy(wildcard_msg, cur_topic->name);
              strcat(wildcard_msg, ": ");
              strcat(wildcard_msg, cur_topic->msgs.arr[j]);

              // Send the wildcard_msg.
              sendto(sock, wildcard_msg, wildcard_msg_size, 0, (struct sockaddr*) &client_addr, sizeof(client_addr));

              // wildcard_msg no longer needed.
              free(wildcard_msg);
            }
          }
        }

        // Now look for existing wildcard entry and act accordingly.
        struct wildcard *found_wildcard = search_name(&wildcard_list, wildcard);

        // If the wildcard isn't found, then create it.
        if (found_wildcard == NULL) {
          printf("Wildcard not found, creating it...\n");

          // Allocate space, save subscriber address and increment counter.
          struct wildcard *new_wild = new_wildcard(wildcard);
          new_wild->subs.arr[0] = addr_char;
          new_wild->subs.count++;

          // Add wildcard to the list and expand if necessary.
          wildcard_list.arr[wildcard_list.count] = new_wild;
          wildcard_list.count++;
          if (wildcard_list.count == wildcard_list.max) expand_list(&wildcard_list);
        } else {
          printf("Wildcard found. Appending if not already subscribed...\n");

          // Iterate through the subscribers and see if already subscribed.
          for (size_t i = 0; i < found_wildcard->subs.count; i++) {

            // If found, don't do anything.
            if (!strcmp(found_wildcard->subs.arr[i], addr_char)) {
              printf("Already subscribed to wildcard!\n");
              break;
            }

            // Otherwise append to list and maybe expand it.
            found_wildcard->subs.arr[found_wildcard->subs.count] = addr_char;
            found_wildcard->subs.count++;
            if (found_wildcard->subs.count == found_wildcard->subs.max) expand_list(&found_wildcard->subs);
          }

          // Free the unneeded topic string.
          free(topic);
        }
      } else {

        // If topic isn't wildcard, just search the ordinary topics.
        struct topic *found_topic = search_name(&topic_list, topic);
        if (found_topic == NULL) {
          printf("Topic doesn't exist. Creating it...\n");

          // Create topic.
          struct topic *new_top = new_topic(topic);
          new_top->subs.arr[0] = addr_char;
          new_top->subs.count++;

          // Append to the topic list.
          topic_list.arr[topic_list.count] = new_top;
          topic_list.count++;
          if (topic_list.count == topic_list.max) expand_list(&topic_list);
        } else {
          printf("Topic exists. Sending missed messages...\n");

          // If topic already exists, get all its past messages.
          client_addr.sin_port = htons(PORT + 1);

          // Send all messages in topic.
          for (size_t i = 0; i < found_topic->msgs.count; i++) {
            sendto(sock, found_topic->msgs.arr[i], strlen(found_topic->msgs.arr[i]), 0, (struct sockaddr*) &client_addr, sizeof(client_addr));
          }

          // See if previously subscribed to topic.
          for (size_t i = 0; i < found_topic->subs.count; i++) {

            // When already subscribed, do nothing.
            if (!strcmp(found_topic->subs.arr[i], addr_char)) {
              printf("Already subscribed to topic!\n");
              break;
            }

            // Otherwise append to subscriber list.
            found_topic->subs.arr[found_topic->subs.count] = addr_char;
            found_topic->subs.count++;
            if (found_topic->subs.count == found_topic->subs.max) expand_list(&found_topic->subs);
          }

          // As the topic already exists, clean up the heap.
          free(topic);
        }
      }
      printf("Served request of subscriber.\n");
    }
    printf("Request number %u done.\n\n", ++req_count);
  }

  return 0;
}
