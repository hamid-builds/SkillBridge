#include "core/Message.h"
#include "core/Exceptions.h"
#include <ostream>
using namespace std;
static void validateMessageFields(int senderID,
    int receiverID,
    const vector<uint8_t>& payloadBlob,
    size_t payloadBits,
    const vector<uint8_t>& treeBlob,
    size_t treeBits,
    const string& timestamp) {
    if (senderID <= 0)
        throw ValidationException("Message: senderID must be positive");
    if (receiverID <= 0)
        throw ValidationException("Message: receiverID must be positive");
    if (senderID == receiverID)
        throw ValidationException("Message: sender and receiver must differ");
    if (timestamp.empty())
        throw ValidationException("Message: timestamp cannot be empty");
    if (payloadBits > payloadBlob.size() * 8u) {
        throw ValidationException("Message: payloadBits exceeds payload buffer");
    }
    if (treeBits > treeBlob.size() * 8u) {
        throw ValidationException("Message: treeBits exceeds tree buffer");
    }
    if ((payloadBits == 0) != (treeBits == 0)) {
        throw ValidationException("Message: payload and tree must both be empty or both non-empty");
    }
}
Message::Message() : messageID_(0), senderID_(0), receiverID_(0), payloadBlob_(), payloadBits_(0), treeBlob_(), treeBits_(0), timestamp_(""), isRead_(false) {}
Message::Message(int messageID, int senderID, int receiverID, const vector<uint8_t>& payloadBlob, size_t payloadBits, const vector<uint8_t>& treeBlob, size_t treeBits, const string& timestamp, bool isRead)
    : messageID_(messageID), senderID_(senderID), receiverID_(receiverID), payloadBlob_(payloadBlob), payloadBits_(payloadBits), treeBlob_(treeBlob), treeBits_(treeBits), timestamp_(timestamp), isRead_(isRead){
    if (messageID < 0) {
        throw ValidationException("Message: messageID cannot be negative");
    }
    validateMessageFields(senderID, receiverID,
        payloadBlob, payloadBits,
        treeBlob, treeBits,
        timestamp);
}

Message::Message(int senderID, int receiverID, const vector<uint8_t>& payloadBlob, size_t payloadBits, const vector<uint8_t>& treeBlob, size_t treeBits, const string& timestamp)
    : messageID_(0), senderID_(senderID), receiverID_(receiverID), payloadBlob_(payloadBlob), payloadBits_(payloadBits), treeBlob_(treeBlob), treeBits_(treeBits), timestamp_(timestamp), isRead_(false) {
    validateMessageFields(senderID, receiverID, payloadBlob, payloadBits, treeBlob, treeBits, timestamp);
}

void Message::setMessageID(int id) {
    if (id <= 0) {
        throw ValidationException("Message::setMessageID: id must be positive");
    }
    if (messageID_ != 0 && messageID_ != id) {
        throw ValidationException("Message::setMessageID: messageID is already assigned");
    }
    messageID_ = id;
}

void Message::setSenderID(int senderID) {
    if (senderID <= 0) {
        throw ValidationException("Message: senderID must be positive");
    }
    senderID_ = senderID;
}

void Message::setReceiverID(int receiverID) {
    if (receiverID <= 0) {
        throw ValidationException("Message: receiverID must be positive");
    }
    receiverID_ = receiverID;
}

void Message::setTimestamp(const string& ts) {
    if (ts.empty()) {
        throw ValidationException("Message: timestamp cannot be empty");
    }
    timestamp_ = ts;
}

ostream& operator<<(ostream& os, const Message& m) {
    os << "Message #" << m.messageID_
        << " [from=" << m.senderID_
        << ", to=" << m.receiverID_
        << ", at=" << m.timestamp_
        << ", " << (m.isRead_ ? "read" : "unread")
        << ", payload=" << m.payloadBlob_.size() << "B"
        << " (" << m.payloadBits_ << " bits)"
        << ", tree=" << m.treeBlob_.size() << "B"
        << " (" << m.treeBits_ << " bits)"
        << "]";
    return os;
}