#include "pmdsprite_history.h"

void PmdspriteHistory::save(const WorkspaceUndoState &p_current) {
	undo_stack.push_back(p_current);
	if (undo_stack.size() > 50) {
		undo_stack.remove_at(0);
	}
	redo_stack.clear();
}

bool PmdspriteHistory::undo(const WorkspaceUndoState &p_current, WorkspaceUndoState &r_restored) {
	if (undo_stack.is_empty()) return false;
	redo_stack.push_back(p_current);
	r_restored = undo_stack[undo_stack.size() - 1];
	undo_stack.remove_at(undo_stack.size() - 1);
	return true;
}

bool PmdspriteHistory::redo(const WorkspaceUndoState &p_current, WorkspaceUndoState &r_restored) {
	if (redo_stack.is_empty()) return false;
	undo_stack.push_back(p_current);
	r_restored = redo_stack[redo_stack.size() - 1];
	redo_stack.remove_at(redo_stack.size() - 1);
	return true;
}