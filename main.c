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
		status = parse_ip4_long(&ip4, argv);
		break;
	default:
		printf("Invalid Input.\n");
		status = 1;
	}

	if (!status) get_info(&ip4);
	return status;
}
