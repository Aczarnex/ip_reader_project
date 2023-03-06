#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//consider padding
struct ip4_address {
	uint32_t addr;
	uint32_t mask;
	uint32_t network;
	uint32_t broadcast;
	char scope;
	char broadc;
	char loopb;
	char reservd;
	char class;
};

#define OCT1(ip) ((ip) >> 24)
#define OCT2(ip) OCT1((ip) << 8)
#define OCT3(ip) OCT1((ip) << 16)
#define OCT4(ip) OCT1((ip) << 24)
#define DOT_FORMAT "%u.%u.%u.%u"
#define DOT_DECM(ip) OCT1(ip),OCT2(ip),OCT3(ip),OCT4(ip)

/*
 * checks validity of a given octet, used during translation of input when
 * called by read_data()
 * returns 0 on success and an error code on failure
 */
int value_valid(int);

/*
 * checks validity of a given address in text format, called by read_data()
 * before translation
 * returns 0 on success and an error code on failure
 */
int format_valid(char *);

/*
 * translates text input into a numeric address, can be used to translate
 * an ipv4 address or mask, depending on given mode (0 - address, 1 - mask)
 * returns 0 on success and an error code on failure
 */
int read_data(struct ip4_address *, char *, int);

/*
 * translates a prefix mask attached to an ip address input in text form into a
 * numeric mask or infers a mask form address class.
 * returns 0 on success and an error code on failure
 */
int demask(struct ip4_address *, char *);

/*
 * assigns a class to an address based on its first octet
 */
void get_network_class(struct ip4_address *);

/*
 * determines the scope of the address between private (1) and public (0)
 */
void get_scope(struct ip4_address *);

/*
 * prints to standard output relevant information that can be inferred form
 * a given ip address and the optionally provided mask
 */
void get_info(struct ip4_address *);


int main(int argc, char *argv[])
{
	struct ip4_address ip4 = {.addr = 0, .mask = 0};

	if(read_data(&ip4, argv[1], 1)) {
		printf("Invalid IP address!\n");
		return 1;
	}
	get_network_class(&ip4);

	switch (argc) {		//a lot of lines repeat, streamline
	case 2:
		if (demask(&ip4, argv[1])) {
			printf("Not a valid subnet mask!\n");
			return 1;
		}
		break;
	case 3:
		if (read_data(&ip4, argv[2], 0)) {
			printf("Not a valid subnet mask!\n");
			return 1;
		}
		break;
	default:
		printf("Invalid Input.\n");
		return 1;
	}

	get_info(&ip4);
	return 0;
}

int read_data(struct ip4_address *address, char *input, int mode)
{
	if (format_valid(input)) return 1;

	long carry;	//contains currently read octed
	char *end;	//tracks current octed read

	for (int i = 0; i < 4; i++)
	{
		carry = strtol(input, &end, 10);
		if (value_valid(carry)) {
			printf("Not a valid IP Address.\n");
			return 1;
		}
		input = ++end;	//update position for strtol

		if(mode) {	//reading address for 1
			address->addr = address->addr << 8;
			address->addr = address->addr | carry;
		}
		else {		//reading mask for 0
			address->mask = address->mask << 8;
			address->mask = address->mask | carry;
		}
	}

	return 0;
}

int demask(struct ip4_address *address, char *input) //rework, is fuucked
{
	int len = strlen(input);
	int i;
	int prefix;

	//find '/' if it exists in correct place inside input
	for(i = 1; i < 4 && input[len - i] != '/'; i++);

	if(i != 4) {
		prefix = strtol(input + len - i + 1, NULL, 10);
		//len - i + 1 is the exact position after '/' sign
		if(prefix > 32 || prefix < 0) {
			return 1;
		}

		input[len - i] = 0;	//truncate leftover address
	} else {	//assign a default mask based on class
		switch(address->class) {
		case 'A':
			prefix = 8;
			break;
		case 'B':
			prefix = 16;
			break;
		case 'C':
			prefix = 24;
			break;
		default:
			prefix = 0;
			printf("Mask set to 0.0.0.0, cannot be assigned.\n");
			break;
		}
	}

	//leftshift a set of all 1's by complement of prefix to obtain a mask
	address->mask = (~0)<<(32-prefix);
	return 0;
}

void get_network_class(struct ip4_address *address)
{
	if (OCT1(address->addr) >= 240) {
		address->class = 'E';
	} else if (OCT1(address->addr) >= 224) {
		address->class = 'D';
	} else if (OCT1(address->addr) >= 192) {
		address->class = 'C';
	} else if (OCT1(address->addr) >= 128) {
		address->class = 'B';
	} else {
		address->class = 'A';
	}
}

int value_valid(int oct)
{
	if(oct > 255 || oct < 0) return 1;
	else return 0;
}

int format_valid(char * ip4)
{
	int digits = 0;
	int dots = 0;
	int len = strlen(ip4);
	int i = 0;

	/*
	 * function operates by counting the amount of digits between dots
	 * and the amount of said dots, as well as length of the prefix
	 */
	for(; i < len; i++) {
		if(isdigit(ip4[i])) {
			digits++;
			if(digits > 3) return 2;
		} else if(ip4[i] == '.') {
			dots++;
			digits = 0;
		} else if((ip4[i] == '/')) {
			digits = 0;
			if(len - i != 2 && len - i != 3) return 4;
		} else return 1;
	}
	if(dots != 3) return 3;

	return 0;
}

void get_info (struct ip4_address * address) {

	char *designation[2] = {"Public", "Private"};


	//filling in missing data
	//obtain network address
	address->network = address->addr & address->mask;

	//obtain network broadcast address
	address->broadcast = address->network | ~address->mask;

	//infer the scope of the address
	get_scope(address);

	printf("Address: "DOT_FORMAT"\n"
	"Mask: "DOT_FORMAT"\n"
	"Class: %c\n"
	"Network: "DOT_FORMAT"\n"
	"Broadcast: "DOT_FORMAT"\n"
	"%s Address\n"
	"\n",
	DOT_DECM(address->addr), DOT_DECM(address->mask), address->class,
	DOT_DECM(address->network), DOT_DECM(address->broadcast),
	designation[address->scope]);
}

void get_scope (struct ip4_address * address)
{
	if (OCT1(address->addr) == 10)
		address->scope = 1;
	if (OCT1(address->addr) == 192 && OCT2(address->addr) == 168)
		address->scope = 1;
	if (OCT1(address->addr) == 172 && (OCT2(address->addr) > 15 && OCT2(address->addr) < 32))
		address->scope = 1;
}
