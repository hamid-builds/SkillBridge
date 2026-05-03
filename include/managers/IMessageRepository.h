#pragma once
#ifndef SKILLBRIDGE_IMESSAGEREPOSITORY_H
#define SKILLBRIDGE_IMESSAGEREPOSITORY_H
#include "../core/Message.h"
#include "../utils/DataList.h"

class IMessageRepository {
public:
    virtual ~IMessageRepository() = default;
    virtual void saveMessage(Message& msg) = 0;
    virtual bool markAsRead(int messageID) = 0;
    virtual bool deleteMessage(int messageID) = 0;
    virtual Message findMessageByID(int messageID) const = 0;
    virtual DataList<Message> findConversation(int userA, int userB) const = 0;
    virtual DataList<Message> findInbox(int receiverID) const = 0;
    virtual DataList<Message> findAllMessages() const = 0;
    virtual int countUnread(int receiverID) const = 0;
};

#endif