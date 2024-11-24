#if defined WIN32
#include <winsock.h>
#else
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "protocol.h"
#include "generator.h"

void clearwinsock() {
#if defined WIN32
	WSACleanup();
#endif
}

void errorhandler(char *errorMessage) {
	printf("%s", errorMessage);
}

int main(int argc, char *argv[]) {
	#if defined WIN32
	// Initialize Winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != NO_ERROR) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
	#endif

	// create welcome socket
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
		errorhandler("socket creation failed.\n");
		clearwinsock();
		return -1;
	}

	//set connection settings
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad)); // ensures that extra bytes contain 0 sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(PROTO_PORT); /* converts values between the host and network byte order. Specifically, htons() converts 16-bit quantities from host byte order to network byte order. */
	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("bind() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	// listen
	if (listen(my_socket, QLEN) < 0) {
		errorhandler("listen() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}

	// accept new connection
	struct sockaddr_in cad; // structure for the client address
	int client_socket;       // socket descriptor for the client
	socklen_t client_len;          // the size of the client address
	printf("Waiting for a client to connect...\n\n");
	while (1) {
		client_len = sizeof(cad); // set the size of the client address
		if ((client_socket = accept(my_socket, (struct sockaddr*) &cad, &client_len)) < 0) {
			errorhandler("accept() failed.\n");
			// close connection
			closesocket(client_socket);
			clearwinsock();
			return 0;
		}

		printf("New connection from %s:%d\n", inet_ntoa(cad.sin_addr), PROTO_PORT);

		char *s = "Connection established";
		if (send(client_socket, s, strlen(s), 0) != strlen(s)) {
			errorhandler("send() sent a different number of bytes than expected");
			closesocket(client_socket);
			clearwinsock();
			return -1;
		}

		// while client_socket is connected
		while (1) {
			password p;
			memset(p.password, '\0', 32);

			if ((recv(client_socket, &p, sizeof(password), 0)) <= 0) {
				errorhandler("recv() failed or connection closed prematurely");
				closesocket(client_socket);
				clearwinsock();
				return -1;
			}

			if (p.type == 'q') {
				printf("Client disconnected.\n");
				break;
			}

			if (p.length < 6 || p.length > 32) {
				printf("Password length must be between 6 and 32 characters.\n");
			} else {
				printf("Password length: %d\n", p.length);

				switch (p.type) {
					case 'n':
						printf("Generating numeric password...\n");
						generate_numeric(p.password, p.length);
						printf("Generated password: %s\n", p.password);
						break;
					case 'a':
						printf("Generating alphabetic password...\n");
						generate_alpha(p.password, p.length);
						printf("Generated password: %s\n", p.password);
						break;
					case 'm':
						printf("Generating mixed password...\n");
						generate_mixed(p.password, p.length);
						printf("Generated password: %s\n", p.password);
						break;
					case 's':
						printf("Generating secure password...\n");
						generate_secure(p.password, p.length);
						printf("Generated password: %s\n", p.password);
						break;
					default:
						printf("Invalid password type!\n");
						break;
				}
			}

			// send manipulated data to client
			if (send(client_socket, &p, sizeof(password), 0) != sizeof(password)) {
				errorhandler("send() sent a different number of bytes than expected");
				closesocket(client_socket);
				clearwinsock();
				return -1;
			}
		}
	}

} // main end

// function to generate numeric password
void generate_numeric(char* password, int length) {
	for (int i = 0; i < length; i++) {
		password[i] = (rand() % 10) + '0';
	}
}

// function to generate alphabetic password
void generate_alpha(char *password, int length) {
    for (int i = 0; i < length; i++) {
        password[i] = (rand() % 26) + 'a';
    }
}

// function to generate mixed password
void generate_mixed(char *password, int length) {
    for (int i = 0; i < length; i++) {
        if (rand() % 2 == 0) {
            password[i] = (rand() % 26) + 'a';
        } else {
            password[i] = (rand() % 10) + '0';
        }
    }
}

// function to generate secure password
void generate_secure(char *password, int length) {
    for (int i = 0; i < length; i++) {
        switch (rand() % 4) {
            case 0:
                password[i] = (rand() % 26) + 'a';
                break;
            case 1:
                password[i] = (rand() % 26) + 'A';
                break;
            case 2:
                password[i] = (rand() % 10) + '0';
                break;
            case 3:
                password[i] = (rand() % 15) + '!';
                break;
            default:
                break;
        }
    }
}
