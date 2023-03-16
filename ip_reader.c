#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ip_reader.h"

/*
 * checks validity of a given address in text format,
 * called by read_ipv4_format() before translation
 * returns 0 on success and an error code on failure
 */
static int ipv4_format_invalid(char *ip4)
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

//function for checking if mask is ok
static int ipv4_mask_invalid(uint32_t mask)
{
	int i = 0;

	while ((mask >> i) % 2 != 1) {
		i++;
	}
	if (mask != (~0) << i) return MASK_INVAL;

	return SUCCESS;
}

/*
 * assigns a class to an address based on its first octet
 */
static char get_ipv4_network_class(uint32_t address)
{
	uint8_t oct1 = IPV4_OCT_1(address);

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

/*
 * translates text input into a numeric address, can be used to translate
 * an ipv4 address or mask as long as the format is valid. called by parser
 * functions.
 * returns 0 on success and an error code on failure
 */
static int read_ipv4_format(uint32_t *temp_name, char *input)
{
	if (ipv4_format_invalid(input)) return FORM_INVAL;

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

/*
 * translates a prefix mask attached to an ip address input in text form into a
 * numeric mask or infers a mask form address class. called by parser functions
 * returns 0 on success and an error code on failure
 */
static int demask_ipv4(struct ip4_address *address, char *input)
{
	int len = strlen(input);
	int i;
	int prefix;

	//find '/' if it exists in correct place inside input
	for (i = 1; i < 4 && input[len - i] != '/'; i++);

	if (i != 4) {
		prefix = strtol(input + len - i + 1, NULL, 10);
		//len - i + 1 is the exact position after '/' sign
		if (prefix > 32 || prefix < 0) {
			return MASK_INVAL;
		}

		input[len - i] = 0;	//truncate leftover address
	} else {	//assign a default mask based on class
		switch (get_ipv4_network_class(address->addr)) {
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

/*
 * determines if the scope of the address is private
 */
static int is_private_address(uint32_t address)
{
	uint8_t oct1 = IPV4_OCT_1(address);
	uint8_t oct2 = IPV4_OCT_2(address);

	if (oct1 == 10)
		return 1;
	if (oct1 == 192 && oct2 == 168)
		return 1;
	if (oct1 == 172 && oct2 > 15 && oct2 < 32)
		return 1;
	return 0;
}

int parse_ip4_short(struct ip4_address *address, char *input)
{
	if (read_ipv4_format(&address->addr, input)) {
		printf("Invalid IP address!\n");
		return ADDR_INVAL;
	}

	if (demask_ipv4(address, input)) {
		printf("Not a valid subnet mask!\n");
		return MASK_INVAL;
	}
	return SUCCESS;
}

int parse_ip4_long(struct ip4_address *address, char *addr_input, char *mask_input)
{
	if (read_ipv4_format(&address->addr, addr_input)) {
		printf("Invalid IP address!\n");
		return ADDR_INVAL;
	}

	if (read_ipv4_format(&address->mask, mask_input)) {
		printf("Not a valid subnet mask!\n");
		return MASK_INVAL;
	}

	if (ipv4_mask_invalid(address->mask)) {
		printf("Not a valid subnet mask!\n");
		return MASK_INVAL;
	}

	return SUCCESS;
	/*
	 * Alternatively, result of is_mask_ipv4_valid(address->mask) could be
	 * returned to avoid an entire conditional block, but this way it's
	 * a lot more readable the way it is now.
	 */
}

void print_ipv4_info(struct ip4_address *address)
{
	if (get_ipv4_network_class(address->addr) == 'D') {
		printf(
			"Address: "IPV4_DECIMAL_FORMAT_STRING"\n"
			"Class: %c\n",
			IPV4_DECIMAL_FORMAT_VALUES(address->addr),
			get_ipv4_network_class(address->addr));

	} else {
		char *designation[2] = {"Public", "Private"};

		//filling in missing data
		//obtain network address
		uint32_t network = address->addr & address->mask;

		//obtain network broadcast address
		uint32_t broadcast = network | ~address->mask;

		printf(
			"Address: "IPV4_DECIMAL_FORMAT_STRING"\n"
			"Mask: "IPV4_DECIMAL_FORMAT_STRING"\n"
			"Class: %c\n"
			"Network: "IPV4_DECIMAL_FORMAT_STRING"\n"
			"Broadcast: "IPV4_DECIMAL_FORMAT_STRING"\n"
			"%s Address\n",
			IPV4_DECIMAL_FORMAT_VALUES(address->addr),
			IPV4_DECIMAL_FORMAT_VALUES(address->mask),
			get_ipv4_network_class(address->addr),
			IPV4_DECIMAL_FORMAT_VALUES(network),
			IPV4_DECIMAL_FORMAT_VALUES(broadcast),
			designation[is_private_address(address->addr)]);
	}
}
