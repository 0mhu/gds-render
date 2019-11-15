#include <stdio.h>
#include <glib.h>
#include <gds-render/gds-utils/gds-types.h>
#include <gds-render/output-renderers/external-renderer-interfaces.h>

int FUNC_DECL(EXTERNAL_LIBRARY_RENDER_FUNCTION)(struct gds_cell *toplevel, GList *layer_info_list, const char *output_file_name, double scale)
{
	if (!toplevel)
		return -1000;

	printf("Rendering %s\n", toplevel->name);
	return 0;
}

int FUNC_DECL(EXTERNAL_LIBRARY_INIT_FUNCTION)(const char *params, const char *version)
{
	printf("Init with params: %s\ngds-render version: %s\n", params, version);
	return 0;
}
