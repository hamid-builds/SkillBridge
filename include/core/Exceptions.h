#ifndef SKILLBRIDGE_EXCEPTIONS_H
#define SKILLBRIDGE_EXCEPTIONS_H

#include <stdexcept>
#include <string>

class SkillBridgeException : public std::runtime_error 
{
public:
    explicit SkillBridgeException(const std::string& msg) : std::runtime_error(msg) 
    {

    }
};

class ValidationException : public SkillBridgeException 
{
public:
    explicit ValidationException(const std::string& msg) : SkillBridgeException("Validation error: " + msg) 
    {

    }
};

class AuthenticationException : public SkillBridgeException 
{
public:
    explicit AuthenticationException(const std::string& msg) : SkillBridgeException("Authentication error: " + msg) 
    {

    }
};

class DuplicateEntryException : public SkillBridgeException 
{
public:
    explicit DuplicateEntryException(const std::string& msg) : SkillBridgeException("Duplicate entry: " + msg) 
    {

    }
};

class DatabaseException : public SkillBridgeException 
{
public:
    explicit DatabaseException(const std::string& msg) : SkillBridgeException("Database error: " + msg) 
    {

    }
};

class RateLimitException : public SkillBridgeException 
{
public:
    explicit RateLimitException(const std::string& msg) : SkillBridgeException("Rate limit exceeded: " + msg) 
    {

    }
};
// Thrown when a Gig field fails validation (bad title, price, etc.).
class InvalidGigException : public SkillBridgeException {
public:
    explicit InvalidGigException(const std::string& msg)
        : SkillBridgeException(msg) {
    }
};

// Thrown when a lookup for a Gig by ID returns no result.
class GigNotFoundException : public SkillBridgeException {
public:
    explicit GigNotFoundException(const std::string& msg)
        : SkillBridgeException(msg) {
    }
};
#endif