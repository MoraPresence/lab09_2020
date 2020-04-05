// Copyright 2018 Your Name <your_email>

#include <header.hpp>

int main() {
    std::string adrHTML;
    std::size_t nestingVar;
    std::size_t threadDownload;
    std::size_t threadParser;
    std::string pathToFile;

    std::cout << "Link HTML-page: ";
    std::cin >> adrHTML;

    std::cout << "Nesting lvl: ";
    std::cin >> nestingVar;

    std::cout << "Threads for download: ";
    std::cin >> threadDownload;

    std::cout << "Threads for parsing: ";
    std::cin >> threadParser;

    std::cout << "Path to file: ";
    std::cin >> pathToFile;

    crawler crawler(nestingVar, threadDownload, threadParser);
    std::list<std::string> tmp;
    tmp.push_back(adrHTML);
    crawler.nesting(tmp);
    crawler.writeToFile(std::move(pathToFile));
    return 0;
}

void crawler::nesting(std::list<std::string> currentList) {
    std::list<std::string> tmp;
    std::vector<std::thread> threads;
    threads.reserve(_threadCountDownload);
    for (size_t i = 0; i < _threadCountDownload; ++i) {
        threads.emplace_back(std::thread
        (&crawler::downloader, this, &currentList, &tmp));
    }
    for (auto &th : threads) {
        th.join();
    }
    std::cout << "Level: " << getNestingCounter()
         << " links ready" << std::endl;
    setCounter(1);
    std::list<std::string> linksToNesting = makeLinksList(tmp);
    if (getNestingCounter() <= getNestingVar()) {
        nesting(linksToNesting);
    }
}

void crawler::downloader(std::list<std::string> *currentList,
 std::list<std::string> *tmp) {
    while (!currentList->empty()) {
        _mutex.lock();
        if (currentList->empty()) {
            _mutex.unlock();
            continue;
        }
        tmp->push_back(getHTML(currentList->front()));
        currentList->pop_front();
        _mutex.unlock();
    }
}

std::string crawler::getHTML(std::string &sUri) {
    std::regex rUri{"^(?:(https?)://)?([^/]+)(/.*)?"};
    std::string sBody;
    std::smatch mr;
    if (std::regex_match(sUri, mr, rUri)) {
        for (auto const &sm : mr) {
            std::cout << sm << std::endl;
        }
    } else {
        std::cerr << "std::regex_match failed: " + sUri << "\n\n";
    }
    if (mr[1].str() == "https") // работает пока только с https
    {
        sBody = getHttps(mr);

    } else if (mr[1].str() == "http") {
        sBody = getHttp(mr);
    }
    return sBody;
}

std::string crawler::getHttp(std::smatch &mr) {
    std::string const host = mr[2];
    std::string const port = "80"; // https - 443, http - 80
    std::string const target = (mr[3].length() == 0 ? "/" : mr[3].str());
    int version = 10; // или 10 для http 1.0

    boost::asio::io_context ioc;
    tcp::resolver resolver{ioc};
    boost::beast::tcp_stream stream{ioc};
    auto const results = resolver.resolve(host, port);
    stream.connect(results);
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    http::write(stream, req);
    boost::beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    http::read(stream, buffer, res);
    return boost::beast::buffers_to_string(res.body().data());
}

std::string crawler::getHttps(std::smatch &mr) {
    std::string const host = mr[2];
    std::string const port = "443"; // https - 443, http - 80
    std::string const target = (mr[3].length() == 0 ? "/" : mr[3].str());
    int version = 11; // или 10 для http 1.0
    try {
        boost::asio::io_context ioc;
        ssl::context ctx{ssl::context::sslv23_client};
        load_root_certificates(ctx);
        tcp::resolver resolver{ioc};
        ssl::stream<tcp::socket> stream{ioc, ctx};
        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()),
             boost::asio::error::get_ssl_category()};
            throw boost::system::system_error{ec};
        }
        auto const results = resolver.resolve(host, port);
        boost::asio::connect(stream.next_layer(), results.begin(),
        results.end());
        stream.handshake(ssl::stream_base::client);
        http::request<http::string_body> req{http::verb::get, target, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        http::write(stream, req);
        boost::beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        http::read(stream, buffer, res);
        return boost::beast::buffers_to_string(res.body().data());
    } catch (...) {
        std::cout << "error";
        return "error";
    }
}

std::list<std::string> crawler::makeLinksList
(std::list<std::string> &listBuf) {
    std::list<std::string> linksList;
    std::vector<std::thread> threads;
    threads.reserve(_threadCountParse);
    for (size_t i = 0; i < _threadCountParse; ++i) {
        threads.emplace_back(std::thread(&crawler::parser,
        this, &listBuf, &linksList));
    }
    for (auto &th : threads) {
        th.join();
    }
    unique(linksList);
    std::cout << "Level: " << getNestingCounter() - 1
              << " links on image ready" << std::endl;
    return linksList;
}

void crawler::parser(std::list<std::string> *listBuf,
std::list<std::string> *linksList) {
    while (!listBuf->empty()) {
        _mutex.lock();
        if (listBuf->empty()) {
            _mutex.unlock();
            continue;
        }
        std::string buf = listBuf->front();
        listBuf->pop_front();
        GumboOutput *output = gumbo_parse(buf.c_str());

        std::queue<GumboNode *> qn;
        qn.push(output->root);

        while (!qn.empty()) {
            GumboNode *node = qn.front();
            qn.pop();
            if (GUMBO_NODE_ELEMENT == node->type) {
                GumboAttribute *href = nullptr;
                if (node->v.element.tag == GUMBO_TAG_A &&
                    (href = gumbo_get_attribute
                    (&node->v.element.attributes, "href"))) {
                    if (std::string(href->value).find("http") == 0)
                        linksList->push_back(href->value);
                }
                if (node->v.element.tag == GUMBO_TAG_IMG &&
                    (href = gumbo_get_attribute
                    (&node->v.element.attributes, "src"))) {
                    if (std::string(href->value).find("http") == 0)
                        _arrayList.push_back(href->value);
                }

                GumboVector *children = &node->v.element.children;
                for (unsigned int i = 0; i < children->length; ++i) {
                    qn.push(static_cast<GumboNode *>(children->data[i]));
                }
            }
        }
        gumbo_destroy_output(&kGumboDefaultOptions, output);
        _mutex.unlock();
    }
}

void crawler::elementADD(std::list<std::string> &rx,
 std::list<std::string> &tx) {
    for (auto it = tx.begin(); it != tx.end(); ++it) {
        rx.push_back(*it);
    }
}

void crawler::elementADD(std::queue<std::shared_ptr<std::string>> &rx,
std::list<std::string> &tx) {
    for (auto it = tx.begin(); it != tx.end(); ++it) {
        rx.push(std::make_shared<std::string>(*it));
    }
}

void crawler::unique(std::list<std::string> &list) {
    list.sort();
    list.unique();
}

void crawler::writeToFile(std::string &&path) {
    unique(this->_arrayList);
    std::ofstream ofs{path};
    for (std::string i : this->_arrayList) {
        ofs << i << std::endl;
    }
    ofs.close();
}

