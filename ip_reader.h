#include <stdint.h>

#define IPV4_OCT_1(ip) ((ip) >> 24)
#define IPV4_OCT_2(ip) IPV4_OCT_1((ip) << 8)
#define IPV4_OCT_3(ip) IPV4_OCT_1((ip) << 16)
#define IPV4_OCT_4(ip) IPV4_OCT_1((ip) << 24)
#define IPV4_DECIMAL_FORMAT_STRING "%u.%u.%u.%u"
#define IPV4_DECIMAL_FORMAT_VALUES(ip) IPV4_OCT_1(ip),IPV4_OCT_2(ip),IPV4_OCT_3(ip),IPV4_OCT_4(ip)

enum return_flags {SUCCESS, ADDR_INVAL, MASK_INVAL, FORM_INVAL};

struct ip4_address {
	uint32_t addr;
	uint32_t mask;
};

/*
 * prints to standard output relevant information that can be inferred form
 * a given ip address and the optionally provided mask
 */
void print_ipv4_info(struct ip4_address *);

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
int parse_ip4_long(struct ip4_address *, char *, char *);
