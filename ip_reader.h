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
 * an ipv4 address or mask as long as the format is valid. called by parser
 * functions.]
 * returns 0 on success and an error code on failure
 */
int read_data(uint32_t *, char *);

/*
 * translates a prefix mask attached to an ip address input in text form into a
 * numeric mask or infers a mask form address class. called by parser functions
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

/*
 * used to parse and translate an ip address given as input in form of a
 * character array to an internal format. to be used with a single argument,
 * with or without a prefix. if a mask is provided as a separate argument,
 * use parse_ip4_long
 * returns 0 on success and an error code on failure
 */
int parse_ip4_short(struct ip4_address *, char *);

/*
 * used to parse and translate an ip address given as input in form of a
 * character array to an internal format. to be used with multiple arguments,
 * with mask provided in decimal format. if only one argument is provided,
 * use parse_ip4_short
 * returns 0 on success and an error code on failure
 */
int parse_ip4_long(struct ip4_address *, char *[]);