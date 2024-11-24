#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define PROTO_PORT 60000
#define BUFFER_SIZE 64
#define QLEN 6

typedef struct {
	char type;
	int length;
	char password[32];
} password;

#endif /* PROTOCOL_H_ */
