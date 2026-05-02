#ifndef SKILLBRIDGE_COMMANDHISTORY_H
#define SKILLBRIDGE_COMMANDHISTORY_H

#include "ICommand.h"
#include "../utils/DataList.h"


class CommandHistory {
private:
    static constexpr int MAX_HISTORY = 10;
   DataList<ICommand*> undoStack_;
    DataList<ICommand*> redoStack_;
    void clearStack(DataList<ICommand*>& stack);
    void evictBottomIfOverCap(DataList<ICommand*>& stack);

public:
    CommandHistory() = default;
    ~CommandHistory();
    CommandHistory(const CommandHistory&) = delete;
    CommandHistory& operator=(const CommandHistory&) = delete;

      void recordAndExecute(ICommand* cmd);
    bool canUndo() const
    { 
        return undoStack_.size() > 0;
    }
    bool canRedo() const 
    {
        return redoStack_.size() > 0;
    }
    int undoSize() const 
    {
        return undoStack_.size();
    }
    int redoSize() const 
    {
        return redoStack_.size(); 
    }
   void undo();
  void redo();
   std::string peekUndoDescription() const;
    std::string peekRedoDescription() const;
    void clear();
};

#endif