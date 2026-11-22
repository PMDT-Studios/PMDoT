#include "pmdsprite_exporter.h"
#include "core/io/dir_access.h"

Error PmdspriteExporter::export_pages(const Vector<Ref<Image>> &p_pages, int p_current_index, const String &p_dir_path, const String &p_sprite_name) {
	String sname = p_sprite_name.strip_edges();
	if (sname.is_empty()) sname = "sprite_01";
	if (sname.ends_with(".png")) sname = sname.left(sname.length() - 4);

	String dir_path = p_dir_path.strip_edges();
	if (!dir_path.ends_with("/")) dir_path += "/";

	String base_dir = dir_path.replace("res://", "");
	Ref<DirAccess> da = DirAccess::open("res://");
	if (da.is_valid()) {
		da->make_dir_recursive(base_dir);
	}

	String main_export_path = dir_path + sname + ".png";
	Error err = p_pages[p_current_index]->save_png(main_export_path);

	if (p_pages.size() > 1) {
		for (int i = 0; i < p_pages.size(); ++i) {
			if (p_pages[i].is_valid()) {
				String page_path = dir_path + sname + vformat("_page_%d.png", i + 1);
				p_pages[i]->save_png(page_path);
			}
		}
	}
	return err;
}