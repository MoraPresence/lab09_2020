// Copyright 2018 Your Name <your_email>

#ifndef INCLUDE_HEADER_HPP_
#define INCLUDE_HEADER_HPP_
#include <list>
#include <mutex>
#include <gumbo.h>
#include <boost/beast.hpp>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include "root_certificates.hpp"
#include <queue>
#include <regex>
using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;

class crawler{
public:
    std::list<std::string> getLinks(std::string&);

    std::list<std::string> uniqueURL(std::list<std::string>&);

    void nesting(std::list<std::string>&);

    std::string getHTML(std::smatch&);

    std::list<std::string> makeLinksList(std::string&);

    void setCounter(unsigned i){
        _nestingCounter+=i;
    }
    unsigned getNestingVar(){
        return _nestingVar;
    }

    unsigned getNestingCounter(){
        return _nestingCounter;
    }

private:
unsigned _nestingVar = 0;
unsigned _nestingCounter = 0;
std::list<std::string> _arrayList;
};
#endif // INCLUDE_HEADER_HPP_

