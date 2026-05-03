#include "core/Message.h"
#include "core/Exceptions.h"
#include "managers/IMessageRepository.h"
#include "utils/DataList.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>

using std::string;
using std::vector;
using std::cout;
using std::uint8_t;
using std::size_t;

static int passed = 0;
static int failed = 0;

#define CHECK(cond, name) \
    do { \
        if (cond) { ++passed; cout << "  [PASS] " << (name) << "\n"; } \
        else { ++failed; cout << "  [FAIL] " << (name) << "\n"; } \
    } while (0)

class FakeMessageRepository : public IMessageRepository {
private:
    DataList<Message> rows_;
    int nextID_;

public:
    FakeMessageRepository() : nextID_(1) {}
    void saveMessage(Message& msg) override {
        msg.setMessageID(nextID_++);
        rows_.add(msg);
    }
    bool markAsRead(int messageID) override {
        for (int i = 0; i < rows_.size(); ++i) {
            if (rows_[i].getMessageID() == messageID) {
                rows_[i].setIsRead(true);
                return true;
            }
        }
        return false;
    }
    bool deleteMessage(int messageID) override {
        for (int i = 0; i < rows_.size(); ++i) {
            if (rows_[i].getMessageID() == messageID) {
                rows_.removeAt(i);
                return true;
            }
        }
        return false;
    }
    Message findMessageByID(int messageID) const override {
        for (int i = 0; i < rows_.size(); ++i) {
            if (rows_[i].getMessageID() == messageID)
                return rows_[i];
        }
        throw MessageNotFoundException(
            "FakeMessageRepository: no message with ID");
    }
    DataList<Message> findConversation(int userA, int userB) const override {
        DataList<Message> out;
        for (int i = 0; i < rows_.size(); ++i) {
            const Message& m = rows_[i];
            bool ab = m.getSenderID() == userA && m.getReceiverID() == userB;
            bool ba = m.getSenderID() == userB && m.getReceiverID() == userA;
            if (ab || ba) out.add(m);
        }
        for (int i = 1; i < out.size(); ++i) {
            Message key = out[i];
            int j = i - 1;
            while (j >= 0) {
                bool keyEarlier = (key.getTimestamp() < out[j].getTimestamp()) || (key.getTimestamp() == out[j].getTimestamp() && key.getMessageID() < out[j].getMessageID());
                if (!keyEarlier) break;
                out[j + 1] = out[j];
                --j;
            }
            out[j + 1] = key;
        }
        return out;
    }

    DataList<Message> findInbox(int receiverID) const override {
        DataList<Message> out;
        for (int i = 0; i < rows_.size(); ++i) {
            if (rows_[i].getReceiverID() == receiverID) out.add(rows_[i]);
        }
        for (int i = 1; i < out.size(); ++i) {
            Message key = out[i];
            int j = i - 1;
            while (j >= 0) {
                bool keyNewer = (key.getTimestamp() > out[j].getTimestamp())
                    || (key.getTimestamp() == out[j].getTimestamp()
                        && key.getMessageID() > out[j].getMessageID());
                if (!keyNewer) break;
                out[j + 1] = out[j];
                --j;
            }
            out[j + 1] = key;
        }
        return out;
    }
    DataList<Message> findAllMessages() const override {
        DataList<Message> out;
        for (int i = 0; i < rows_.size(); ++i) out.add(rows_[i]);
        return out;
    }
    int countUnread(int receiverID) const override {
        int n = 0;
        for (int i = 0; i < rows_.size(); ++i) {
            if (rows_[i].getReceiverID() == receiverID && !rows_[i].getIsRead()) {
                ++n;
            }
        }
        return n;
    }
};

static vector<uint8_t> bytes(std::initializer_list<uint8_t> il) {
    return vector<uint8_t>(il);
}

static Message makeNewMessage(int from, int to, const string& ts) {
    return Message(from, to,
        bytes({ 0xAB, 0xCD }), 13,
        bytes({ 0x12, 0x34 }), 11,
        ts);
}

static void test_default_ctor_unset() {
    cout << "test_default_ctor_unset:\n";
    Message m;
    CHECK(m.getMessageID() == 0, "default messageID is 0");
    CHECK(m.getSenderID() == 0, "default senderID is 0");
    CHECK(m.getReceiverID() == 0, "default receiverID is 0");
    CHECK(m.getPayloadBits() == 0, "default payloadBits is 0");
    CHECK(m.getTreeBits() == 0, "default treeBits is 0");
    CHECK(m.getPayloadBlob().empty(), "default payloadBlob empty");
    CHECK(m.getTreeBlob().empty(), "default treeBlob empty");
    CHECK(m.getTimestamp() == "", "default timestamp empty");
    CHECK(m.getIsRead() == false, "default isRead false");
}

static void test_new_message_ctor_valid() {
    cout << "test_new_message_ctor_valid:\n";
    Message m = makeNewMessage(1, 2, "2026-04-26 10:00:00");
    CHECK(m.getMessageID() == 0, "new message has unset ID");
    CHECK(m.getSenderID() == 1, "senderID set");
    CHECK(m.getReceiverID() == 2, "receiverID set");
    CHECK(m.getPayloadBits() == 13, "payloadBits set");
    CHECK(m.getTreeBits() == 11, "treeBits set");
    CHECK(m.getIsRead() == false, "isRead defaults false");
}

static void test_new_message_rejects_self() {
    cout << "test_new_message_rejects_self:\n";
    bool threw = false;
    try {
        (void)makeNewMessage(7, 7, "2026-04-26 10:00:00");
    }
    catch (const ValidationException&) {
        threw = true;
    }
    CHECK(threw, "ctor throws when sender == receiver");
}

static void test_new_message_rejects_zero_ids() {
    cout << "test_new_message_rejects_zero_ids:\n";
    bool t1 = false, t2 = false;
    try { (void)makeNewMessage(0, 5, "2026-04-26"); }
    catch (const ValidationException&) { t1 = true; }
    try { (void)makeNewMessage(5, 0, "2026-04-26"); }
    catch (const ValidationException&) { t2 = true; }
    CHECK(t1, "ctor throws when senderID is 0");
    CHECK(t2, "ctor throws when receiverID is 0");
}

static void test_new_message_rejects_empty_timestamp() {
    cout << "test_new_message_rejects_empty_timestamp:\n";
    bool threw = false;
    try { (void)makeNewMessage(1, 2, ""); }
    catch (const ValidationException&) { threw = true; }
    CHECK(threw, "ctor throws on empty timestamp");
}

static void test_payload_bits_must_fit() {
    cout << "test_payload_bits_must_fit:\n";
    bool threw = false;
    try {
        Message bad(1, 2, bytes({ 0xFF, 0x00 }), 17,
            bytes({ 0x00 }), 5,
            "2026-04-26");
        (void)bad;
    }
    catch (const ValidationException&) {
        threw = true;
    }
    CHECK(threw, "ctor throws when payloadBits exceeds buffer");
}

static void test_tree_bits_must_fit() {
    cout << "test_tree_bits_must_fit:\n";
    bool threw = false;
    try {
        Message bad(1, 2, bytes({ 0xFF }), 5,
            bytes({ 0x00 }), 9,
            "2026-04-26");
        (void)bad;
    }
    catch (const ValidationException&) {
        threw = true;
    }
    CHECK(threw, "ctor throws when treeBits exceeds buffer");
}

static void test_consistency_blob_and_bits() {
    cout << "test_consistency_blob_and_bits:\n";
    bool threw1 = false;
    try {
        Message bad(1, 2, bytes({ 0xFF }), 5,
            vector<uint8_t>(), 0,
            "2026-04-26");
        (void)bad;
    }
    catch (const ValidationException&) { threw1 = true; }
    CHECK(threw1, "ctor rejects payload-without-tree");

    bool threw2 = false;
    try {
        Message bad(1, 2, vector<uint8_t>(), 0,
            bytes({ 0xFF }), 5,
            "2026-04-26");
        (void)bad;
    }
    catch (const ValidationException&) { threw2 = true; }
    CHECK(threw2, "ctor rejects tree-without-payload");

    bool ok = true;
    try {
        Message empty(1, 2, vector<uint8_t>(), 0,
            vector<uint8_t>(), 0,
            "2026-04-26");
        (void)empty;
    }
    catch (...) { ok = false; }
    CHECK(ok, "ctor accepts both-empty (degenerate but consistent)");
}

static void test_setMessageID_once() {
    cout << "test_setMessageID_once:\n";
    Message m = makeNewMessage(1, 2, "2026-04-26");
    m.setMessageID(42);
    CHECK(m.getMessageID() == 42, "first setMessageID succeeds");

    bool ok = true;
    try { m.setMessageID(42); }
    catch (...) { ok = false; }
    CHECK(ok, "setMessageID with same value is idempotent");

    bool threw = false;
    try { m.setMessageID(99); }
    catch (const ValidationException&) { threw = true; }
    CHECK(threw, "setMessageID rejects conflicting reassign");

    bool t1 = false, t2 = false;
    try { Message m2 = makeNewMessage(1, 2, "x"); m2.setMessageID(0); }
    catch (const ValidationException&) { t1 = true; }
    try { Message m3 = makeNewMessage(1, 2, "x"); m3.setMessageID(-5); }
    catch (const ValidationException&) { t2 = true; }
    CHECK(t1, "setMessageID rejects 0");
    CHECK(t2, "setMessageID rejects negative");
}

static void test_equality_by_id() {
    cout << "test_equality_by_id:\n";
    Message a = makeNewMessage(1, 2, "2026-04-26");
    Message b = makeNewMessage(99, 100, "2099-01-01");
    a.setMessageID(7);
    b.setMessageID(7);
    CHECK(a == b, "two messages with same ID are equal regardless of content");
    Message c = makeNewMessage(1, 2, "2026-04-26");
    c.setMessageID(8);
    CHECK(a != c, "different IDs are unequal");
}

static void test_ordering_by_timestamp() {
    cout << "test_ordering_by_timestamp:\n";
    Message older = makeNewMessage(1, 2, "2026-04-26 10:00:00");
    Message newer = makeNewMessage(1, 2, "2026-04-26 10:00:01");
    CHECK(older < newer, "older timestamp is less than newer");
    CHECK(!(newer < older), "newer is not less than older");
}

static void test_stream_output_no_blob_bytes() {
    cout << "test_stream_output_no_blob_bytes:\n";
    Message m = makeNewMessage(3, 5, "2026-04-26 12:00:00");
    m.setMessageID(101);
    std::ostringstream os;
    os << m;
    string s = os.str();
    bool hasID = s.find("#101") != string::npos;
    bool hasFrom = s.find("from=3") != string::npos;
    bool hasTo = s.find("to=5") != string::npos;
    bool hasUnread = s.find("unread") != string::npos;
    bool hasBitsLabel = s.find("bits") != string::npos;
    CHECK(hasID, "stream contains messageID");
    CHECK(hasFrom, "stream contains sender");
    CHECK(hasTo, "stream contains receiver");
    CHECK(hasUnread, "stream shows unread state");
    CHECK(hasBitsLabel, "stream shows bit-count metadata");
}

static void test_repo_save_assigns_id() {
    cout << "test_repo_save_assigns_id:\n";
    FakeMessageRepository repo;
    Message m = makeNewMessage(1, 2, "2026-04-26 10:00:00");
    repo.saveMessage(m);
    CHECK(m.getMessageID() > 0, "save assigns positive messageID");
    CHECK(m.getMessageID() == 1, "first save gets ID 1");
    Message m2 = makeNewMessage(1, 2, "2026-04-26 10:00:01");
    repo.saveMessage(m2);
    CHECK(m2.getMessageID() == 2, "second save gets ID 2");
}

static void test_repo_find_by_id() {
    cout << "test_repo_find_by_id:\n";
    FakeMessageRepository repo;
    Message m = makeNewMessage(1, 2, "2026-04-26");
    repo.saveMessage(m);
    Message found = repo.findMessageByID(m.getMessageID());
    CHECK(found.getMessageID() == m.getMessageID(), "found matches saved");
    CHECK(found.getSenderID() == 1, "sender preserved");
    CHECK(found.getReceiverID() == 2, "receiver preserved");
}

static void test_repo_find_by_id_missing_throws() {
    cout << "test_repo_find_by_id_missing_throws:\n";
    FakeMessageRepository repo;
    bool threw = false;
    try { (void)repo.findMessageByID(999); }
    catch (const MessageNotFoundException&) { threw = true; }
    CHECK(threw, "missing ID throws MessageNotFoundException");
}

static void test_repo_conversation_symmetric() {
    cout << "test_repo_conversation_symmetric:\n";
    FakeMessageRepository repo;
    Message a = makeNewMessage(1, 2, "2026-04-26 10:00:00");
    Message b = makeNewMessage(2, 1, "2026-04-26 10:00:01");
    Message c = makeNewMessage(1, 2, "2026-04-26 10:00:02");
    repo.saveMessage(a);
    repo.saveMessage(b);
    repo.saveMessage(c);
    Message d = makeNewMessage(3, 4, "2026-04-26 10:00:00");
    repo.saveMessage(d);
    DataList<Message> conv12 = repo.findConversation(1, 2);
    DataList<Message> conv21 = repo.findConversation(2, 1);
    CHECK(conv12.size() == 3, "conversation has 3 messages");
    CHECK(conv21.size() == 3, "calling with reversed order has same size");
    bool ordered = conv12[0].getTimestamp() <= conv12[1].getTimestamp() && conv12[1].getTimestamp() <= conv12[2].getTimestamp();
    CHECK(ordered, "conversation is chronologically ordered (oldest first)");
    bool none34 = true;
    for (int i = 0; i < conv12.size(); ++i) {
        int s = conv12[i].getSenderID();
        int r = conv12[i].getReceiverID();
        if (s == 3 || s == 4 || r == 3 || r == 4) {
            none34 = false;
            break;
        }
    }
    CHECK(none34, "conversation excludes unrelated users");
}
static void test_repo_conversation_empty() {
    cout << "test_repo_conversation_empty:\n";
    FakeMessageRepository repo;
    DataList<Message> conv = repo.findConversation(42, 99);
    CHECK(conv.size() == 0, "no messages between strangers returns empty");
}

static void test_repo_inbox_newest_first() {
    cout << "test_repo_inbox_newest_first:\n";
    FakeMessageRepository repo;
    Message a = makeNewMessage(1, 5, "2026-04-26 10:00:00");
    Message b = makeNewMessage(2, 5, "2026-04-26 10:00:02");
    Message c = makeNewMessage(3, 5, "2026-04-26 10:00:01");
    repo.saveMessage(a);
    repo.saveMessage(b);
    repo.saveMessage(c);
    Message d = makeNewMessage(5, 9, "2026-04-26 10:00:00");
    repo.saveMessage(d);
    DataList<Message> inbox = repo.findInbox(5);
    CHECK(inbox.size() == 3, "inbox has 3 received messages");
    bool newestFirst =
        inbox[0].getTimestamp() >= inbox[1].getTimestamp() &&
        inbox[1].getTimestamp() >= inbox[2].getTimestamp();
    CHECK(newestFirst, "inbox is newest-first");
}

static void test_repo_mark_as_read() {
    cout << "test_repo_mark_as_read:\n";
    FakeMessageRepository repo;
    Message m = makeNewMessage(1, 2, "2026-04-26 10:00:00");
    repo.saveMessage(m);
    int id = m.getMessageID();
    CHECK(repo.markAsRead(id) == true, "markAsRead returns true on hit");
    Message after = repo.findMessageByID(id);
    CHECK(after.getIsRead() == true, "isRead flipped to true");
    CHECK(repo.markAsRead(99999) == false, "markAsRead returns false on miss");
}

static void test_repo_count_unread() {
    cout << "test_repo_count_unread:\n";
    FakeMessageRepository repo;
    Message a = makeNewMessage(1, 5, "t1");
    Message b = makeNewMessage(2, 5, "t2");
    Message c = makeNewMessage(3, 5, "t3");
    repo.saveMessage(a);
    repo.saveMessage(b);
    repo.saveMessage(c);
    CHECK(repo.countUnread(5) == 3, "all 3 received messages start unread");
    repo.markAsRead(b.getMessageID());
    CHECK(repo.countUnread(5) == 2, "after marking one, count is 2");
    CHECK(repo.countUnread(999) == 0, "user with no inbox has 0 unread");
}

static void test_repo_delete_message() {
    cout << "test_repo_delete_message:\n";
    FakeMessageRepository repo;
    Message m = makeNewMessage(1, 2, "2026-04-26");
    repo.saveMessage(m);
    int id = m.getMessageID();
    CHECK(repo.deleteMessage(id) == true, "delete returns true on hit");
    bool gone = false;
    try { (void)repo.findMessageByID(id); }
    catch (const MessageNotFoundException&) { gone = true; }
    CHECK(gone, "deleted message is no longer findable");
    CHECK(repo.deleteMessage(id) == false, "second delete returns false");
    CHECK(repo.deleteMessage(99999) == false, "delete of unknown ID returns false");
}

static void test_repo_find_all() {
    cout << "test_repo_find_all:\n";
    FakeMessageRepository repo;
    Message m1 = makeNewMessage(1, 2, "t1");
    Message m2 = makeNewMessage(2, 3, "t2");
    Message m3 = makeNewMessage(3, 4, "t3");
    repo.saveMessage(m1);
    repo.saveMessage(m2);
    repo.saveMessage(m3);
    DataList<Message> all = repo.findAllMessages();
    CHECK(all.size() == 3, "findAllMessages returns every message");
}

int main() {
    cout << "Running Message + IMessageRepository tests...\n\n";
    test_default_ctor_unset();
    test_new_message_ctor_valid();
    test_new_message_rejects_self();
    test_new_message_rejects_zero_ids();
    test_new_message_rejects_empty_timestamp();
    test_payload_bits_must_fit();
    test_tree_bits_must_fit();
    test_consistency_blob_and_bits();
    test_setMessageID_once();
    test_equality_by_id();
    test_ordering_by_timestamp();
    test_stream_output_no_blob_bytes();
    test_repo_save_assigns_id();
    test_repo_find_by_id();
    test_repo_find_by_id_missing_throws();
    test_repo_conversation_symmetric();
    test_repo_conversation_empty();
    test_repo_inbox_newest_first();
    test_repo_mark_as_read();
    test_repo_count_unread();
    test_repo_delete_message();
    test_repo_find_all();
    cout << "\n==============================\n";
    cout << "Passed: " << passed << "\n";
    cout << "Failed: " << failed << "\n";
    cout << "==============================\n";
    return failed == 0 ? 0 : 1;
}