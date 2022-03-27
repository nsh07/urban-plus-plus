/*

    Copyright (c) 2021, 2022 Nishant Mishra <https://github.com/NMrocks>

    This software is released under the MIT License.
    See https://choosealicense.com/licenses/mit/ for more info.

*/

/*
This program works by fetching the JSON from the Urban Dictionary API url at api.urbandictionary.com
(https://dev.to/nhighleysalongenius/comment/epgk)
I don't know how this works or when Urban dictionary might lock this API down, but you can use this lib
until then :P
*/

#ifndef URBAN_PLUS_PLUS
#define URBAN_PLUS_PLUS

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace nm
{
    class Urban
    {
        private:
        std::string searchResult, searchTermBackup;
        char* searchTerm;
        std::string randomURL = "https://api.urbandictionary.com/v0/random";
        std::string urlPrefix = "https://api.urbandictionary.com/v0/define?term=";
        int sizeOfList;

        CURL *curl;
        CURLcode res;
        nlohmann::json resultJSON;

        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) // member function to write URL JSON data to std::string
        {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        public:
        Urban() // Sets all curl options
        {
            curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &searchResult);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
            }
            else {
                std::cerr << "Fatal error: Unable to initialize curl object.";
                exit(1);
            }
        }
        ~Urban()
        {
            curl_easy_cleanup(curl);
        }

        void setSearchTerm(const std::string term); // Sets the search term
        void setSearchTerm(const char *term); // Overloaded member function for char * type

        CURLcode fetch(); // Fetch results and return CURLcode for success/any errors
        CURLcode fetchRandom(); // Fetch results for a random word

        /*
        The format for getters is: get<top/bottom/><property>(unsigned int index as argument in case of non-top/bottom getter)
        
        getTop<property>() returns the specific property (for example definition) of the top result in the array of definitions
        for the current word from the list of results in the JSON
        Same for the getBottom<property>(), except that it returns the bottom most result.

        get<property>(unsigned int index) returns the property of the result at index <index>

        The type returned depends on the property, for example it is std::string for getDefinition(index) but it is
        std::uint64_t for getThumbsUb(index)
        */

        std::string getDefinition(unsigned int index) { return resultJSON["list"][index]["definition"]; }
        std::string getTopDefinition() { return getDefinition(0); }
        std::string getBottomDefinition() { return getDefinition(sizeOfList - 1); }

        std::uint64_t getThumbsUp(unsigned int index) { return resultJSON["list"][index]["thumbs_up"]; }
        std::uint64_t getTopThumbsUp() { return getThumbsUp(0); }
        std::uint64_t getBottomThumbsUp() { return getThumbsUp(sizeOfList - 1); }
        std::uint64_t getThumbsDown(unsigned int index) { return resultJSON["list"][index]["thumbs_down"]; }
        std::uint64_t getTopThumbsDown() { return getThumbsDown(0); }
        std::uint64_t getBottomThumbsDown() { return getThumbsDown(sizeOfList - 1); }

        std::string getPermalink(unsigned int index) { return resultJSON["list"][index]["permalink"]; }
        std::string getTopPermalink() { return getPermalink(0); }
        std::string getBottomPermalink() { return getPermalink(sizeOfList - 1); }

        std::vector <std::string> getSoundURLs(unsigned int index) { return resultJSON["list"][index]["sound_urls"]; }
        std::vector <std::string> getTopSoundURLs() { return getSoundURLs(0); }
        std::vector <std::string> getBottomSoundURLs() { return getSoundURLs(sizeOfList - 1); }
        
        std::string getAuthor(unsigned int index) { return resultJSON["list"][index]["author"]; }
        std::string getTopAuthor() { return getAuthor(0); }
        std::string getBottomAuthor() { return getAuthor(sizeOfList - 1); }

        std::string getWord(unsigned int index) { return resultJSON["list"][index]["word"]; }
        std::string getTopWord() { return getWord(0); }
        std::string getBottomWord() { return getWord(sizeOfList - 1); }

        std::uint64_t getDefID(unsigned int index) { return resultJSON["list"][index]["defid"]; }
        std::uint64_t getTopDefID() { return getDefID(0); }
        std::uint64_t getBottomDefID() { return getDefID(sizeOfList - 1); }

        std::string getWrittenOn(unsigned int index) { return resultJSON["list"][index]["written_on"]; }
        std::string getTopWrittenOn() { return getWrittenOn(0); }
        std::string getBottomWrittenOn() { return getWrittenOn(sizeOfList - 1); }

        std::string getExample(unsigned int index) { return resultJSON["list"][index]["example"]; }
        std::string getTopExample() { return getExample(0); }
        std::string getBottomExample() { return getExample(sizeOfList - 1); }

        int sizeOfJSON() { return sizeOfList; } // Returns the number of items in the list of definitions in the json
        nlohmann::json rawJSON() { return resultJSON; } // Returns the raw JSON do the user to extract data from manually
    };

    void Urban::setSearchTerm(const std::string term)
    {
        searchTerm = curl_easy_escape(curl, term.data(), term.size()); // Replaces ASCII characters with their URL encoded strings
        searchTermBackup = searchTerm;
        curl_easy_setopt(curl, CURLOPT_URL, (urlPrefix + searchTerm).data());
        curl_free(searchTerm);
    }
    
    void Urban::setSearchTerm(const char *term)
    {
        searchTerm = curl_easy_escape(curl, term, 0);
        searchTermBackup = searchTerm;
        curl_easy_setopt(curl, CURLOPT_URL, (urlPrefix + searchTerm).data());
        curl_free(searchTerm);
    }

    CURLcode Urban::fetch()
    {
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            resultJSON = nlohmann::json::parse(searchResult); // Parse JSON result string and save to resultJSON
            sizeOfList = resultJSON["list"].size(); // Set sizeOfList to the length of "list" vector in JSON
            if (sizeOfList == 0) return CURLE_GOT_NOTHING; // Return CURLE_GOT_NOTHING if the word does not have any search results
            searchResult = ""; // Resets the search result string
        }
        return res;
    }

    CURLcode Urban::fetchRandom()
    {
        curl_easy_setopt(curl, CURLOPT_URL, randomURL.data()); // Sets the URL to the random URL
        
        res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            resultJSON = nlohmann::json::parse(searchResult);
            sizeOfList = resultJSON["list"].size();
            searchResult = "";
        }

        curl_easy_setopt(curl, CURLOPT_URL, (urlPrefix + searchTermBackup).data()); // Restores the original search term URL
        return res;
    }
}

#endif