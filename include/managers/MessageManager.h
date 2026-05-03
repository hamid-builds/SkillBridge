#ifndef SKILLBRIDGE_MESSAGEMANAGER_H
#define SKILLBRIDGE_MESSAGEMANAGER_H

#include "IMessageRepository.h"
#include "IUserRepository.h"
#include "../core/Message.h"
#include "../core/UserRole.h"
#include "../utils/DataList.h"
#include "../utils/HuffmanCoder.h"
#include "../utils/LRUCache.h"

#include <string>


struct DecodedMessage {
    int         messageID;
    int         senderID;
    int         receiverID;
    std::string text;          
    std::string timestamp;
    bool        isRead;

    DecodedMessage()
        : messageID(0), senderID(0), receiverID(0),
        text(""), timestamp(""), isRead(false) {
    }
};


class MessageManager {
private:
    IMessageRepository& messageRepo_;
    IUserRepository& userRepo_;
    HuffmanCoder& coder_;

    static constexpr std::size_t CACHE_CAPACITY = 32;
    mutable LRUCache<std::string, DataList<DecodedMessage>> conversationCache_;

    std::string currentTimestamp() const;

    std::string conversationKey(int a, int b) const;

    User* loadUser(int userID) const;

    void requireSelfOrAdmin(int callerID, int expectedUserID) const;

    void requireParticipantOrAdmin(int callerID, int userA, int userB) const;

   
    DecodedMessage decodeOne(const Message& m) const;

   
    DataList<DecodedMessage> decodeAll(const DataList<Message>& msgs) const;

public:
    MessageManager(IMessageRepository& messageRepo,
        IUserRepository& userRepo,
        HuffmanCoder& coder);

    
    MessageManager(const MessageManager&) = delete;
    MessageManager& operator=(const MessageManager&) = delete;

    Message sendMessage(int currentUserID,
        int senderID,
        int receiverID,
        const std::string& text);

   
    bool markAsRead(int currentUserID, int messageID);

    int markConversationRead(int currentUserID,
        int receiverID,
        int otherUserID);

    DataList<DecodedMessage> getConversation(int currentUserID,
        int viewerID,
        int otherUserID);

   
    DataList<DecodedMessage> getInbox(int currentUserID, int receiverID);

    int countUnread(int currentUserID, int receiverID);

    void clearCache();

    std::size_t cacheSize() const { return conversationCache_.size(); }
};

#endif