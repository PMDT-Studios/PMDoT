#ifndef PMDOT_VSCODE_EDITOR_DOCK_H
#define PMDOT_VSCODE_EDITOR_DOCK_H

#ifdef TOOLS_ENABLED

#include "editor/plugins/editor_plugin.h"
#include "platform/webview_backend.h"
#include "vscode_bridge.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/gui/label.h"

class VSCodeEditorDock : public VBoxContainer {
	GDCLASS(VSCodeEditorDock, VBoxContainer);

private:
	Control *webview_container = nullptr;
	Label *status_label = nullptr;
	WebViewBackend *backend = nullptr;
	VSCodeBridge bridge;
	bool prewarmed = false;

	void _ensure_backend_initialized();
	void _sync_bounds();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	VSCodeEditorDock();
	~VSCodeEditorDock();

	void prewarm();
	void set_ide_visible(bool p_visible);
	bool open_script(const String &p_res_path);
	WebViewBackend *get_backend() const { return backend; }
	VSCodeBridge *get_bridge() { return &bridge; }
};

class PMDoTVSCodeEditorPlugin : public EditorPlugin {
	GDCLASS(PMDoTVSCodeEditorPlugin, EditorPlugin);

private:
	VSCodeEditorDock *dock = nullptr;

public:
	virtual String get_name() const override { return "Code"; }
	virtual bool has_main_screen() const override { return true; }
	virtual bool handles(Object *p_object) const override;
	virtual void edit(Object *p_object) override;
	virtual void make_visible(bool p_visible) override;

	PMDoTVSCodeEditorPlugin();
	~PMDoTVSCodeEditorPlugin();
};

#endif // TOOLS_ENABLED

#endif // PMDOT_VSCODE_EDITOR_DOCK_H
