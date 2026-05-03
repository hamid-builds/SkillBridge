//
//#include "utils/HuffmanCoder.h"
//#include <iostream>
//#include <string>
//#include <vector>
//#include <cstdint>
//#include <stdexcept>
//using std::string;
//using std::cout;
//using std::vector;
//using std::uint8_t;
//
//static int passed = 0;
//static int failed = 0;
//
//#define CHECK(cond, name) \
//    do { \
//        if (cond) { ++passed; cout << "  [PASS] " << (name) << "\n"; } \
//        else { ++failed; cout << "  [FAIL] " << (name) << "\n"; } \
//    } while (0)
//
//static bool roundTrip(const string& input, string& decoded) {
//    HuffmanCoder coder;
//    EncodedMessage msg = coder.encode(input);
//    decoded = coder.decode(msg);
//    return decoded == input;
//}
//
//static void test_empty_string() {
//    cout << "test_empty_string:\n";
//    HuffmanCoder coder;
//    EncodedMessage msg = coder.encode("");
//    CHECK(msg.bits.empty(), "empty input -> empty payload");
//    CHECK(msg.bitCount == 0, "empty input -> bitCount 0");
//    CHECK(msg.serializedTree.empty(), "empty input -> empty tree");
//    CHECK(msg.treeBitCount == 0, "empty input -> treeBitCount 0");
//    string out = coder.decode(msg);
//    CHECK(out == "", "empty decode round-trips to empty");
//}
//
//static void test_single_character() {
//    cout << "test_single_character:\n";
//    string decoded;
//    bool ok = roundTrip("a", decoded);
//    CHECK(ok, "single character 'a' round-trips");
//    CHECK(decoded == "a", "decoded value matches");
//}
//
//static void test_single_unique_repeated() {
//    cout << "test_single_unique_repeated:\n";
//    string decoded;
//    bool ok = roundTrip("aaaaa", decoded);
//    CHECK(ok, "all-same string 'aaaaa' round-trips");
//    CHECK(decoded == "aaaaa", "decoded length matches");
//    string longRun(1000, 'z');
//    bool ok2 = roundTrip(longRun, decoded);
//    CHECK(ok2, "long run of one byte round-trips");
//    CHECK(decoded.size() == 1000, "decoded size matches");
//}
//
//static void test_two_characters() {
//    cout << "test_two_characters:\n";
//    string decoded;
//    bool ok = roundTrip("ab", decoded);
//    CHECK(ok, "ab round-trips");
//    bool ok2 = roundTrip("abababab", decoded);
//    CHECK(ok2, "abababab round-trips");
//}
//
//static void test_typical_chat_message() {
//    cout << "test_typical_chat_message:\n";
//    string decoded;
//    bool ok = roundTrip("hey, are you free for a quick call later today?", decoded);
//    CHECK(ok, "typical chat message round-trips");
//}
//
//static void test_long_message_with_punctuation() {
//    cout << "test_long_message_with_punctuation:\n";
//    string input =
//        "Salam Hamid, I had a chance to look at your portfolio last night. "
//        "Your work on the SkillBridge demo was strong; the search ranking "
//        "felt snappy and the CLI was clean. Could we set up a 30-minute "
//        "call this week to walk through the order flow? I want to see "
//        "how undo/redo handles the corner cases. Best regards.";
//    string decoded;
//    bool ok = roundTrip(input, decoded);
//    CHECK(ok, "long English paragraph round-trips");
//    CHECK(decoded.size() == input.size(), "decoded size matches input");
//}
//
//static void test_all_256_bytes() {
//    cout << "test_all_256_bytes:\n";
//    string input;
//    input.reserve(256);
//    for (int i = 0; i < 256; ++i) {
//        input.push_back(static_cast<char>(i));
//    }
//    string decoded;
//    bool ok = roundTrip(input, decoded);
//    CHECK(ok, "all 256 byte values round-trip");
//    CHECK(decoded.size() == 256, "decoded size is 256");
//    bool allMatch = true;
//    for (int i = 0; i < 256; ++i) {
//        if (static_cast<uint8_t>(decoded[i]) != static_cast<uint8_t>(i)) {
//            allMatch = false;
//            break;
//        }
//    }
//    CHECK(allMatch, "every byte value preserved");
//}
//
//static void test_random_repeated_patterns() {
//    cout << "test_random_repeated_patterns:\n";
//    string decoded;
//    bool ok1 = roundTrip("the quick brown fox jumps over the lazy dog", decoded);
//    CHECK(ok1, "pangram round-trips");
//    bool ok2 = roundTrip("abc def\nghi\tjkl  mno", decoded);
//    CHECK(ok2, "whitespace + tabs + newlines round-trip");
//    bool ok3 = roundTrip("12345!@#$%^&*()_+-=", decoded);
//    CHECK(ok3, "digits and symbols round-trip");
//}
//
//static void test_compression_visible_for_skewed_input() {
//    cout << "test_compression_visible_for_skewed_input:\n";
//    HuffmanCoder coder;
//    string input;
//    for (int i = 0; i < 700; ++i) input.push_back('a');
//    for (int i = 0; i < 200; ++i) input.push_back('b');
//    for (int i = 0; i < 70; ++i) input.push_back('c');
//    for (int i = 0; i < 30; ++i) input.push_back('d');
//    EncodedMessage msg = coder.encode(input);
//    size_t uncompressedBits = input.size() * 8;
//    CHECK(msg.bitCount < uncompressedBits, "payload bits < uncompressed bits for skewed input");
//    string decoded = coder.decode(msg);
//    CHECK(decoded == input, "skewed input round-trips");
//}
//
//static void test_corrupted_payload_throws() {
//    cout << "test_corrupted_payload_throws:\n";
//    HuffmanCoder coder;
//    EncodedMessage msg = coder.encode("hello world");
//    EncodedMessage bad = msg;
//    bad.bitCount = bad.bits.size() * 8 + 100;
//    bool threw = false;
//    try {
//        (void)coder.decode(bad);
//    }
//    catch (const std::runtime_error&) {
//        threw = true;
//    }
//    CHECK(threw, "decode throws on bitCount exceeding buffer");
//}
//
//static void test_corrupted_tree_throws() {
//    cout << "test_corrupted_tree_throws:\n";
//    HuffmanCoder coder;
//    EncodedMessage msg = coder.encode("hello");
//    EncodedMessage bad = msg;
//    if (bad.treeBitCount > 4) {
//        bad.treeBitCount = 4;
//    }
//    bool threw = false;
//    try {
//        (void)coder.decode(bad);
//    }
//    catch (const std::runtime_error&) {
//        threw = true;
//    }
//    CHECK(threw, "decode throws on truncated tree");
//}
//static void test_independent_messages() {
//    cout << "test_independent_messages:\n";
//    HuffmanCoder coder;
//    EncodedMessage m1 = coder.encode("first message");
//    EncodedMessage m2 = coder.encode("second message, different content");
//    string d1 = coder.decode(m1);
//    string d2 = coder.decode(m2);
//    CHECK(d1 == "first message", "first message decodes correctly");
//    CHECK(d2 == "second message, different content",
//        "second message decodes correctly");
//}
//
//static void test_deterministic_encoding() {
//    cout << "test_deterministic_encoding:\n";
//    HuffmanCoder coder;
//    string input = "deterministic encoding sanity check";
//    EncodedMessage a = coder.encode(input);
//    EncodedMessage b = coder.encode(input);
//    CHECK(a.bitCount == b.bitCount, "bit counts match across runs");
//    CHECK(a.bits == b.bits, "payload bytes match across runs");
//    CHECK(a.treeBitCount == b.treeBitCount, "tree bit counts match");
//    CHECK(a.serializedTree == b.serializedTree, "tree bytes match across runs");
//}
//
//int main() {
//    cout << "Running HuffmanCoder tests...\n\n";
//    test_empty_string();
//    test_single_character();
//    test_single_unique_repeated();
//    test_two_characters();
//    test_typical_chat_message();
//    test_long_message_with_punctuation();
//    test_all_256_bytes();
//    test_random_repeated_patterns();
//    test_compression_visible_for_skewed_input();
//    test_corrupted_payload_throws();
//    test_corrupted_tree_throws();
//    test_independent_messages();
//    test_deterministic_encoding();
//    cout << "\n==============================\n";
//    cout << "Passed: " << passed << "\n";
//    cout << "Failed: " << failed << "\n";
//    cout << "==============================\n";
//    return failed == 0 ? 0 : 1;
//}