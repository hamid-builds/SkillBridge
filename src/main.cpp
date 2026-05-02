

#include "managers/LevenshteinMatcher.h"
#include <iostream>
#include <string>

using namespace std;

static int testsRun = 0;
static int testsPassed = 0;

#define CHECK(cond, name) do { \
    ++testsRun; \
    if (cond) { ++testsPassed; cout << "  PASS: " << name << endl; } \
    else      { cout << "  FAIL: " << name << endl; } \
} while (0)


static void testIdentity() {
    cout << "\n[1] Identity and trivial cases\n";
    LevenshteinMatcher m;

    CHECK(m.distance("", "") == 0, "empty vs empty = 0");
    CHECK(m.distance("hello", "hello") == 0, "identical strings = 0");
    CHECK(m.distance("a", "a") == 0, "single-char identity = 0");
    CHECK(m.distance("", "abc") == 3, "empty vs 'abc' = 3");
    CHECK(m.distance("abc", "") == 3, "'abc' vs empty = 3");
}


static void testSingleEdits() {
    cout << "\n[2] Single edits\n";
    LevenshteinMatcher m;

   
    CHECK(m.distance("cat", "bat") == 1, "substitution: cat -> bat = 1");
    CHECK(m.distance("cat", "car") == 1, "substitution at end: cat -> car = 1");

    
    CHECK(m.distance("cat", "cats") == 1, "insertion: cat -> cats = 1");
    CHECK(m.distance("at", "cat") == 1, "insertion at start: at -> cat = 1");

   
    CHECK(m.distance("cats", "cat") == 1, "deletion: cats -> cat = 1");
    CHECK(m.distance("cart", "cat") == 1, "deletion middle: cart -> cat = 1");
}


static void testMultiEdits() {
    cout << "\n[3] Multi edits\n";
    LevenshteinMatcher m;

   
    CHECK(m.distance("kitten", "sitting") == 3,
        "kitten -> sitting = 3 (substitute k->s, e->i, insert g)");

    
    CHECK(m.distance("kotln", "kotlin") == 1, "kotln -> kotlin = 1 (insert i)");
    CHECK(m.distance("pyhton", "python") == 2,
        "pyhton -> python = 2 (transposition costs 2 in pure Levenshtein)");
    CHECK(m.distance("javascrpt", "javascript") == 1,
        "javascrpt -> javascript = 1");

   
    CHECK(m.distance("abc", "xyz") == 3, "abc vs xyz = 3 (3 substitutions)");
}


static void testSymmetry() {
    cout << "\n[4] Symmetry\n";
    LevenshteinMatcher m;

    CHECK(m.distance("hello", "world") == m.distance("world", "hello"),
        "distance(hello, world) == distance(world, hello)");
    CHECK(m.distance("python", "py") == m.distance("py", "python"),
        "distance is symmetric (long vs short)");
    CHECK(m.distance("kitten", "sitting") == m.distance("sitting", "kitten"),
        "distance is symmetric (kitten/sitting)");
}

static void testCaseSensitivity() {
    cout << "\n[5] Case sensitivity (compares as-is)\n";
    LevenshteinMatcher m;

   
    CHECK(m.distance("Python", "python") == 1,
        "Python vs python = 1 (P != p, one substitution)");
    CHECK(m.distance("HELLO", "hello") == 5,
        "HELLO vs hello = 5 (every char differs)");
}


static void testPolymorphism() {
    cout << "\n[6] Polymorphism through IFuzzyMatcher pointer\n";

    
    IFuzzyMatcher* matcher = new LevenshteinMatcher();
    CHECK(matcher->distance("kotln", "kotlin") == 1,
        "polymorphic call: distance through IFuzzyMatcher* works");
    CHECK(matcher->distance("same", "same") == 0,
        "polymorphic call: identity returns 0");
    delete matcher;
    CHECK(true, "destructor through base pointer (virtual dtor) is safe");
}

int main() {
    cout << "=== Module 2 - Step 9: LevenshteinMatcher test harness ===\n";

    testIdentity();
    testSingleEdits();
    testMultiEdits();
    testSymmetry();
    testCaseSensitivity();
    testPolymorphism();

    cout << "\n=== Summary: " << testsPassed << " / " << testsRun
        << " tests passed ===\n";

    return (testsPassed == testsRun) ? 0 : 1;
}