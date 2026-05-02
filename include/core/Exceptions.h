#ifndef SKILLBRIDGE_EXCEPTIONS_H
#define SKILLBRIDGE_EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include <cstdio>


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

class UnauthorizedException : public SkillBridgeException {
public:
    explicit UnauthorizedException(const std::string& msg)
        : SkillBridgeException("Unauthorized: " + msg) {
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

class GigNotFoundException : public SkillBridgeException {
public:
    explicit GigNotFoundException(const std::string& msg)
        : SkillBridgeException("Gig not found: " + msg) {
    }
};

class InsufficientFundsException : public SkillBridgeException {
private:
    double required_;
    double available_;

public:
    InsufficientFundsException(double required, double available)
        : SkillBridgeException(buildMessage(required, available)),
        required_(required),
        available_(available) {
    }

    double getRequired()  const { return required_; }
    double getAvailable() const { return available_; }

private:
    
    static std::string buildMessage(double required, double available)
    {
       
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Insufficient funds: required Rs. %.2f, available Rs. %.2f", required, available);
        return std::string(buf);
    }
};


class OrderNotFoundException : public SkillBridgeException {
public:
    explicit OrderNotFoundException(const std::string& msg) : SkillBridgeException("Order not found: " + msg) 
    {

    }
};


#endif