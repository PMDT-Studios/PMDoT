#ifndef WORKSPACE_UNDO_STATE_H
#define WORKSPACE_UNDO_STATE_H

#include "core/io/image.h"
#include "core/templates/vector.h"

struct WorkspaceUndoState {
	int width = 0;
	int height = 0;
	int page_index = 0;
	Vector<Ref<Image>> pages_backup;
};

#endif