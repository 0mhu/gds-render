#include <stdio.h>
#include "gdsparse.h"


int main()
{
	GList *libs = NULL;
	unsigned int count = 0;
	int i = 0;

	parse_gds_from_file("/home/mari/Desktop/test.gds", &libs);

	return 0;
}
