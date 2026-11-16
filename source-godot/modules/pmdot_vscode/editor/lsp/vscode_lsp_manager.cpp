#ifdef TOOLS_ENABLED

#include "vscode_lsp_manager.h"

#include "core/io/json.h"
#include "editor/editor_settings.h"

void VSCodeLSPManager::register_editor_settings() {
	EDITOR_DEF("pmdot/vscode/theme", "vs-dark");
	EditorSettings::get_singleton()->add_property_hint(PropertyInfo(Variant::STRING, "pmdot/vscode/theme", PROPERTY_HINT_ENUM, "vs-dark,vs,hc-black"));
	EDITOR_DEF("pmdot/vscode/font_size", 14);
	EDITOR_DEF("pmdot/vscode/minimap_enabled", true);
	EDITOR_DEF("pmdot/vscode/font_ligatures", true);
	EDITOR_DEF("pmdot/vscode/lsp_enabled", true);
}

void VSCodeLSPManager::attach(WebViewBackend *p_backend) {
	backend = p_backend;
	apply_editor_settings();
}

String VSCodeLSPManager::get_server_command(LanguageTarget p_lang) const {
	switch (p_lang) {
		case LANG_CSHARP:
			return "OmniSharp";
		case LANG_CPP:
			return "clangd";
		case LANG_JAVA:
			return "jdtls";
		case LANG_MARKDOWN:
		default:
			return "";
	}
}

Dictionary VSCodeLSPManager::build_theme_config() const {
	Dictionary cfg;
	EditorSettings *es = EditorSettings::get_singleton();
	cfg["theme"] = es ? es->get("pmdot/vscode/theme") : String("vs-dark");
	cfg["fontSize"] = es ? int(es->get("pmdot/vscode/font_size")) : 14;
	cfg["minimap"] = es ? bool(es->get("pmdot/vscode/minimap_enabled")) : true;
	cfg["fontLigatures"] = es ? bool(es->get("pmdot/vscode/font_ligatures")) : true;
	return cfg;
}

void VSCodeLSPManager::apply_editor_settings() {
	if (!backend || !backend->is_ready()) {
		return;
	}
	Dictionary msg;
	msg["method"] = "apply_config";
	msg["params"] = build_theme_config();
	backend->post_message(JSON::stringify(msg));
}

#endif // TOOLS_ENABLED
