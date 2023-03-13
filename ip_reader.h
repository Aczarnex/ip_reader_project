#include <stdint.h>


struct ip4_address {
	uint32_t addr;
	uint32_t mask;
};

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
int read_data(uint32_t *, char *);

/*
 * translates a prefix mask attached to an ip address input in text form into a
 * numeric mask or infers a mask form address class.
 * returns 0 on success and an error code on failure
 */
int demask(struct ip4_address *, char *);

/*
 * assigns a class to an address based on its first octet
 */
char get_network_class(uint32_t);

/*
 * determines the scope of the address between private (1) and public (0)
 */
int get_scope(uint32_t);

/*
 * prints to standard output relevant information that can be inferred form
 * a given ip address and the optionally provided mask
 */
void get_info(struct ip4_address *);

int parse_ip4_1(struct ip4_address *, char *);
int parse_ip4_2(struct ip4_address *, char *[]);