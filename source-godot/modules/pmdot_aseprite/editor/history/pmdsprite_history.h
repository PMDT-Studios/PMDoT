#ifndef PMDSPRITE_HISTORY_H
#define PMDSPRITE_HISTORY_H

#include "workspace_undo_state.h"

class PmdspriteHistory {
	Vector<WorkspaceUndoState> undo_stack;
	Vector<WorkspaceUndoState> redo_stack;

public:
	void save(const WorkspaceUndoState &p_current);
	bool undo(const WorkspaceUndoState &p_current, WorkspaceUndoState &r_restored);
	bool redo(const WorkspaceUndoState &p_current, WorkspaceUndoState &r_restored);
};
#endif