// Copyright 2018 Your Name <your_email>

#include <header.hpp>
int main() {
    std::list<std::string> *uniqueList = new std::list<std::string>();
    std::list<std::string> *uniqueList2 = new std::list<std::string>();
    uniqueList->push_back("bla");
    uniqueList->push_back("bla");
    uniqueList->push_back("bla");

    for (std::string i : *uniqueList) {
        std::cout << i << std::endl;
    }
    uniqueList->unique();
    for (std::string i : *uniqueList) {
        std::cout << i;
    }
    uniqueList2->merge(*uniqueList);
    for (std::string i : *uniqueList2) {
        std::cout << i;
    }
    crawler cr;
    std::string a = "a";
    std::ofstream ofs{ "/home/mora/Desktop/out.txt" }; // запись html-страницы в файл
    std::list<std::string> uniqueList3 = cr.getLinks(a);
    for (std::string i : uniqueList3) {
         ofs << i << std::endl;
    }
    ofs.close();
}

void crawler::nesting(std::list<std::string> &) {
    auto *tmp = new std::list<std::string>();
    for (auto it = _arrayList.begin(); it != _arrayList.end(); ++it) {
        tmp->merge(getLinks(*it));
    }
    setCounter(1);
    if (getNestingCounter() < getNestingVar()) {
        nesting(*tmp);
    }
}

std::list<std::string> crawler::getLinks(std::string &) {
    std::string sUri{"https://go.mail.ru/search?q=бла+&fm=1"}; // адрес исходной страницы
    std::regex rUri{"^(?:(https?)://)?([^/]+)(/.*)?"};

    std::smatch mr;
    if (std::regex_match(sUri, mr, rUri)) {
        for (auto const &sm : mr) {
            //std::cout << sm << std::endl;
        }
    } else {
        std::cerr << "std::regex_match failed: " + sUri << "\n\n";
    }
    if (mr[1].str() != "https") // работает пока только с https
    {
        std::cerr << "mr[1].str() != \"https\"" << "\n\n";

    }
    std::string sBody = getHTML(mr);

    return makeLinksList(sBody);

}

std::string crawler::getHTML(std::smatch &mr) {
    std::string const host = mr[2];
    std::string const port = "443"; // https - 443, http - 80
    std::string const target = (mr[3].length() == 0 ? "/" : mr[3].str());
    int version = 11; // или 10 для http 1.0

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
}

std::list<std::string> crawler::makeLinksList(std::string &buf) {
    std::list<std::string> *linksList = new std::list<std::string>;
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
                linksList->push_back(href->value);
            }

            GumboVector *children = &node->v.element.children;
            for (unsigned int i = 0; i < children->length; ++i) {
                qn.push(static_cast<GumboNode *>(children->data[i]));
            }
        }
    }

    gumbo_destroy_output(&kGumboDefaultOptions, output);
    linksList->unique();
    return *linksList;
}

