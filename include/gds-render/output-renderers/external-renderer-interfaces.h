#ifndef __EXTERNAL_RENDERER_INTERFACES_H__
#define __EXTERNAL_RENDERER_INTERFACES_H__

#ifndef xstr

#define xstr(a) str(a)
#define str(a) #a

#endif /* xstr */

/**
 * @addtogroup ExternalRenderer
 * @{
 */

/**
 * @brief This define is used to export a function from a shared object
 */
#define EXPORT_FUNC __attribute__((visibility("default")))

/**
 * @brief Function name expected to be found in external library for rendering.
 *
 * The function has to be defined as follows:
 * @code
 * int EXTERNAL_LIBRARY_RENDER_FUNCTION(struct gds_cell *toplevel, GList *layer_info_list, const char *output_file_name, double scale);
 * @endcode
 */
#define EXTERNAL_LIBRARY_RENDER_FUNCTION exported_render_cell_to_file

/**
 * @brief Function name expected to be found in external library for initialization.
 *
 * @code
 * int EXTERNAL_LIBRARY_INIT_FUNCTION(const char *option_string, const char *version_string);
 * @endcode
 */
#define EXTERNAL_LIBRARY_INIT_FUNCTION exported_init

/**
 * @brief Global integer specified by an external renderer to signal, that the init and render functions shall be executed in a subprocess
 *
 * The pure presence of this symbol name causes forking. The content of this variable is don't care.
 * @note Use this if you mess with the internal structures of gds-render
 */
#define EXTERNAL_LIBRARY_FORK_REQUEST exported_fork_request

/**
 * @brief Define for declaring the exported functions.
 *
 * This not only helps with the declaration but also makes the symbols visible, so they can be called form outside the library
 */
#define EXPORTED_FUNC_DECL(FUNC) EXPORT_FUNC FUNC

/** @} */

#endif /* __EXTERNAL_RENDERER_INTERFACES_H__ */
