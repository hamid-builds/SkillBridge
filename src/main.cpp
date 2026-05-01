#include <iostream>
#include <string>
#include <sstream>

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
            << " (wrong exception type: " << e.what() << ")\n";
        g_fail++;
    }
}



static void testCategoryEnum() {
    cout << "\n[GigCategory enum]\n";

    check(gigCategoryToString(GigCategory::DESIGN) == "DESIGN",
        "toString DESIGN");
    check(gigCategoryToString(GigCategory::CODING) == "CODING",
        "toString CODING");
    check(gigCategoryToString(GigCategory::OTHER) == "OTHER",
        "toString OTHER");

    check(gigCategoryFromString("DESIGN") == GigCategory::DESIGN,
        "fromString DESIGN uppercase");
    check(gigCategoryFromString("design") == GigCategory::DESIGN,
        "fromString design lowercase");
    check(gigCategoryFromString("Design") == GigCategory::DESIGN,
        "fromString Design mixed case");
    check(gigCategoryFromString("MARKETING") == GigCategory::MARKETING,
        "fromString MARKETING");

    expectThrow<invalid_argument>(
        []() { gigCategoryFromString("nonsense"); },
        "fromString rejects unknown category");
}

static void testValidGigConstruction() {
    cout << "\n[Gig construction: valid inputs]\n";

    Gig g(7,
        "Logo Design",
        "I will design a professional logo for your brand or startup.",
        500.0,
        GigCategory::DESIGN);

    check(g.getGigID() == 0, "new Gig has gigID 0 (not yet saved)");
    check(g.getOwnerID() == 7, "ownerID set to 7");
    check(g.getTitle() == "Logo Design", "title stored");
    check(g.getPrice() == 500.0, "price stored");
    check(g.getCategory() == GigCategory::DESIGN, "category stored");
    check(g.getIsActive() == true, "new Gig is active by default");
    check(g.getCreatedAt() == "", "createdAt empty until DB assigns it");
}

static void testInvalidGigConstruction() {
    cout << "\n[Gig construction: validation]\n";

    // Title too short
    expectThrow<InvalidGigException>(
        []() {
            Gig g(1, "Lo", "A valid description here.", 100.0,
                GigCategory::OTHER);
        },
        "title < 3 chars rejected");

    // Title empty
    expectThrow<InvalidGigException>(
        []() {
            Gig g(1, "", "A valid description here.", 100.0,
                GigCategory::OTHER);
        },
        "empty title rejected");

    // Title too long
    expectThrow<InvalidGigException>(
        []() {
            string longTitle(150, 'x');
            Gig g(1, longTitle, "A valid description here.", 100.0,
                GigCategory::OTHER);
        },
        "title > 100 chars rejected");

    // Description too short
    expectThrow<InvalidGigException>(
        []() {
            Gig g(1, "Valid Title", "short", 100.0, GigCategory::OTHER);
        },
        "description < 10 chars rejected");

    // Price zero
    expectThrow<InvalidGigException>(
        []() {
            Gig g(1, "Valid Title", "A valid description here.", 0.0,
                GigCategory::OTHER);
        },
        "price 0 rejected");

    // Price negative
    expectThrow<InvalidGigException>(
        []() {
            Gig g(1, "Valid Title", "A valid description here.", -50.0,
                GigCategory::OTHER);
        },
        "negative price rejected");

    // Price too high
    expectThrow<InvalidGigException>(
        []() {
            Gig g(1, "Valid Title", "A valid description here.", 2000000.0,
                GigCategory::OTHER);
        },
        "price >= 1,000,000 rejected");

    // Bad ownerID
    expectThrow<InvalidGigException>(
        []() {
            Gig g(0, "Valid Title", "A valid description here.", 100.0,
                GigCategory::OTHER);
        },
        "ownerID 0 rejected");

    expectThrow<InvalidGigException>(
        []() {
            Gig g(-5, "Valid Title", "A valid description here.", 100.0,
                GigCategory::OTHER);
        },
        "negative ownerID rejected");
}

static void testSetters() {
    cout << "\n[Gig setters with validation]\n";

    Gig g(1, "Original Title", "Original description works fine.",
        200.0, GigCategory::WRITING);

    g.setTitle("Updated Title");
    check(g.getTitle() == "Updated Title", "setTitle updates value");

    g.setPrice(350.0);
    check(g.getPrice() == 350.0, "setPrice updates value");

    g.setCategory(GigCategory::CODING);
    check(g.getCategory() == GigCategory::CODING, "setCategory updates value");

  
    g.setGigID(42);
    check(g.getGigID() == 42, "setGigID assigns DB ID");

    g.setCreatedAt("2026-04-25 10:00:00");
    check(g.getCreatedAt() == "2026-04-25 10:00:00",
        "setCreatedAt stores timestamp");

    
    expectThrow<InvalidGigException>(
        [&]() { g.setTitle(""); },
        "setTitle rejects empty");

    expectThrow<InvalidGigException>(
        [&]() { g.setPrice(-1.0); },
        "setPrice rejects negative");

    // Deactivate
    check(g.getIsActive() == true, "gig is active before deactivate");
    g.deactivate();
    check(g.getIsActive() == false, "deactivate sets isActive to false");
    g.setIsActive(true);
    check(g.getIsActive() == true, "setIsActive can reactivate");
}

static void testOperatorOverloads() {
    cout << "\n[Gig operator overloads]\n";

    Gig a(1, "Gig A", "Description for gig A here.",
        100.0, GigCategory::DESIGN);
    Gig b(2, "Gig B", "Description for gig B here.",
        200.0, GigCategory::CODING);
    Gig c(1, "Different Title", "Different description entirely.",
        999.0, GigCategory::OTHER);

    
    a.setGigID(1);
    b.setGigID(2);
    c.setGigID(1);  

    
    check(a == c, "operator==: same gigID means equal (content ignored)");
    check(!(a == b), "operator==: different gigID means not equal");
    check(a != b, "operator!=: different gigID returns true");

   
    check(a < b, "operator<: 100 < 200 by price");
    check(!(b < a), "operator<: 200 not < 100");

   
    ostringstream oss;
    oss << a;
    string out = oss.str();
    check(out.find("Gig A") != string::npos,
        "operator<<: output contains title");
    check(out.find("DESIGN") != string::npos,
        "operator<<: output contains category");
    check(out.find("100") != string::npos,
        "operator<<: output contains price");
    check(out.find("active") != string::npos,
        "operator<<: output contains active flag");
}

static void testDefaultConstructor() {
    cout << "\n[Gig default constructor]\n";

    Gig empty;
    check(empty.getGigID() == 0, "default gigID is 0");
    check(empty.getOwnerID() == 0, "default ownerID is 0");
    check(empty.getTitle() == "", "default title empty");
    check(empty.getPrice() == 0.0, "default price 0");
    check(empty.getCategory() == GigCategory::OTHER, "default category OTHER");
    check(empty.getIsActive() == true, "default isActive true");
}



int main() {
    cout << "=======================================\n";
    cout << " Module 2 Step 1: Gig + GigCategory\n";
    cout << "=======================================\n";

    try {
        testCategoryEnum();
        testValidGigConstruction();
        testInvalidGigConstruction();
        testSetters();
        testOperatorOverloads();
        testDefaultConstructor();
    }
    catch (const exception& e) {
        cout << "\nUNEXPECTED EXCEPTION AT TOP LEVEL: " << e.what() << "\n";
        g_fail++;
    }

    cout << "\n---------------------------------------\n";
    cout << "Passed: " << g_pass << "  Failed: " << g_fail << "\n";
    cout << "---------------------------------------\n";

    return g_fail == 0 ? 0 : 1;
}