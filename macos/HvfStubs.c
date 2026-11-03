// Homebrew's freetype 2.15 static library (libfreetype.a) calls libhvf for Apple's HVF variable-font format, but Homebrew ships no
// libhvf, so a static link leaves these symbols undefined and the dylib would fail to load. CET only renders its own bundled TrueType
// fonts and never opens an HVF font, so these stubs are never reached; if one ever were, it reports failure and freetype skips the font.
// (Claude, 2026-10-23, for the 2.5 rebuild.)
#include <stddef.h>
void *HVF_open_part_renderer(void *a, void *b, void *c, void *d) { (void)a; (void)b; (void)c; (void)d; return NULL; }
void HVF_close_part_renderer(void *r) { (void)r; }
size_t HVF_part_renderer_storage_size(void) { return 0; }
void HVF_clear_part_cache(void *r) { (void)r; }
int HVF_set_render_part(void *r, unsigned int p) { (void)r; (void)p; return -1; }
int HVF_render_part_axis_count(void *r) { (void)r; return 0; }
int HVF_set_axis_value(void *r, unsigned int i, float v) { (void)r; (void)i; (void)v; return -1; }
int HVF_render_current_part(void *r, void *a, void *b) { (void)r; (void)a; (void)b; return -1; }
