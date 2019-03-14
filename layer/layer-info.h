#ifndef _LAYER_INFO_H_
#define _LAYER_INFO_H_

#include <gtk/gtk.h>

/**
 * @brief Layer information.
 *
 * This structs contains information on how to render a layer
 */
struct layer_info
{
	int layer; /**< @brief Layer number */
	char *name; /**< @brief Layer name */
	int stacked_position; ///< @brief Position of layer in output @warning This parameter is not used by any renderer so far @note Lower is bottom, higher is top
	GdkRGBA color; /**< @brief RGBA color used to render this layer */
};

/**
 * @brief Delete a layer_info struct
 * @param info Struct to be deleted.
 * @note The layer_info::name Element has to be freed manually
 */
void layer_info_delete_struct(struct layer_info *info);

#endif // _LAYER_INFO_H_
