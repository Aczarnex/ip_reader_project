//rename error codes, add enums
//rename placeholders

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ip_reader.h"

#define OCTET_1ST(ip) ((ip) >> 24)
#define OCTET_2ND(ip) OCTET_1ST((ip) << 8)
#define OCTET_3RD(ip) OCTET_1ST((ip) << 16)
#define OCTET_4TH(ip) OCTET_1ST((ip) << 24)
#define DECIMAL_FORMAT_STRING "%u.%u.%u.%u"
#define DECIMAL_FORMAT_VALUES(ip) OCTET_1ST(ip),OCTET_2ND(ip),OCTET_3RD(ip),OCTET_4TH(ip)

enum return_flags {SUCCESS, ADDR_INVAL, MASK_INVAL, FORM_INVAL, EXIT_FLAG};

int parse_ip4_short(struct ip4_address *address, char *input)
{
	if (read_data(&address->addr, input)) {
		printf("Invalid IP address!\n");
		return ADDR_INVAL;
	}

	if (demask(address, input)) {
		printf("Not a valid subnet mask!\n");
		return MASK_INVAL;
	}
	return SUCCESS;
}

int parse_ip4_long(struct ip4_address *address, char *input[])
{
	if (read_data(&address->addr, input[1])) {
		printf("Invalid IP address!\n");
		return ADDR_INVAL;
	}

	if (read_data(&address->mask, input[2])) {
		printf("Not a valid subnet mask!\n");
		return MASK_INVAL;
	}

	return SUCCESS;
}

int read_data(uint32_t *temp_name, char *input)
{
	if (format_valid(input)) return FORM_INVAL;

	long carry;	//contains currently read octed
	char *end;	//tracks current octed read

	for (int i = 0; i < 4; i++)
	{
		carry = strtol(input, &end, 10);
		if (carry > 255 || carry < 0) {
			printf("Not a valid IP Address.\n");
			return FORM_INVAL;
		}
		input = ++end;	//update position for strtol
		*temp_name = *temp_name << 8;
		*temp_name = *temp_name | carry;
	}

	return SUCCESS;
}

int demask(struct ip4_address *address, char *input) //check if I should use uint8_t, should find examples in kernel //mostly int
{
	int len = strlen(input);
	int i;
	int prefix;

	//find '/' if it exists in correct place inside input
	for(i = 1; i < 4 && input[len - i] != '/'; i++);

	if (i != 4) {
		prefix = strtol(input + len - i + 1, NULL, 10);
		//len - i + 1 is the exact position after '/' sign
		if (prefix > 32 || prefix < 0) {
			return MASK_INVAL;
		}

		input[len - i] = 0;	//truncate leftover address
	} else {	//assign a default mask based on class
		switch (get_network_class(address->addr)) {
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
	address->mask = (~0) << (32 - prefix);
	return SUCCESS;
}

char get_network_class(uint32_t address)
{
	uint8_t oct1 = OCTET_1ST(address);

	if (oct1 >= 240) {
		return 'E';
	} else if (oct1 >= 224) {
		return 'D';
	} else if (oct1 >= 192) {
		return 'C';
	} else if (oct1 >= 128) {
		return 'B';
	} else {
		return 'A';
	}
}

int format_valid(char *ip4)
{
	int digits = 0;
	int dots = 0;
	int len = strlen(ip4);
	int i;

	/*
	 * function operates by counting the amount of digits between dots
	 * and the amount of said dots, as well as length of the prefix
	 */
	for (i = 0; i < len; i++) {
		if (isdigit(ip4[i])) {
			digits++;
			if (digits > 3) return FORM_INVAL;
		} else if (ip4[i] == '.') {
			dots++;
			digits = 0;
		} else if ((ip4[i] == '/')) {
			digits = 0;
			if (len - i != 2 && len - i != 3) return FORM_INVAL;
		} else return FORM_INVAL;
	}
	if (dots != 3) return FORM_INVAL;

	return SUCCESS;
}

void get_info(struct ip4_address *address)
{
	if (get_network_class(address->addr) == 'D') {
		printf(
		"Address: "DECIMAL_FORMAT_STRING"\n"
		"Class: %c\n",
		DECIMAL_FORMAT_VALUES(address->addr),
		get_network_class(address->addr));

	} else {
		char *designation[2] = {"Public", "Private"};

		//filling in missing data
		//obtain network address
		uint32_t network = address->addr & address->mask;

		//obtain network broadcast address
		uint32_t broadcast = network | ~address->mask;

		printf(
		"Address: "DECIMAL_FORMAT_STRING"\n"
		"Mask: "DECIMAL_FORMAT_STRING"\n"
		"Class: %c\n"
		"Network: "DECIMAL_FORMAT_STRING"\n"
		"Broadcast: "DECIMAL_FORMAT_STRING"\n"
		"%s Address\n\n",
		DECIMAL_FORMAT_VALUES(address->addr),
		DECIMAL_FORMAT_VALUES(address->mask),
		get_network_class(address->addr),
		DECIMAL_FORMAT_VALUES(network),
		DECIMAL_FORMAT_VALUES(broadcast),
		designation[get_scope(address->addr)]);
	}
}

int get_scope(uint32_t address)
{
	uint8_t oct1 = OCTET_1ST(address);
	uint8_t oct2 = OCTET_2ND(address);

	if (oct1 == 10)
		return EXIT_FLAG;
	if (oct1 == 192 && oct2 == 168)
		return EXIT_FLAG;
	if (oct1 == 172 && oct2 > 15 && oct2 < 32)
		return EXIT_FLAG;
	return SUCCESS;
}
