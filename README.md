# SkillBridge: Campus Freelancing Platform

SkillBridge is an Object-Oriented Programming final project built in C++17 for FAST NUCES Lahore. It simulates a complete student freelancing marketplace with role-based workflows for Clients, Freelancers, and Admins, and supports both:

- Web mode (REST API + frontend)
- CLI mode (terminal menus)

---

## Technical Highlights

### 1. Dual-Mode Application
- Same backend logic is reused by Web and CLI interfaces.
- Web mode serves pages from `public/` and exposes API routes.
- CLI mode provides full text-based interaction for major workflows.

### 2. OOP + SOLID Architecture
- Inheritance hierarchy: `User` -> `Client`, `Freelancer`, `Admin`.
- Dedicated managers per domain (`UserManager`, `GigManager`, `OrderManager`, `MessageManager`, `ReviewManager`, `SkillGraphManager`).
- Interface-driven repositories (`IUserRepository`, `IGigRepository`, `IOrderRepository`, etc.) with SQLite implementations.
- Separation of domain, persistence, orchestration, and UI layers.

### 3. Design Patterns Implemented
- Singleton: `DatabaseManager`
- Factory: `UserFactory`
- Strategy: ranking, fuzzy matching, sentiment analyzer
- Command: order actions with undo/redo history
- pImpl: HTTP server internals hidden behind clean interface

### 4. Search and Ranking Stack
- Inverted index for fast lookup.
- TF-IDF relevance scoring.
- Levenshtein edit distance for typo tolerance.
- Soundex phonetic fallback.
- Trie-based autocomplete.

### 5. Messaging + Optimization
- Huffman compression for stored message payloads.
- Decoding pipeline via manager layer.
- LRU caching for repeated conversation fetches.

### 6. Trust and Reputation Graph
- Endorsements form a directed weighted graph.
- PageRank for freelancer ranking.
- BFS traversal for trusted-nearby recommendations.

---

## Core Features (Implemented)

1. Role-based registration/login with session auth.
2. Password hashing (SHA-256).
3. Rate limiting for auth flows (token bucket).
4. Bloom filter assisted duplicate-email checks.
5. Gig create/edit/deactivate + admin delete.
6. Category/sort-based gig browse.
7. Smart search (exact + fuzzy + phonetic fallback) with autocomplete.
8. Order placement with deadline and balance validation.
9. Order state machine transitions.
10. Automatic refund/payout behavior on status updates.
11. Undo/redo for order commands.
12. Messaging inbox, threads, unread counts, mark-as-read.
13. Review submission only on completed buyer-owned orders.
14. Sentiment scoring + rating aggregation.
15. Endorsement create/remove and given/received views.
16. PageRank leaderboard + BFS trust recommendations.
17. Admin moderation for users, gigs, and reviews.
18. Full frontend pages for browse, search, profile, messages, orders, endorsements, admin.

---

## Algorithms and Data Structures

### Algorithms
- SHA-256
- Token Bucket
- TF-IDF
- Levenshtein Distance (DP)
- Soundex
- Merge Sort
- Binary Search (+ bounds)
- Huffman Coding
- Bag-of-Words Sentiment Scoring
- PageRank
- BFS

### Custom Data Structures
- `DataList<T>`
- `HashMap<K, V>`
- `BloomFilter`
- `AutocompleteTrie`
- `InvertedIndex`
- `MinHeap<T, Compare>`
- `LRUCache<K, V>`
- `SkillGraph`

---

## Database Schema (SQLite)

Auto-initialized tables:
- `users`
- `gigs`
- `orders`
- `messages`
- `reviews`
- `endorsements`

Key details:
- Foreign keys enabled
- Cascading deletes
- DB-level constraints for validation safety
- Indices added for commonly queried paths

---

## API Surface (High-Level)

- Health: `/api/health`
- Auth: `/api/register`, `/api/login`, `/api/logout`, `/api/me`
- Gigs: browse/search/autocomplete/detail/create/update/delete/active toggle
- Users: public profile, self profile update, password change, deposit, delete
- Orders: list/detail/place/status update/review submit
- Messages: unread, inbox, conversation, send
- Endorsements: create/delete/given/received/rankings/trusted
- Admin: user management, gig listing, review delete

Server entrypoint: `src/api/HttpServer.cpp`

---

## Project Structure

- `src/main.cpp`: startup wiring and mode selection
- `src/core/`: entities, enums, exceptions
- `src/managers/`: business logic and repositories
- `src/utils/`: custom algorithms and data structures
- `src/strategies/`: sentiment strategy
- `src/api/`: HTTP server and JSON serializers
- `src/cli/`: terminal app
- `public/`: frontend assets
- `third_party/sqlite/`: embedded SQLite source
- `include/`: headers

---

## Build and Run

### Requirements
- Visual Studio 2022
- C++17 toolset (`v143`)

### Build Steps
1. Open `SkillBridge.sln`
2. Select `Debug | x64`
3. Build the project

### Run (Web Mode)
Run directly from Visual Studio (Start Debugging / Local Windows Debugger).

### Run (CLI Mode)
Open terminal, go to the project directory, then run:

```bash
"x64/Debug/SkillBridge.exe" --cli
```

---

## First Run Notes

- `skillbridge.db` is created automatically if missing.
- Default admin is seeded once:
  - Email: `admin@skillbridge.com`
  - Password: `Admin@4321`

---

## Team

This project was developed as a collaborative OOP final project by:

- **Ashir Bin Asif** (`25L-0021`)
- **Muhammad Sufiyan** (`25L-3020`)
- **Muhammad Hamid Raza Qadri** (`25L-0046`)
- **Muhammad Farrukh Usama** (`25L-0026`)

---

## Institution

National University of Computer and Emerging Sciences (FAST NUCES), Lahore  
Department of Computer Science
