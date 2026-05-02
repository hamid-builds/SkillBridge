#ifndef SKILLBRIDGE_ICOMMAND_H
#define SKILLBRIDGE_ICOMMAND_H

#include <string>

class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string description() const = 0;
};

#endif