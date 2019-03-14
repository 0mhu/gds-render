

#include "layer-info.h"

void delete_layer_info_struct(struct layer_info *info)
{
	if (info)
		free(info);
}
