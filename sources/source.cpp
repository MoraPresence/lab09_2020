// Copyright 2018 Your Name <your_email>

#include <header.hpp>
int main() {
    crawler cr;
    std::list<std::string> *uniqueList = new std::list<std::string>();
    std::list<std::string> *uniqueList2 = new std::list<std::string>();
    uniqueList->push_back("bla1");
    uniqueList->push_back("bla2");
    uniqueList->push_back("bla3");

    for (std::string i : *uniqueList) {
        std::cout << i << std::endl;
    }
    cr.listElementADD(*uniqueList2, *uniqueList);
    for (std::string i : *uniqueList2) {
        std::cout << i;
    }

    std::list<std::string> tmp;
    tmp.push_back("https://yandex.ru/");
    std::ofstream ofs{"/home/mora/Desktop/out.txt"}; // запись html-страницы в файл
    cr.nesting(tmp);
    cr._arrayList.sort();
    cr._arrayList.unique();
    for (std::string i : cr._arrayList) {
        ofs << i << std::endl;
    }
    ofs.close();
}

void crawler::nesting(std::list<std::string> &currentList) {
    std::list<std::string> tmp;
    for (auto it = currentList.begin(); it != currentList.end(); ++it) {
        tmp.merge(getLinks(*it));
    }
    setCounter(1);
    tmp.sort();
    tmp.unique();
    this->listElementADD(_arrayList, tmp);
    if (getNestingCounter() <= getNestingVar()) {
        nesting(tmp);
    }
}

std::list<std::string> crawler::getLinks(std::string &sUri) {
    std::regex rUri{"^(?:(https?)://)?([^/]+)(/.*)?"};
    std::string sBody;
    std::smatch mr;
    if (std::regex_match(sUri, mr, rUri)) {
        for (auto const &sm : mr) {
            //std::cout << sm << std::endl;
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


    return makeLinksList(sBody);

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
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            throw boost::system::system_error{ec};
        }
        auto const results = resolver.resolve(host, port);
        boost::asio::connect(stream.next_layer(), results.begin(), results.end());
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

std::list<std::string> crawler::makeLinksList(std::string &buf) {
    std::list<std::string> linksList;

    GumboOutput *output = gumbo_parse(buf.c_str());

    std::queue<GumboNode *> qn;
    qn.push(output->root);

    while (!qn.empty()) {
        GumboNode *node = qn.front();
        qn.pop();

        if (GUMBO_NODE_ELEMENT == node->type) {
            GumboAttribute *href = nullptr;
            if (node->v.element.tag == GUMBO_TAG_A &&
                (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
                if (std::string(href->value).find("http") == 0)
                    linksList.push_back(href->value);
            }

            GumboVector *children = &node->v.element.children;
            for (unsigned int i = 0; i < children->length; ++i) {
                qn.push(static_cast<GumboNode *>(children->data[i]));
            }
        }
    }
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    linksList.sort();
    linksList.unique();
    return linksList;
}

void crawler::listElementADD(std::list<std::string> &rx, std::list<std::string> &tx) {
    for (auto it = tx.begin(); it != tx.end(); ++it) {
        rx.push_back(*it);
    }
}

void crawler::startThreadsDownload(std::list<std::string> &currentList) {
    setLinkList(currentList);
    if (_threadCountDownload >= 1 && _threadCountDownload
                                     <= std::thread::hardware_concurrency()) {
        std::vector<std::thread> threads;
        threads.reserve(_threadCountDownload);
        for (size_t i = 0; i < _threadCountDownload; ++i) {
            threads.emplace_back();
        }
        for (auto &th : threads) {
            th.join();
        }
    } else {
        return;
    }

}

void crawler::downloader() {

}

