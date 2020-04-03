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
#include <thread>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;

class crawler{
public:
    std::list<std::string> getLinks(std::string&);

    void nesting(std::list<std::string>&);

    std::string getHttp(std::smatch&);

    std::string getHttps(std::smatch&);

    std::list<std::string> makeLinksList(std::string&);

    void startThreadsDownload(std::list<std::string> &);

    void downloader();

    void setCounter(unsigned i){
        _nestingCounter+=i;
    }
    void setLinkList(std::list<std::string> list){
        listElementADD(_linkList, list);
    }
    unsigned getNestingVar(){
        return _nestingVar;
    }

    unsigned getNestingCounter(){
        return _nestingCounter;
    }

    void listElementADD(std::list<std::string>&, std::list<std::string>&);

//private:
unsigned _nestingVar = 1;
std::atomic_uint _nestingCounter = 0;
unsigned _threadsCount = 0;
std::size_t _threadCountDownload = std::thread::hardware_concurrency();
std::list<std::string> _arrayList;
std::list<std::string> _linkList;
};
#endif // INCLUDE_HEADER_HPP_

