#ifndef PMDSPRITE_EXPORTER_H
#define PMDSPRITE_EXPORTER_H

#include "core/io/image.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

class PmdspriteExporter {
public:
	static Error export_pages(const Vector<Ref<Image>> &p_pages, int p_current_index, const String &p_dir_path, const String &p_sprite_name);
};

#endif