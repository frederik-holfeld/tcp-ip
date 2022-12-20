# TCP/IP project

This is the work I have done for a university course called "TCP/IP-Programming". We were given two weeks for each of the exercises, which are implemented in C.


## Exercise 1 - Simple remote system monitor

Simple implementation of a remote system monitor client and server application utilizing TCP and run as a non interactive, single command. Some parameters execute specific commands, otherwise the same command is sent to the remote machine and its outputs forwarded to the client.


## Exercise 2 - Very lightweight file transfer protocol

Implementation of a basic TCP file transfer protocol. The client can send files to and receive files from the server.


## Exercise 3 - Simple message broker

This is the final and rated program of the course. We were to implement a basic message broker system based on UDP, consisting of programs for:

Publisher - Sends messages to a topic, which the broker distributes to subscribers.
Subscriber - Subscribes to a topic and receives messages from publishers delivered by the broker.
Broker - Receives publishing and subscription requests and forwards published messages to every subscriber of that topic.


Unfortunately there was some curious misbehavior of the code when my professor tested it on non x86 hardware that didn't show up in my testing, so the grade sadly didn't pan out the way I wanted.
But it was a great learning experience nonetheless and a good way of becoming a little more experienced with C in projects of a few hundred lines of code :)
