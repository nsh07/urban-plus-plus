# Urban++

A simple C++ library for fetching words from [Urban Dictionary](https://urbandictionary.com)

This README is also the documentation for this library.

## Table of contents
- [Getting Started](#getting-started)
    - [How to include](#how-to-include)
- [Usage](#usage)
    - [Basic usage](#basic-usage)
    - [Advanced usage and list of member functions](#advanced-usage)
        - [Initializing the transfer environment](#initializing-the-transfer-environment)
        - [Initializing objects](#initializing-objects)
        - [Fetch random words](#fetching-random-words)
        - [List of getters](#list-of-getters)
        - [Raw JSON and number of definitions](#raw-json-and-number-of-definitions)
        - [Error handling](#error-handling-and-timeout)
            - [If no result is found](#if-no-result-is-found)
            - [Timeout](#timeout)
- [Dependencies](#dependencies)

# Getting Started

## How to include

This is a single header library, so you can just copy `include/urban++.hpp` into your project's directory.

To include it the header:

```cpp
#include "urban++.hpp"
```

Add the `-lcurl` option in your compile command to link the required curl libraries. See the [dependencies](#dependencies) section for more info.

# Usage

## Basic usage

You only need to set the search term and run the `fetch()` function to fetch the results.

Quickstart example:

```cpp
#include <iostream>
#include "urban++.hpp"

int main() {
    static nm::Initializer init; // Initialize transfer environment
    nm::Urban urban; // Initialize object
    urban.setSearchTerm("hello"); // Set the search term
    urban.fetch(); // Fetch results

    // Print the word, top definition, and author of top definition
    std::cout << "Word: " << urban.getTopWord() << std::endl
              << "Definition: " << urban.getTopDefinition() << std::endl
              << "Author: " << urban.getTopAuthor() << std::endl;
    
    return 0;
}
```

This program prints the top definition of the word "hello" and also prints the username of the author of the top definition. Note the usage of the `fetch()` member function, you need to call it every time you set the search term. Also note the unused `init` object (see [Initializing the transfer environment](#initializing-the-transfer-environment)). See also: [advanced usage](#advanced-usage). `#include "urban++.hpp"` will be omitted in further examples.

## Advanced usage

This library can get you all the info about the current word's search results on Urban Dictionary. A list of available get functions is provided in [list of member functions](#list-of-getters).

### Initializing the transfer environment

Before you start the search, you must create an object of the `Initializer` class (introduced in v1.3.0) in `static` storage. This object will [setup the libcurl transfer environment](https://curl.se/libcurl/c/curl_global_init.html) and will clean up the required memory at the end of the program. **There must be only one object of the Initializer class in your entire program.**

Example:

```cpp
...
int main() {
    static nm::Initializer init; // First thing to do
    // rest of the code
}
```

### Initializing objects

To initialize an object,

```cpp
nm::Urban objectName;
```

After initializing the object, you need to set the search term using:

```cpp
objectName.setSearchTerm(word) // word is either an std::string or a char *
```

As an example, `objectname.setSearchTerm("hello")` sets the search term to hello.

After setting the search term, the results must be fetched using the `fetch()` member function before you can access any results, so for our `objectName` it will be `objectName.fetch()`. Now you can finally access the info available about the search results using the [get functions](#list-of-getters). **The return value of both the `fetch()` and [`fetchRandom()`](#fetching-random-words) functions is [`CURLcode`](https://curl.se/libcurl/c/libcurl-errors.html).** Please have a look at the [error handling](#error-handling-and-timeout) section.

### Fetching random words

You can fetch the search results of random words using the `fetchRandom()` member function. Note that fetching random words does not require a search term to be already set, but if one is already set, the search term will be left unchanged after `fetchRandom()` has done its job. After running `objectName.fetchRandom()`, you'll use the [getters](#list-of-getters) just like you would after running `fetch()`.

### List of getters

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

### Raw JSON and number of definitions

You can also get the raw JSON response using the `rawJSON()` function. The return type of `rawJSON()` is `nlohmann::json`. Using this is completely optional and upto you.
In a terminal or open that URL with your search term in a browser to have a look at the JSON response.

To get the length of the list of definitions, you can use the `sizeOfJSON()` function which returns an `int` with the number of definitions in the list of definitions.

```cpp
...
objectName.setSearchTerm("hello");
objectName.fetch();
std::cout << "There are " << objectName.sizeOfJSON() << " definitions available for the word \"hello\"" << std::endl;
...
```

The above example prints the number of definitions available for the word "hello".

### Error handling and timeout

The return value of both the `fetch()` and `fetchRandom()` functions is [`CURLcode`](https://curl.se/libcurl/c/libcurl-errors.html), which you can utilize to print out any errors occured while fetching the results. An example is given below:

```cpp
#include <iostream>

#include <curl/curl.h>
#include "urban++.hpp"

int main() {
    static nm::Initializer init; // Initialize transfer environment
    nm::Urban objectName; // Initialize object
    objectName.setSearchTerm("hello"); // Set search term
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

The above program makes use of [`curl_easy_strerror()`](https://curl.se/libcurl/c/curl_easy_strerror.html) to convert the error code from a `CURLcode` object (note that the return value of `objectName.fetch()` is assigned to `err_code`) to a string, which is then printed.

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

There is a connection timeout duration of 30 seconds set by the class.

# Dependencies

This library depends on:

- The [curl](https://github.com/curl/curl/tree/master/include/curl) library by [Daniel Stenberg](https://github.com/bagder)

- The [JSON for Modern C++](https://github.com/nlohmann/json) library by [Niels Lohmann](https://github.com/nlohmann)

# Contribute to Urban++

Got a feature suggestion? Don't understand something in the documentation and want it to be more clear? Don't hesitate, open an issue, or even better, make a pull request!
