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
#include <string>
#include <fstream>
#include "root_certificates.hpp"
#include <queue>
#include <regex>
#include <thread>


using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace http = boost::beast::http;

class crawler {
public:
    crawler() :
            _threadCountDownload(std::thread::hardware_concurrency()),
            _threadCountParse(std::thread::hardware_concurrency()) {}

    explicit crawler(std::size_t nestingVar) :
            _nestingVar(nestingVar),
            _threadCountDownload(std::thread::hardware_concurrency()),
            _threadCountParse(std::thread::hardware_concurrency()) {}

    explicit crawler(std::size_t nestingVar,
    std::size_t download, std::size_t parser) :
            _nestingVar(nestingVar),
            _threadCountDownload(download),
            _threadCountParse(parser) {}

    void nesting(std::list<std::string>);

    void writeToFile(std::string &&);

private:
    std::string getHTML(std::string &);

    std::string getHttp(std::smatch &);

    std::string getHttps(std::smatch &);

    std::list<std::string> makeLinksList(std::list<std::string> &);

    void unique(std::list<std::string> &);

    void downloader(std::list<std::string> *, std::list<std::string> *);

    void parser(std::list<std::string> *, std::list<std::string> *);

    void setCounter(unsigned i) {
        _nestingCounter += i;
    }

    unsigned getNestingVar() {
        return _nestingVar;
    }

    unsigned getNestingCounter() {
        return _nestingCounter;
    }

    void elementADD(std::list<std::string> &, std::list<std::string> &);

    void elementADD(std::queue<std::shared_ptr<std::string>> &,
     std::list<std::string> &);

    std::size_t _nestingVar = 1;
    std::atomic_uint _nestingCounter = 0;
    std::size_t _threadCountDownload;
    std::size_t _threadCountParse;
    std::list<std::string> _arrayList;
    std::mutex _mutex;
};
#endif // INCLUDE_HEADER_HPP_

