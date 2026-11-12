#pragma once
#include "scene/gui/control.h"

class ReactUI : public Control{
    GDCLASS(ReactUI, Control)

    String bundle_path;


protected:
    void _ready() override;
    void _process(double delta) override;
    void _draw() override;
    static void _bind_methods();

public:
    void set_bundle_path(const String &p_path);
    void get_bundle_path() const;
};