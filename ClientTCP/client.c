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
#include "protocol.h"

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

	// create client socket
	int c_socket;
	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket < 0) {
		errorhandler("socket creation failed.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	// set connection settings
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP del server
	sad.sin_port = htons(PROTO_PORT); // Server port

	// connection
	if (connect(c_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("Failed to connect.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	// receive from server
	char buffer[BUFFER_SIZE];
	memset(buffer, '\0', BUFFER_SIZE);
	if ((recv(c_socket, buffer, BUFFER_SIZE - 1, 0)) <= 0) {
		errorhandler("recv() failed or connection closed prematurely");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}
	printf("%s\n", buffer); // Print the echo buffer

	while (1) {
		// get data from user
		password p = {0};

        do {
            printf("Enter the type of password to be generated, followed by a space and the length. (e.g. 'n 8')\n");
            printf("\tn: numeric password (only digit)\n");
            printf("\ta: alphabetic password (lower case letters only)\n");
            printf("\tm: mixed password (lowercase letters and numbers)\n");
            printf("\ts: secure password (upper and lower case letters, numbers and symbols)\n");
            printf("Enter 'q' to exit.\n");
            printf("Choice: ");
            scanf("%c", &p.type);  // get password type

            if (p.type == 'q') {
                break;
			}

			scanf("%d", &p.length); // get password length
            fflush(stdin);

			if (p.type != 'n' && p.type != 'a' && p.type != 'm' && p.type != 's') {
				printf("\nInvalid password type!\n");
			}

			if (p.length < 6 || p.length > 32) {
				printf("\nInvalid password length! Min. 6 chars and Max 32 chars\n\n");
			}
		} while ((p.type != 'n' && p.type != 'a' && p.type != 'm' && p.type != 's') || p.length < 6 || p.length > 32);

		// send data to server
		if (send(c_socket, &p, sizeof(password), 0) != sizeof(password)) {
			errorhandler("send() sent a different number of bytes than expected");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}

		if (p.type == 'q') {
			closesocket(c_socket);
			clearwinsock();
			return 0;
		}

		// get reply from server
		if ((recv(c_socket, &p, sizeof(password), 0)) != sizeof(password)) {
			errorhandler("recv() failed or connection closed prematurely");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}
		printf("Generated password: %s\n", p.password);
        }
} // main end
