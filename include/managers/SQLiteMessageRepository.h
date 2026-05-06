#ifndef SKILLBRIDGE_SQLITEMESSAGEREPOSITORY_H
#define SKILLBRIDGE_SQLITEMESSAGEREPOSITORY_H
#include "IMessageRepository.h"

struct sqlite3_stmt;
class SQLiteMessageRepository : public IMessageRepository {
public:
    SQLiteMessageRepository() = default;
    ~SQLiteMessageRepository() override = default;
    void saveMessage(Message& msg) override;
    bool markAsRead(int messageID) override;
    bool deleteMessage(int messageID) override;
    Message findMessageByID(int messageID) const override;
    DataList<Message> findConversation(int userA, int userB) const override;
    DataList<Message> findInbox(int receiverID) const override;
    DataList<Message> findAllMessages() const override;
    int countUnread(int receiverID) const override;
    DataList<int> findConversationPartners(int userID) const override;

private:
    Message buildMessageFromRow(sqlite3_stmt* stmt) const;
    void throwPrepareError(const std::string& sql) const;
};

#endif