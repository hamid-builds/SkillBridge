#include "managers/MessageManager.h"
#include "core/Exceptions.h"
#include "core/User.h"

#include <chrono>
#include <ctime>
#include <cstdio>
#include <string>

using namespace std;


MessageManager::MessageManager(IMessageRepository& messageRepo,
    IUserRepository& userRepo,
    HuffmanCoder& coder)
    : messageRepo_(messageRepo),
    userRepo_(userRepo),
    coder_(coder),
    conversationCache_(CACHE_CAPACITY)
{
}




string MessageManager::currentTimestamp() const {
   
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm local{};
#ifdef _WIN32
    localtime_s(&local, &t);
#else
    localtime_r(&t, &local);
#endif
    char buf[64];
    snprintf(buf, sizeof(buf),
        "%04d-%02d-%02d %02d:%02d:%02d",
        local.tm_year + 1900,
        local.tm_mon + 1,
        local.tm_mday,
        local.tm_hour,
        local.tm_min,
        local.tm_sec);
    return string(buf);
}

string MessageManager::conversationKey(int a, int b) const {
    int low = (a < b) ? a : b;
    int high = (a < b) ? b : a;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d:%d", low, high);
    return string(buf);
}

User* MessageManager::loadUser(int userID) const {
    User* u = userRepo_.findUserByID(userID);
    if (u == nullptr) {
        throw ValidationException(
            "MessageManager: no user with ID " + to_string(userID));
    }
    return u;
}

void MessageManager::requireSelfOrAdmin(int callerID, int expectedUserID) const {
    if (callerID == expectedUserID) return;

    User* caller = loadUser(callerID);
    UserRole role;
    try {
        role = caller->getRole();
    }
    catch (...) {
        delete caller;
        throw;
    }
    delete caller;

    if (role != UserRole::ADMIN) {
        throw UnauthorizedException(
            "MessageManager: caller " + to_string(callerID) +
            " is not authorized to act for user " + to_string(expectedUserID));
    }
}

void MessageManager::requireParticipantOrAdmin(int callerID,
    int userA, int userB) const {
    if (callerID == userA || callerID == userB) return;

    User* caller = loadUser(callerID);
    UserRole role;
    try {
        role = caller->getRole();
    }
    catch (...) {
        delete caller;
        throw;
    }
    delete caller;

    if (role != UserRole::ADMIN) {
        throw UnauthorizedException(
            "MessageManager: caller " + to_string(callerID) +
            " is not a participant of conversation between " +
            to_string(userA) + " and " + to_string(userB));
    }
}

DecodedMessage MessageManager::decodeOne(const Message& m) const {
    DecodedMessage out;
    out.messageID = m.getMessageID();
    out.senderID = m.getSenderID();
    out.receiverID = m.getReceiverID();
    out.timestamp = m.getTimestamp();
    out.isRead = m.getIsRead();

    EncodedMessage enc;
    enc.bits = m.getPayloadBlob();
    enc.bitCount = m.getPayloadBits();
    enc.serializedTree = m.getTreeBlob();
    enc.treeBitCount = m.getTreeBits();
    out.text = coder_.decode(enc);
    return out;
}

DataList<DecodedMessage>
MessageManager::decodeAll(const DataList<Message>& msgs) const {
    DataList<DecodedMessage> out;
    for (int i = 0; i < msgs.size(); ++i) {
        out.add(decodeOne(msgs[i]));
    }
    return out;
}


Message MessageManager::sendMessage(int currentUserID,
    int senderID,
    int receiverID,
    const string& text) {
    
    if (currentUserID != senderID) {
        throw UnauthorizedException(
            "MessageManager: cannot send messages on behalf of another user");
    }

   
    bool hasNonSpace = false;
    for (char c : text) {
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            hasNonSpace = true;
            break;
        }
    }
    if (!hasNonSpace) {
        throw ValidationException(
            "MessageManager: message text cannot be empty");
    }

    
    if (senderID == receiverID) {
        throw ValidationException(
            "MessageManager: you cannot send a message to yourself");
    }

    
    User* sender = loadUser(senderID);
    delete sender;   
    User* receiver = loadUser(receiverID);
    delete receiver;

    EncodedMessage enc = coder_.encode(text);

   
    string ts = currentTimestamp();
    Message msg(senderID,
        receiverID,
        enc.bits,
        enc.bitCount,
        enc.serializedTree,
        enc.treeBitCount,
        ts);
    messageRepo_.saveMessage(msg);

    
    conversationCache_.remove(conversationKey(senderID, receiverID));

    return msg;
}



bool MessageManager::markAsRead(int currentUserID, int messageID) {
    
    Message m = messageRepo_.findMessageByID(messageID);

    requireSelfOrAdmin(currentUserID, m.getReceiverID());

    bool changed = messageRepo_.markAsRead(messageID);

    conversationCache_.remove(conversationKey(m.getSenderID(), m.getReceiverID()));

    return changed;
}


int MessageManager::markConversationRead(int currentUserID,
    int receiverID,
    int otherUserID) {
    
    requireSelfOrAdmin(currentUserID, receiverID);

    if (receiverID == otherUserID) {
        throw ValidationException(
            "MessageManager: receiver and other user must differ");
    }

    DataList<Message> conv = messageRepo_.findConversation(receiverID, otherUserID);

    int flipped = 0;
    for (int i = 0; i < conv.size(); ++i) {
        const Message& m = conv[i];
        
        if (m.getReceiverID() == receiverID && !m.getIsRead()) {
            if (messageRepo_.markAsRead(m.getMessageID())) {
                ++flipped;
            }
        }
    }

    if (flipped > 0) {
        conversationCache_.remove(conversationKey(receiverID, otherUserID));
    }

    return flipped;
}




DataList<DecodedMessage>
MessageManager::getConversation(int currentUserID,
    int viewerID,
    int otherUserID) {
    

    requireParticipantOrAdmin(currentUserID, viewerID, otherUserID);

    if (viewerID == otherUserID) {
        throw ValidationException(
            "MessageManager: viewer and other user must differ");
    }

    string key = conversationKey(viewerID, otherUserID);

    

    DataList<DecodedMessage>* hit = conversationCache_.get(key);
    if (hit != nullptr) {
       

        return *hit;
    }


    DataList<Message> raw = messageRepo_.findConversation(viewerID, otherUserID);
    DataList<DecodedMessage> decoded = decodeAll(raw);
    conversationCache_.put(key, decoded);
    return decoded;
}




DataList<DecodedMessage>
MessageManager::getInbox(int currentUserID, int receiverID) {
    requireSelfOrAdmin(currentUserID, receiverID);

    DataList<Message> raw = messageRepo_.findInbox(receiverID);
    return decodeAll(raw);
}




int MessageManager::countUnread(int currentUserID, int receiverID) {
    requireSelfOrAdmin(currentUserID, receiverID);
    return messageRepo_.countUnread(receiverID);
}


void MessageManager::clearCache() {
    conversationCache_.clear();
}