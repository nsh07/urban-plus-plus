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
#include <initializer_list>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

using easy_escape_t = std::unique_ptr<char, decltype(&curl_free)>;

namespace nm
{
    class Initializer
    {
        /*
        Class to globally initialize the libcurl environment. Not copyable since there should only
        be one instance of this class in a program.
        The constructor inits the environment and the destructor cleans it up.
        */
        public:
        Initializer()
        {
            curl_global_init(CURL_GLOBAL_ALL);
        }
        ~Initializer()
        {
            curl_global_cleanup();
        }

        Initializer(const Initializer& that) = delete;
        Initializer& operator=(const Initializer& that) = delete;
    };


    class Urban
    {
        private:
        std::string searchResult, searchTermBackup;
        const std::string randomURL, urlPrefix;
        int sizeOfList;

        CURL* curl;

        CURLcode res;
        nlohmann::json resultJSON;

        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) // member function to write URL JSON data to std::string
        {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        public:
        Urban() // Constructor
        : randomURL("https://api.urbandictionary.com/v0/random")
        , urlPrefix("https://api.urbandictionary.com/v0/define?term=")
        , curl(curl_easy_init())
        {
            /*
            Set all curl options and initialize stuff. Exit with an error if the curl object fails to
            initialize for some reason (rare)
            */
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &searchResult);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
            }
            else {
                std::cerr << "Fatal error: Unable to initialize curl object." << std::endl;
                exit(1);
            }
        }

        Urban(const Urban& that) // Copy constructor
        : randomURL(that.randomURL)
        , urlPrefix(that.urlPrefix)
        , curl(curl_easy_duphandle(that.curl))
        {
            /*
            What we need to do here:
            1. Duplicate the curl handle and initialize the URL variables
            2. Copy the other (needed) variables
            3. Set the CURLOPT_WRITEDATA option again so that it points to the string belonging
               to the new object and not the older one
            4. Set the CURLOPT_URL option so that running fetch() after copying will not result in an error
               or blank response JSON
            */
            this->searchTermBackup = that.searchTermBackup;
            this->sizeOfList = that.sizeOfList;
            this->resultJSON = that.resultJSON;

            if (curl) {
                curl_easy_setopt(this->curl, CURLOPT_WRITEDATA, &(this->searchResult)); // Changes the string to which transferred data is stored to the one in the new object
                curl_easy_setopt(curl, CURLOPT_URL, (urlPrefix + searchTermBackup).data());
            }
            else {
                std::cerr << "Fatal error: Unable to copy object because of inability to duplicate curl handle." << std::endl;
                exit(1);
            }
        }

        Urban& operator=(const Urban& that) // Copy assignment operator
        {
            /* 
            We don't need to do anything much here. Only copying the backed up search term, size of the list
            and the result JSON object is enough, since the curl handle is already initialized properly
            */
            this->searchTermBackup = that.searchTermBackup;
            this->sizeOfList = that.sizeOfList;
            this->resultJSON = that.resultJSON;
            curl_easy_setopt(curl, CURLOPT_URL, (urlPrefix + searchTermBackup).data());
            return *this;
        }

        ~Urban() // Destructor
        {
            /* Only need to cleanup the curl handle, no other resources are handled by the class */
            curl_easy_cleanup(curl);
        }


        void setSearchTerm(const std::string term) // Sets the search term
        {
            /* URL encode the search term, back it up to another string and set the CURLOPT_URL option */
            easy_escape_t searchTerm(curl_easy_escape(curl, term.data(), term.size()), curl_free);
            searchTermBackup = searchTerm.get();
            curl_easy_setopt(curl, CURLOPT_URL, (urlPrefix + searchTerm.get()).data());
        }
        
        void setSearchTerm(const char *term) // Overloaded member function for char * type
        {
            /* Same as the OG setSearchTerm but with a char* */
            easy_escape_t searchTerm(curl_easy_escape(curl, term, 0), curl_free);
            searchTermBackup = searchTerm.get();
            curl_easy_setopt(curl, CURLOPT_URL, (urlPrefix + searchTerm.get()).data());
        }

        CURLcode fetch() // Fetch results and return CURLcode for success/any errors
        {
            /*
            What happens here:
            1. Perform the transfer
            2. Check the error code
              i.  If ok, proceed to 3.
              ii. If not, return the error code without changing anything
            3. Parse the recieved JSON response, clear the searchResult string, calculate and set the size of the result
               list.
              i.  If result list is empty, return CURLE_GOT_NOTHING
            */
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                resultJSON = nlohmann::json::parse(searchResult);
                searchResult = "";
                sizeOfList = resultJSON["list"].size();
                if (sizeOfList == 0) return CURLE_GOT_NOTHING;
            }
            return res;
        }
        
        CURLcode fetchRandom() // Fetch results for random words
        {
            /*
            Same as the OG fetch(), but restores the original URL from the backed up search term
            before returning. Also, we do not need to check if the result list is empty since the random
            URL is guaranteed to return something.
            */
            curl_easy_setopt(curl, CURLOPT_URL, randomURL.data());
            
            res = curl_easy_perform(curl);
            if (res == CURLE_OK) {
                resultJSON = nlohmann::json::parse(searchResult);
                searchResult = "";
                sizeOfList = resultJSON["list"].size();
            }

            curl_easy_setopt(curl, CURLOPT_URL, (urlPrefix + searchTermBackup).data());
            return res;
        }

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
}

#endif
