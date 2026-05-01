#include <iostream>
#include <string>
#include <sstream>
#include <exception>

#include "core/Gig.h"
#include "core/GigCategory.h"
#include "core/Exceptions.h"

using namespace std;

static int g_pass = 0;
static int g_fail = 0;

static void check(bool condition, const string& label) {
    if (condition) {
        cout << "  PASS  " << label << "\n";
        g_pass++;
    }
    else {
        cout << "  FAIL  " << label << "\n";
        g_fail++;
    }
}


template <typename ExType, typename Fn>
static void expectThrow(Fn fn, const string& label) {
    try {
        fn();
        cout << "  FAIL  " << label << " (no exception thrown)\n";
        g_fail++;
    }
    catch (const ExType&) {
        cout << "  PASS  " << label << "\n";
        g_pass++;
    }
    catch (const exception& e) {
        cout << "  FAIL  " << label
            << " (wrong type: " << e.what() << ")\n";
        g_fail++;
    }
}



static void testExceptionHierarchy() {
    cout << "\n[Exception hierarchy: polymorphic catch + prefixes]\n";

    // Every exception type catches as SkillBridgeException
    try {
        throw ValidationException("bad input");
    }
    catch (const SkillBridgeException& e) {
        string msg = e.what();
        check(msg == "Validation error: bad input",
            "ValidationException: prefix and message");
    }

    try {
        throw UnauthorizedException("not your gig");
    }
    catch (const SkillBridgeException& e) {
        string msg = e.what();
        check(msg == "Unauthorized: not your gig",
            "UnauthorizedException: prefix and message");
    }

    try {
        throw GigNotFoundException("gigID=42");
    }
    catch (const SkillBridgeException& e) {
        string msg = e.what();
        check(msg == "Gig not found: gigID=42",
            "GigNotFoundException: prefix and message");
    }

    try {
        throw AuthenticationException("wrong password");
    }
    catch (const SkillBridgeException& e) {
        string msg = e.what();
        check(msg == "Authentication error: wrong password",
            "AuthenticationException: prefix preserved");
    }

    try {
        throw DatabaseException("disk full");
    }
    catch (const SkillBridgeException& e) {
        string msg = e.what();
        check(msg == "Database error: disk full",
            "DatabaseException: prefix preserved");
    }

    try {
        throw DuplicateEntryException("email already exists");
    }
    catch (const SkillBridgeException& e) {
        string msg = e.what();
        check(msg == "Duplicate entry: email already exists",
            "DuplicateEntryException: prefix preserved");
    }

    try {
        throw RateLimitException("too many attempts");
    }
    catch (const SkillBridgeException& e) {
        string msg = e.what();
        check(msg == "Rate limit exceeded: too many attempts",
            "RateLimitException: prefix preserved");
    }
}

static void testStdExceptionRoot() {
    cout << "\n[Exception hierarchy: all catch as std::exception]\n";

    try {
        throw ValidationException("x");
    }
    catch (const exception& e) {
        check(string(e.what()).find("Validation error") != string::npos,
            "ValidationException caught via std::exception");
    }

    try {
        throw UnauthorizedException("y");
    }
    catch (const exception& e) {
        check(string(e.what()).find("Unauthorized") != string::npos,
            "UnauthorizedException caught via std::exception");
    }

    try {
        throw GigNotFoundException("z");
    }
    catch (const exception& e) {
        check(string(e.what()).find("Gig not found") != string::npos,
            "GigNotFoundException caught via std::exception");
    }
}

static void testGigThrowsValidationException() {
    cout << "\n[Gig validation now throws ValidationException]\n";

    expectThrow<ValidationException>(
        []() {
            Gig g(1, "Lo", "A valid description here.",
                100.0, GigCategory::OTHER);
        },
        "short title -> ValidationException");

    expectThrow<ValidationException>(
        []() {
            Gig g(1, "Valid Title", "short",
                100.0, GigCategory::OTHER);
        },
        "short description -> ValidationException");

    expectThrow<ValidationException>(
        []() {
            Gig g(1, "Valid Title", "A valid description here.",
                -5.0, GigCategory::OTHER);
        },
        "negative price -> ValidationException");

    expectThrow<ValidationException>(
        []() {
            Gig g(0, "Valid Title", "A valid description here.",
                100.0, GigCategory::OTHER);
        },
        "bad ownerID -> ValidationException");

    // Setter validation
    Gig g(1, "Valid Title", "A valid description here.",
        100.0, GigCategory::OTHER);

    expectThrow<ValidationException>(
        [&]() { g.setTitle(""); },
        "setTitle empty -> ValidationException");

    expectThrow<ValidationException>(
        [&]() { g.setPrice(0.0); },
        "setPrice zero -> ValidationException");
}

static void testValidGigStillWorks() {
    cout << "\n[Valid Gig construction still works after refactor]\n";

    Gig g(7, "Logo Design",
        "I will design a professional logo for your startup.",
        500.0, GigCategory::DESIGN);

    check(g.getOwnerID() == 7, "ownerID preserved");
    check(g.getTitle() == "Logo Design", "title preserved");
    check(g.getPrice() == 500.0, "price preserved");
    check(g.getCategory() == GigCategory::DESIGN, "category preserved");
    check(g.getIsActive() == true, "active by default");
}

// -------------------------
// Entry point
// -------------------------

int main() {
    cout << "=======================================\n";
    cout << " Module 2 Step 2: Exception hierarchy\n";
    cout << "=======================================\n";

    try {
        testExceptionHierarchy();
        testStdExceptionRoot();
        testGigThrowsValidationException();
        testValidGigStillWorks();
