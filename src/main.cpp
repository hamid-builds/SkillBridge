
#include <iostream>
#include <string>
#include "managers/ICommand.h"
#include "managers/CommandHistory.h"
#include "core/Exceptions.h"

using namespace std;

static int testsRun = 0;
static int testsPassed = 0;

#define CHECK(cond, label) do { \
    ++testsRun; \
    if (cond) { ++testsPassed; cout << "  PASS: " << label << "\n"; } \
    else { cout << "  FAIL: " << label << "\n"; } \
} while(0)

#define CHECK_THROWS(expr, ExType, label) do { \
    ++testsRun; \
    bool caught = false; \
    try { expr; } catch (const ExType&) { caught = true; } catch (...) { } \
    if (caught) { ++testsPassed; cout << "  PASS: " << label << "\n"; } \
    else { cout << "  FAIL: " << label << "\n"; } \
} while(0)


static int    g_destructed_count = 0;

class FakeCommand : public ICommand {
private:
    int id_;
    bool throwOnExecute_;
    bool throwOnUndo_;
public:
    FakeCommand(int id, bool throwOnExec = false, bool throwOnUndo = false)
        : id_(id), throwOnExecute_(throwOnExec), throwOnUndo_(throwOnUndo) {
    }

    ~FakeCommand() override {
        ++g_destructed_count;
    }

    void execute() override {
        if (throwOnExecute_) {
            throw ValidationException("fake-execute-fail-" + to_string(id_));
        }
        g_log += "E" + to_string(id_) + " ";
    }

    void undo() override {
        if (throwOnUndo_) {
            throw ValidationException("fake-undo-fail-" + to_string(id_));
        }
        g_log += "U" + to_string(id_) + " ";
    }

    string description() const override {
        return "FakeCommand#" + to_string(id_);
    }
};

static void resetLog() {
    g_log.clear();
    g_destructed_count = 0;
}


static void testBasicExecute() {
    cout << "\n[1] recordAndExecute basic\n";
    resetLog();
    {
        CommandHistory h;
        CHECK(h.canUndo() == false, "fresh history: canUndo = false");
        CHECK(h.canRedo() == false, "fresh history: canRedo = false");
        CHECK(h.undoSize() == 0, "fresh history: undoSize = 0");
        CHECK(h.redoSize() == 0, "fresh history: redoSize = 0");

        h.recordAndExecute(new FakeCommand(1));
        CHECK(g_log == "E1 ", "execute called once");
        CHECK(h.canUndo() == true, "after one record: canUndo = true");
        CHECK(h.canRedo() == false, "after one record: canRedo = false");
        CHECK(h.undoSize() == 1, "undoSize = 1");
        CHECK(h.peekUndoDescription() == "FakeCommand#1", "peek shows command 1");

        h.recordAndExecute(new FakeCommand(2));
        CHECK(g_log == "E1 E2 ", "second execute called");
        CHECK(h.undoSize() == 2, "undoSize = 2");
        CHECK(h.peekUndoDescription() == "FakeCommand#2", "peek shows command 2 (most recent)");
    }
    
    CHECK(g_destructed_count == 2, "destructor deleted both commands");
}


static void testNullRejection() {
    cout << "\n[2] null command rejection\n";
    CommandHistory h;
    CHECK_THROWS(h.recordAndExecute(nullptr), ValidationException,
        "recordAndExecute(nullptr) throws ValidationException");
    CHECK(h.undoSize() == 0, "history unchanged after null");
}

static void testUndoRedoBasic() {
    cout << "\n[3] undo/redo basic flow\n";
    resetLog();
    {
        CommandHistory h;
        h.recordAndExecute(new FakeCommand(1));
        h.recordAndExecute(new FakeCommand(2));
        h.recordAndExecute(new FakeCommand(3));
        CHECK(g_log == "E1 E2 E3 ", "three executes ran");

        h.undo();
        CHECK(g_log == "E1 E2 E3 U3 ", "undo called on most-recent (cmd 3)");
        CHECK(h.undoSize() == 2, "undoSize back to 2");
        CHECK(h.redoSize() == 1, "redoSize is 1");
        CHECK(h.canRedo() == true, "canRedo = true");
        CHECK(h.peekRedoDescription() == "FakeCommand#3", "redo top is cmd 3");
        CHECK(h.peekUndoDescription() == "FakeCommand#2", "undo top is cmd 2");

        h.undo();
        CHECK(g_log == "E1 E2 E3 U3 U2 ", "undo called on cmd 2");
        CHECK(h.undoSize() == 1, "undoSize is 1");
        CHECK(h.redoSize() == 2, "redoSize is 2");

        h.redo();
        CHECK(g_log == "E1 E2 E3 U3 U2 E2 ", "redo re-executes cmd 2");
        CHECK(h.undoSize() == 2, "undoSize back to 2");
        CHECK(h.redoSize() == 1, "redoSize back to 1");

        h.redo();
        CHECK(g_log == "E1 E2 E3 U3 U2 E2 E3 ", "redo re-executes cmd 3");
        CHECK(h.undoSize() == 3, "undoSize back to 3");
        CHECK(h.redoSize() == 0, "redoSize is 0");
    }
    CHECK(g_destructed_count == 3, "all 3 commands deleted by destructor");
}


static void testEmptyStackNoop() {
    cout << "\n[4] undo/redo on empty stacks are no-ops\n";
    resetLog();
    CommandHistory h;
    h.undo();   // should not throw
    h.redo();   // should not throw
    CHECK(g_log == "", "no calls were made");
    CHECK(h.canUndo() == false, "still cannot undo");
    CHECK(h.canRedo() == false, "still cannot redo");
    CHECK(h.peekUndoDescription() == "", "peek empty undo returns empty string");
    CHECK(h.peekRedoDescription() == "", "peek empty redo returns empty string");
}

static void testNewActionClearsRedo() {
    cout << "\n[5] new action clears redo stack\n";
    resetLog();
    {
        CommandHistory h;
        h.recordAndExecute(new FakeCommand(1));
        h.recordAndExecute(new FakeCommand(2));
        h.recordAndExecute(new FakeCommand(3));
        h.undo();
        h.undo();
        CHECK(h.redoSize() == 2, "redo has 2 entries before new action");

        // New action: should clear redo stack (and delete cmds 2 and 3)
        int destructedBefore = g_destructed_count;
        h.recordAndExecute(new FakeCommand(99));
        CHECK(h.redoSize() == 0, "redo stack cleared after new action");
        CHECK(g_destructed_count - destructedBefore == 2,
            "two redo-stack commands deleted on clear");
        CHECK(h.canRedo() == false, "canRedo = false after new action");
    }
}


static void testCapEnforcement() {
    cout << "\n[6] cap enforcement at MAX_HISTORY = 10\n";
    resetLog();
    {
        CommandHistory h;
        
        for (int i = 1; i <= 12; ++i) {
            h.recordAndExecute(new FakeCommand(i));
        }
        CHECK(h.undoSize() == 10, "undo cap enforced at 10");
        CHECK(g_destructed_count == 2, "2 oldest commands deleted on eviction");

        
        CHECK(h.peekUndoDescription() == "FakeCommand#12", "top is cmd 12");

       
        string expected = "";
        for (int i = 1; i <= 12; ++i) expected += "E" + to_string(i) + " ";
        for (int i = 12; i >= 3; --i) expected += "U" + to_string(i) + " ";

        for (int i = 0; i < 10; ++i) h.undo();
        CHECK(g_log == expected, "undo drains 10 commands in reverse order (12..3)");
        CHECK(h.canUndo() == false, "undo stack is empty");
        CHECK(h.redoSize() == 10, "redo has 10 entries");
    }
}


static void testExecuteThrows() {
    cout << "\n[7] execute() throw deletes command, does not push\n";
    resetLog();
    {
        CommandHistory h;
        h.recordAndExecute(new FakeCommand(1)); 

        FakeCommand* bad = new FakeCommand(99, /*throwOnExec=*/true);
        CHECK_THROWS(h.recordAndExecute(bad), ValidationException,
            "execute() throw propagates");

       
        CHECK(g_destructed_count == 1, "throwing command deleted");
        CHECK(h.undoSize() == 1, "undoSize still 1 (bad command not pushed)");
        CHECK(h.peekUndoDescription() == "FakeCommand#1", "still shows cmd 1 on top");
    }
    
    CHECK(g_destructed_count == 2, "all commands accounted for");
}

static void testRedoExecuteThrows() {
    cout << "\n[8] redo when re-execute throws: command deleted, rethrown\n";
    resetLog();
    {
        CommandHistory h;
        
        class OneShotFail : public ICommand {
            int callCount_ = 0;
        public:
            void execute() override {
                ++callCount_;
                if (callCount_ > 1) throw ValidationException("redo-fail");
                g_log += "OS-E ";
            }
            void undo() override { g_log += "OS-U "; }
            string description() const override { return "OneShotFail"; }
            ~OneShotFail() override { ++g_destructed_count; }
        };

        h.recordAndExecute(new OneShotFail());
        h.undo();
        CHECK(g_log == "OS-E OS-U ", "one execute, one undo so far");

        CHECK_THROWS(h.redo(), ValidationException, "redo throw propagates");
        CHECK(h.redoSize() == 0, "command was popped from redo stack");
        CHECK(h.undoSize() == 0, "command not pushed back to undo stack");
        CHECK(g_destructed_count == 1, "command deleted on redo failure");
    }
}


static void testClear() {
    cout << "\n[9] clear() wipes both stacks\n";
    resetLog();
    {
        CommandHistory h;
        h.recordAndExecute(new FakeCommand(1));
        h.recordAndExecute(new FakeCommand(2));
        h.recordAndExecute(new FakeCommand(3));
        h.undo();
        CHECK(h.undoSize() == 2 && h.redoSize() == 1, "before clear: 2 undo, 1 redo");

        h.clear();
        CHECK(h.undoSize() == 0, "after clear: undo empty");
        CHECK(h.redoSize() == 0, "after clear: redo empty");
        CHECK(h.canUndo() == false && h.canRedo() == false,
            "after clear: cannot undo or redo");
        CHECK(g_destructed_count == 3, "all 3 commands deleted by clear");
    }
}


static void testStressUndoRedo() {
    cout << "\n[10] stress: fill, drain, refill\n";
    resetLog();
    {
        CommandHistory h;
        for (int i = 1; i <= 10; ++i) h.recordAndExecute(new FakeCommand(i));
        CHECK(h.undoSize() == 10, "10 commands recorded");

        for (int i = 0; i < 10; ++i) h.undo();
        CHECK(h.undoSize() == 0 && h.redoSize() == 10, "all undone");

        for (int i = 0; i < 10; ++i) h.redo();
        CHECK(h.undoSize() == 10 && h.redoSize() == 0, "all redone");

        
        string expected = "";
        for (int i = 1; i <= 10; ++i) expected += "E" + to_string(i) + " ";
        for (int i = 10; i >= 1; --i) expected += "U" + to_string(i) + " ";
        for (int i = 1; i <= 10; ++i) expected += "E" + to_string(i) + " ";
        CHECK(g_log == expected, "call order matches expected sequence");
    }
    CHECK(g_destructed_count == 10, "all 10 commands deleted on destruction");
}

int main() {
    cout << "=== Module 3 - Step 4: ICommand + CommandHistory ===\n";

    try {
        testBasicExecute();
        testNullRejection();
        testUndoRedoBasic();
        testEmptyStackNoop();
        testNewActionClearsRedo();
        testCapEnforcement();
        testExecuteThrows();
        testRedoExecuteThrows();
        testClear();
        testStressUndoRedo();
    }
    catch (const exception& e) {
        cout << "\nFATAL: uncaught: " << e.what() << "\n";
        return 2;
    }

    cout << "\n=== Summary: " << testsPassed << " / " << testsRun
        << " tests passed ===\n";
    return (testsPassed == testsRun) ? 0 : 1;
}