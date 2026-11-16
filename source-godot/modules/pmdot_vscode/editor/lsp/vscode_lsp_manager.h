#ifndef PMDOT_VSCODE_LSP_MANAGER_H
#define PMDOT_VSCODE_LSP_MANAGER_H

#ifdef TOOLS_ENABLED

#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
#include "platform/webview_backend.h"

class VSCodeLSPManager {
public:
	enum LanguageTarget {
		LANG_CSHARP,
		LANG_CPP,
		LANG_JAVA,
		LANG_MARKDOWN
	};

private:
	WebViewBackend *backend = nullptr;
	bool lsp_enabled = true;

public:
	static void register_editor_settings();
	void attach(WebViewBackend *p_backend);
	void apply_editor_settings();

	String get_server_command(LanguageTarget p_lang) const;
	Dictionary build_theme_config() const;
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_LSP_MANAGER_H
