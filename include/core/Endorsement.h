#ifndef SKILLBRIDGE_ENDORSEMENT_H
#define SKILLBRIDGE_ENDORSEMENT_H

#include <string>
#include <ostream>



class Endorsement {
public:
    Endorsement() = default;

    
    Endorsement(int fromUserID, int toUserID,
        const std::string& skill, double weight = 1.0);

    int                getEndorsementID() const { return endorsementID_; }
    int                getFromUserID()    const { return fromUserID_; }
    int                getToUserID()      const { return toUserID_; }
    double             getWeight()        const { return weight_; }
    const std::string& getSkill()         const { return skill_; }
    const std::string& getTimestamp()     const { return timestamp_; }

    void setEndorsementID(int id) { endorsementID_ = id; }
    void setFromUserID(int id) { fromUserID_ = id; }
    void setToUserID(int id) { toUserID_ = id; }
    void setSkill(const std::string& s) { skill_ = s; }
    void setWeight(double w) { weight_ = w; }
    void setTimestamp(const std::string& t) { timestamp_ = t; }

    bool operator==(const Endorsement& o) const { return endorsementID_ == o.endorsementID_; }
    bool operator< (const Endorsement& o) const { return timestamp_ < o.timestamp_; }

    friend std::ostream& operator<<(std::ostream& os, const Endorsement& e);

private:
    int         endorsementID_ = 0;
    int         fromUserID_ = 0;
    int         toUserID_ = 0;
    std::string skill_;
    double      weight_ = 1.0;
    std::string timestamp_;
};

#endif