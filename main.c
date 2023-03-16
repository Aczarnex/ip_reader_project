#include <stdio.h>
#include "ip_reader.h"

int main(int argc, char *argv[])
{
	struct ip4_address ip4 = {0};
	int status;

	switch (argc) {
	case 2:
		status = parse_ip4_short(&ip4, argv[1]);
		break;
	case 3:
		status = parse_ip4_long(&ip4, argv[1], argv[2]);
		break;
	default:
		printf("Invalid Input.\n");
		status = FORM_INVAL;
	}

	if (!status) print_ipv4_info(&ip4);
	return status;
}
