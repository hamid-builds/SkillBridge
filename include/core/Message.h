#pragma once
#ifndef SKILLBRIDGE_MESSAGE_H
#define SKILLBRIDGE_MESSAGE_H

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <iosfwd>

class Message {
private:
    int messageID_;
    int senderID_;
    int receiverID_;
    std::vector<uint8_t> payloadBlob_;
    std::size_t payloadBits_;
    std::vector<uint8_t> treeBlob_;
    std::size_t treeBits_;
    std::string timestamp_;
    bool isRead_;
public:
    Message();
    Message(int messageID,
        int senderID,
        int receiverID,
        const std::vector<uint8_t>& payloadBlob,
        std::size_t payloadBits,
        const std::vector<uint8_t>& treeBlob,
        std::size_t treeBits,
        const std::string& timestamp,
        bool isRead);
    Message(int senderID, int receiverID, const std::vector<uint8_t>& payloadBlob, std::size_t payloadBits, const std::vector<uint8_t>& treeBlob, std::size_t treeBits, const std::string& timestamp);

    int getMessageID() const {
        return messageID_;
    }
    int getSenderID()    const {
        return senderID_;
    }
    int getReceiverID()  const {
        return receiverID_;
    }
    const std::vector<uint8_t>& getPayloadBlob() const {
        return payloadBlob_;
    }
    std::size_t getPayloadBits() const {
        return payloadBits_;
    }
    const std::vector<uint8_t>& getTreeBlob() const {
        return treeBlob_;
    }
    std::size_t getTreeBits() const {
        return treeBits_;
    }
    const std::string& getTimestamp() const {
        return timestamp_;
    }
    bool getIsRead() const {
        return isRead_;
    }
    void setMessageID(int id);
    void setSenderID(int senderID);
    void setReceiverID(int receiverID);
    void setPayloadBlob(const std::vector<uint8_t>& bytes) {
        payloadBlob_ = bytes;
    }
    void setPayloadBits(std::size_t bits) {
        payloadBits_ = bits;
    }
    void setTreeBlob(const std::vector<uint8_t>& bytes) {
        treeBlob_ = bytes;
    }
    void setTreeBits(std::size_t bits) {
        treeBits_ = bits;
    }
    void setTimestamp(const std::string& ts);
    void setIsRead(bool r) { isRead_ = r; }
    bool operator==(const Message& other) const {
        return messageID_ == other.messageID_;
    }
    bool operator!=(const Message& other) const {
        return !(*this == other);
    }
    bool operator<(const Message& other)  const {
        return timestamp_ < other.timestamp_;
    }
    friend std::ostream& operator<<(std::ostream& os, const Message& m);
};

#endif