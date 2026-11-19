#ifndef PMDOT_REACTUI_UI_TREE_H
#define PMDOT_REACTUI_UI_TREE_H

#include "./Nodes/ui_node.h"

class UiTree{

protected:
    void create_instance()

public:
    void set_bundle_path(const String &p_path);
    void get_bundle_path() const;
};


#endif // PMDOT_REACTUI_UI_TREE_H
