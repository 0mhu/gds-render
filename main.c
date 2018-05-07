#include <stdio.h>
#include "gdsparse.h"

int main()
{
	struct gds_library *libs;
	unsigned int count = 0;
	parse_gds_from_file("/home/mari/Desktop/test.gds", &libs, &count);

	return 0;
}
