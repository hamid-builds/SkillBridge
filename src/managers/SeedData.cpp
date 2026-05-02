#include "managers/SeedData.h"
#include "core/Gig.h"
#include "core/GigCategory.h"

using namespace std;


namespace {
    struct SeedEntry {
        const char* title;
        const char* description;
        double price;
        GigCategory category;
    };

   
    
    const SeedEntry SEED_GIGS[] = {
       
        { "Python Web Developer",
          "I will build full-stack Django and Flask applications with REST APIs",
          15000.0, GigCategory::CODING },
        { "Python Data Scripts",
          "Automation scripts in Python for scraping, cleaning, and reporting",
          5000.0, GigCategory::CODING },
        { "JavaScript Frontend Engineer",
          "React and Vue applications with modern build tools and testing",
          18000.0, GigCategory::CODING },
        { "Mobile App Developer",
          "Native Android and iOS apps using Kotlin, Swift, and React Native",
          35000.0, GigCategory::CODING },
        { "WordPress Site Setup",
          "Complete WordPress installation, theme, and plugin configuration",
          8000.0, GigCategory::CODING },

         
          { "Modern Logo Design",
            "Professional logo design with three concept variants and source files",
            5000.0, GigCategory::DESIGN },
          { "Brand Identity Package",
            "Complete brand identity including logo, color palette, and typography",
            25000.0, GigCategory::DESIGN },
          { "UI UX Design",
            "Mobile and web interface design in Figma with interactive prototypes",
            20000.0, GigCategory::DESIGN },
          { "Social Media Graphics",
            "Custom social media post designs for Instagram and Facebook campaigns",
            3000.0, GigCategory::DESIGN },

            
            { "Blog Article Writer",
              "SEO-optimized blog articles in fluent English on technology topics",
              2000.0, GigCategory::WRITING },
            { "Technical Documentation",
              "Clear technical documentation for software products and APIs",
              12000.0, GigCategory::WRITING },
            { "Resume and Cover Letter",
              "Professional resume writing and cover letter customization service",
              4000.0, GigCategory::WRITING },

             
              { "Digital Marketing Strategy",
                "Complete digital marketing plan including SEO, social media, and ads",
                30000.0, GigCategory::MARKETING },
              { "Facebook Ads Campaign",
                "Targeted Facebook and Instagram ad campaigns with weekly reports",
                15000.0, GigCategory::MARKETING },
              { "SEO Optimization",
                "On-page and off-page SEO with keyword research and backlink building",
                18000.0, GigCategory::MARKETING },

               
                { "Mathematics Tutor",
                  "One-on-one tutoring for high school and college mathematics topics",
                  1500.0, GigCategory::TUTORING },
                { "Python Programming Tutor",
                  "Beginner-friendly Python programming lessons for students and adults",
                  2000.0, GigCategory::TUTORING },
                { "English Language Coach",
                  "Spoken English coaching and IELTS preparation with practice tests",
                  2500.0, GigCategory::TUTORING },

                 
                  { "Video Editing Services",
                    "Professional video editing for YouTube, weddings, and corporate events",
                    10000.0, GigCategory::OTHER },
                  { "Voiceover Recording",
                    "Professional voiceover recording in clear American and British accents",
                    3500.0, GigCategory::OTHER },
    };

    constexpr int SEED_COUNT = sizeof(SEED_GIGS) / sizeof(SEED_GIGS[0]);
}

int SeedData::seedGigs(IGigRepository& repo,
    const DataList<int>& freelancerIDs) {
    if (freelancerIDs.size() == 0) {
        return 0;
    }

    int saved = 0;
    int n = freelancerIDs.size();

    for (int i = 0; i < SEED_COUNT; ++i) {
        
        int ownerID = freelancerIDs.get(i % n);

       
        Gig g(ownerID,
            SEED_GIGS[i].title,
            SEED_GIGS[i].description,
            SEED_GIGS[i].price,
            SEED_GIGS[i].category);

       
        repo.saveGig(g);
        ++saved;
    }

    return saved;
}