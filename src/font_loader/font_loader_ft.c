/*
 * font_loader_ft.c
 *
 *  Created on: 2018��12��26��
 *      Author: zjm09
 */

#ifdef WITH_FT_FONT
#ifdef USE_SYSTEM_FREETYPE
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftpfr.h>
#include <freetype/ftadvanc.h>
#else
/*3rd/freetype_single_header/freetype.h*/
#include "freetype_single_header/freetype.h"
#endif /*USE_SYSTEM_FREETYPE*/

#include "tkc/mem.h"
#include "base/glyph_cache.h"
#include "font_loader/font_loader_ft.h"

typedef struct {
  FT_Face face;
  FT_Library library;
} ft_fontinfo;

typedef struct _font_ft_t {
  font_t base;
  ft_fontinfo ft_font;
  glyph_cache_t cache;
} font_ft_t;

static bool_t font_ft_match(font_t* f, const char* name, uint16_t font_size) {
  (void)font_size;
  return (name == NULL || strcmp(name, f->name) == 0);
}

static ret_t font_ft_find_glyph(font_t* f, wchar_t c, glyph_t* g, uint16_t font_size) {
  font_ft_t* font = (font_ft_t*)f;
  ft_fontinfo* sf = &(font->ft_font);
  FT_Glyph glyph;
  FT_GlyphSlot glyf;

  g->data = NULL;

  if (glyph_cache_lookup(&(font->cache), c, font_size, g) == RET_OK) {
    return RET_OK;
  }

  FT_Set_Char_Size(sf->face, 0, font_size * 64, 0, 50);

  if (!FT_Load_Char(sf->face, c, FT_LOAD_DEFAULT | FT_LOAD_RENDER)) {
    glyf = sf->face->glyph;
    FT_Get_Glyph(glyf, &glyph);
    g->data = glyf->bitmap.buffer;
    g->h = glyf->bitmap.rows;
    g->w = glyf->bitmap.width;
    g->x = glyf->bitmap_left;
    g->y = -glyf->bitmap_top;
    g->advance = glyf->metrics.horiAdvance / 64;
  }

  glyph_cache_add(&(font->cache), c, font_size, g);

  return g->data != NULL ? RET_OK : RET_NOT_FOUND;
}

static int32_t font_ft_get_baseline(font_t* f, uint16_t font_size) {
  font_ft_t* font = (font_ft_t*)f;
  ft_fontinfo* sf = &(font->ft_font);

  FT_Set_Char_Size(sf->face, 0, font_size * 64, 0, 50);

  int ascender = FT_MulFix(sf->face->ascender, sf->face->size->metrics.y_scale);

  return ascender >> 6;
}

static ret_t font_ft_destroy(font_t* f) {
  font_ft_t* font = (font_ft_t*)f;
  glyph_cache_deinit(&(font->cache));

  FT_Done_FreeType(font->ft_font.library);

  TKMEM_FREE(f);

  return RET_OK;
}

static ret_t destroy_glyph(void* data) {
  glyph_t* g = (glyph_t*)data;
  if (g->data) {
    // TKMEM_FREE(g->data);
  }

  return RET_OK;
}

font_t* font_ft_create(const char* name, const uint8_t* buff, uint32_t size) {
  font_ft_t* f = NULL;
  return_value_if_fail(buff != NULL && name != NULL, NULL);

  f = TKMEM_ZALLOC(font_ft_t);
  return_value_if_fail(f != NULL, NULL);

  if (FT_Init_FreeType(&f->ft_font.library)) {
    TKMEM_FREE(f);
    return NULL;
  }

  if (FT_New_Memory_Face(f->ft_font.library, buff, size, 0, &f->ft_font.face)) {
    FT_Done_FreeType(f->ft_font.library);
    TKMEM_FREE(f);
    return NULL;
  }

  if (FT_Select_Charmap(f->ft_font.face, FT_ENCODING_UNICODE)) {
    FT_Select_Charmap(f->ft_font.face, FT_ENCODING_ADOBE_LATIN_1);
  }

  f->base.name = name;
  f->base.match = font_ft_match;
  f->base.find_glyph = font_ft_find_glyph;
  f->base.get_baseline = font_ft_get_baseline;
  f->base.destroy = font_ft_destroy;

  glyph_cache_init(&(f->cache), 256, destroy_glyph);

  return &(f->base);
}

static font_t* font_ft_load(font_loader_t* loader, const char* name, const uint8_t* buff,
                            uint32_t buff_size) {
  (void)loader;

  return font_ft_create(name, buff, buff_size);
}

font_loader_t* font_loader_ft(void) {
  static font_loader_t loader;
  loader.type = ASSET_TYPE_FONT_TTF;
  loader.load = font_ft_load;

  return &loader;
}

#endif /*WITH_FT_FONT*/
