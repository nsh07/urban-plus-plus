# Urban++

A simple C++ library for fetching words from [Urban Dictionary](https://urbandictionary.com)

This README is also the documentation for this library. This is a very simple and straightforward library so I thought of keeping the documentation in the README for quick and easy access.

## Table of contents
- [Getting Started](#getting-started)
    - [How to include](#how-to-include)
- [Usage](#usage)
    - [Basic usage](#basic-usage)
    - [Advanced usage and list of member functions](#advanced-usage)
        - [Initializing objects](#initializing-objects)
        - [Fetch random words](#fetching-random-words)
        - [List of getters](#list-of-getters)
        - [Error handling](#error-handling-and-timeout)
            - [If no result is found](#if-no-result-is-found)
            - [Timeout](#timeout)
- [Dependencies](#dependencies)

# Getting Started

## How to include

This is a single header library, so just copy `include/urban++.hpp` into your project's directory and you're good.

To include it, of course you'll need to do:

```cpp
#include "urban++.hpp"
```

When compiling your program that uses this library, don't forget to add the `-lcurl` option in your compile command to link the curl libraries. See the [dependencies](#dependencies) section for more info.

# Usage

## Basic usage

This library is designed to be simple and easy to use. You only need to set the search term and run the `fetch()` function, the more crude stuff like curl cleanup is handled by the library, so you don't need to care about memory leaks ( ͡° ͜ʖ ͡°)

Quickstart example:

```cpp
#include <iostream>
#include "urban++.hpp"

int main() {
    nm::Urban urban; // Initialize object
    urban.setSearchTerm("lol"); // Set the search term
    urban.fetch(); // Fetch results

    // Print the word, top definition, and author of top definition
    std::cout << "Word: " << urban.getTopWord() << std::endl
              << "Definition: " << urban.getTopDefinition() << std::endl
              << "Author: " << urban.getTopAuthor() << std::endl;
    
    return 0;
}
```

This program prints the top definition of the word "lol" and also prints the username of the author of the top definition. Note the usage of the `fetch()` member function, you need to call it every time you set the search term. See also: [advanced usage](#advanced-usage). `#include "urban++.hpp"` will be omitted in further examples.

## Advanced usage

This library can get you all sorts of info about the current word's search results on Urban Dictionary (yes redditors, that includes the upvote/downvote count). A list of available get functions is provided in [list of member functions](#list-of-getters).

### Initializing objects

To initialize an object,

```cpp
nm::Urban objectName;
```

You can also allocate the object on the heap using `new`, or for avoiding accidental memory leaks, using `std::unique_ptr`:

```cpp
nm::Urban *objectName = new nm::Urban;
...
delete objectName;
```

```cpp
#include <memory>
...
std::unique_ptr <nm::Urban> objectName (new nm::Urban);
// No delete required
```

After initializing the object, you need to set the search term using:

```cpp
objectName.setSearchTerm(word) // word is either an std::string or a char *
```

As an example, `objectname.setSearchTerm("lol")` sets the search term to our favourite word, lol.

After setting the search term, the results must be fetched using the `fetch()` function (this is the third time I'm talking about `fetch()` what's wrong with me), so for our good 'ol object `objectName` it will be `objectName.fetch()`. Now you can finally access the info available about the search results using the [get functions](#list-of-getters). **The return value of both the `fetch()` and [`fetchRandom()`](#fetching-random-words) functions is [`CURLcode`](https://curl.se/libcurl/c/libcurl-errors.html).** Please have a look at the [error handling](#error-handling-and-timeout) section.

### Fetching random words

You can fetch the search results of random words using the `fetchRandom()` member function. Note that fetching random words does not require a search term to be already set, but if one is already set, the search term will be left unchanged after `fetchRandom()` has done its job. After running `objectName.fetchRandom()`, you'll use the [getters](#list-of-getters) just like you would after running `fetch()`.

### List of getters

Yay we're talking about the getters now

The format for getters is: get<top/bottom/>\<property>(unsigned int index as argument in case of non-top/bottom getter)

getTop\<property>() returns the specific property (for example definition) of the top result in the vector of definitions for the current word from the list of results in the JSON. Same for the getBottom\<property>(), except that it returns the bottom most result.

get\<property>(unsigned int index) returns the property of the result at index \<index>

The type returned depends on the property, for example it is std::string for getDefinition(index) but it is
std::uint64_t for getThumbsUb(index). Given below is a list of all the getters excluding the top/bottom getters to give you an idea of the return values of each getter:

- `std::string getDefinition(unsigned int index)` returns the definition of the word at index `index`

- `std::uint64_t getThumbsUp(unsigned int index)` returns the number of likes (upvotes for redditors) for the definition at index `index`

- `std::uint64_t getThumbsDown(unsigned int index)` returns the number of dislikes (downvoves -_-) for the definition at index `index`

- `std::string getPermalink(unsigned int index)` returns the permalink of the definition at index `index`

- `std::vector <std::string> getSoundURLs(unsigned int index)` returns a vector of sound URLs for the word at index `index`

- `std::string getAuthor(unsigned int index)` returns the username of the author of the definition at index `index`

- `std::string getWord(unsigned int index)` returns the word at index `index`, useful in the case of `fetchRandom()` because each index may contain a different word

- `std::uint64_t getDefID(unsigned int index)` returns the ID of the definition at index `index`

- `std::string getWrittenOn(unsigned int index)` returns the date and time at which the definition at index `index` was submitted. This is in the format "YYYY-MM-DDTHH:MM:SS.XXXZ" where YYYY is the year, MM is the month, DD is the day, T is the date-time separator, HH, MM, SS are hours, minutes and seconds respectively, XXX is miliseconds, and Z is the ending separator.

- `std::string getExample(unsigned int index)` returns the example given with the definition at index `index`

### Error handling and timeout

The return value of both the `fetch()` and `fetchRandom()` functions is [`CURLcode`](https://curl.se/libcurl/c/libcurl-errors.html), which you can utilize to print out any errors occured while fetching the results. An example is given below:

```cpp
#include <iostream>

#include <curl/curl.h>
#include "urban++.hpp"

int main() {
    nm::Urban objectName; // Initialize object
    objectName.setSearchTerm("lol"); // Set search term
    CURLcode err_code = objectName.fetch(); // Get the return value of fetch() to find any errors
    if (err_code == CURLE_OK) { // If no error occurs
        std::cout << "Word: " << objectName.getTopWord() << std::endl
                  << "Definition:\n" << objectName.getTopDefinition() << std::endl
                  << "Author: " << objectName.getTopAuthor() << std::endl;
    }
    else { // If some error occured
        std::cerr << "An error occured when fetching search results: " << curl_easy_strerror(err_code) << std::endl;
        return 1;
    };
    return 0;
}
```

This program has the following output when the device does not have an internet connection:

```
An error occured when fetching search results: Couldn't resolve host name
```

The above program makes use of [`curl_easy_strerror()`](https://curl.se/libcurl/c/curl_easy_strerror.html) to convert the error code from a `CURLcode` object (note that the return value of `objectName.fetch()` is assigned to `err_code`) to a string, which is then printed. This curl function is useful in user-facing programs which need simple error texts instead of technical error codes.

#### If no result is found

If no search results are found, the API URL returns an empty list. This can lead to errors if you try to access definitions from any index. To avoid this, the `fetch()` member function returns `CURLE_GOT_NOTHING` if no search results can be found for the current search term. To utilise this, modify your error-catching if/else statements accordingly:

```cpp
...
err_code = objectName.fetch();

if (err_code == CURLE_OK) { // Successfully fetched search results
    ...
}
if (err_code == CURLE_GOT_NOTHING) { // No search results for given word
    std::cerr << "No search results found for the given word." << std::endl;
}
else { // Some other error occured
    std::cerr << "An error occured when fetching search results: " << curl_easy_strerror(err_code) << std::endl;
};
...
```

#### Timeout

There is a timeout duration of 30 seconds set by the class, which is pretty much enough since this program only fetches plain text JSON lists which are never more than a few KBs in size.

# Dependencies

This library depends on:

- The [curl](https://github.com/curl/curl/tree/master/include/curl) library by [Daniel Stenberg](https://github.com/bagder)

- The [JSON for Modern C++](https://github.com/nlohmann/json) library by [Niels Lohmann](https://github.com/nlohmann)

Thank you guys for creating such awesome libraries, very cool
