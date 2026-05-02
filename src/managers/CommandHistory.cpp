#include "managers/CommandHistory.h"
#include "core/Exceptions.h"

using namespace std;


void CommandHistory::clearStack(DataList<ICommand*>& stack) {
    
    for (int i = 0; i < stack.size(); ++i) {
        delete stack.get(i);
    }
    stack.clear();
}

void CommandHistory::evictBottomIfOverCap(DataList<ICommand*>& stack) {
    
    while (stack.size() > MAX_HISTORY) {
        ICommand* oldest = stack.get(0);
        stack.removeAt(0);
        delete oldest;
    }
}

CommandHistory::~CommandHistory() {
    
    clearStack(undoStack_);
    clearStack(redoStack_);
}

void CommandHistory::recordAndExecute(ICommand* cmd) {
    if (cmd == nullptr) {
        throw ValidationException("CommandHistory: cannot record null command");
    }

   
    try {
        cmd->execute();
    }
    catch (...) {
        delete cmd;
        throw;
    }

  
    undoStack_.add(cmd);
    clearStack(redoStack_);
    evictBottomIfOverCap(undoStack_);
}

void CommandHistory::undo() {
    if (undoStack_.size() == 0) {
        
        return;
    }

    int top = undoStack_.size() - 1;
    ICommand* cmd = undoStack_.get(top);
    undoStack_.removeAt(top);

   
    try {
        cmd->undo();
    }
    catch (...) 
    {
        delete cmd;
        throw;
    }

    redoStack_.add(cmd);
    evictBottomIfOverCap(redoStack_);
}

void CommandHistory::redo() {
    if (redoStack_.size() == 0) {
        return;
    }

    int top = redoStack_.size() - 1;
    ICommand* cmd = redoStack_.get(top);
    redoStack_.removeAt(top);

    try 
    {
        cmd->execute();
    }
    catch (...) {
        delete cmd;
        throw;
    }

    undoStack_.add(cmd);
    evictBottomIfOverCap(undoStack_);
}

string CommandHistory::peekUndoDescription() const {
    if (undoStack_.size() == 0) return "";
    return undoStack_.get(undoStack_.size() - 1)->description();
}

string CommandHistory::peekRedoDescription() const {
    if (redoStack_.size() == 0) return "";
    return redoStack_.get(redoStack_.size() - 1)->description();
}

void CommandHistory::clear() {
    clearStack(undoStack_);
    clearStack(redoStack_);
}