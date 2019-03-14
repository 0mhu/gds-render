

#include "layer-info.h"

void layer_info_delete_struct(struct layer_info *info)
{
	if (info)
		free(info);
}
