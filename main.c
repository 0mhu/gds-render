#include <stdio.h>
#include "gdsparse.h"

int main()
{
	struct gds_library *libs;
	unsigned int count = 0;
	int i = 0;

	parse_gds_from_file("/home/mari/Desktop/test.gds", &libs, &count);

	printf("Lib count: %u\n", count);
	for (i = 0; i < count; i++) {
		printf("Lib %s: %d cells\n", libs[i].name, libs[i].cells_count);
	}
	return 0;
}
