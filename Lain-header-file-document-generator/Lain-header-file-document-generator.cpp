#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

struct DocItem {
    std::string type = "UNKNOWN";
    std::string signature = "";
    std::string brief = "";
    std::string details = "";
    std::vector<std::pair<std::string, std::string>> params;
    std::vector<std::string> sees;
    std::vector<std::string> warnings;
};

std::string format_inline_elements(std::string text) {
    size_t pos = 0;
    while ((pos = text.find("\\a ", pos)) != std::string::npos) {
        size_t end_pos = text.find_first_of(" .,;\n", pos + 3);
        if (end_pos == std::string::npos) end_pos = text.length();
        std::string word = text.substr(pos + 3, end_pos - (pos + 3));
        text.replace(pos, end_pos - pos, "<i>" + word + "</i>");
        pos += 7 + word.length();
    }
    return text;
}

std::string trim_whitespace(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string trim_comment_line(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n*");
    if (std::string::npos == first) return "";
    size_t last = str.find_last_not_of(" \t\r\n*");
    return str.substr(first, (last - first + 1));
}

bool check_lain_signature(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) return false;

    std::string first_line;
    if (std::getline(file, first_line)) {
        return (trim_whitespace(first_line) == "// lain-was-here");
    }
    return false;
}

std::vector<DocItem> parse_header_file(const std::string& file_path) {
    std::vector<DocItem> items;
    std::ifstream file(file_path);
    if (!file.is_open()) return items;

    std::string line;
    bool inside_comment = false;
    DocItem current_item;
    bool has_explicit_signature = false;

    std::getline(file, line);

    while (std::getline(file, line)) {
        std::string trimmed = trim_comment_line(line);

        if (line.find("/**") != std::string::npos || line.find("/*!") != std::string::npos) {
            inside_comment = true;
            current_item = DocItem();
            has_explicit_signature = false;
            continue;
        }

        if (inside_comment && line.find("*/") != std::string::npos) {
            inside_comment = false;

            if (!has_explicit_signature) {
                std::string next_line;
                if (std::getline(file, next_line)) {
                    std::string clean_code = trim_whitespace(next_line);
                    if (!clean_code.empty() && clean_code.back() == ';') {
                        clean_code.pop_back();
                    }
                    if (!clean_code.empty()) {
                        current_item.signature = clean_code;
                        if (clean_code.find("typedef") != std::string::npos) current_item.type = "TYPEDEF";
                        else if (clean_code.find('(') != std::string::npos) current_item.type = "FUNCTION";
                        else current_item.type = "VARIABLE";
                    }
                }
            }
            if (!current_item.signature.empty() || current_item.type == "FILE") {
                items.push_back(current_item);
            }
            continue;
        }

        if (inside_comment) {
            if (trimmed.find("\\brief") != std::string::npos || trimmed.find("@brief") != std::string::npos) {
                size_t pos = trimmed.find("brief");
                current_item.brief = format_inline_elements(trimmed.substr(pos + 5));
            }
            else if (trimmed.find("\\param") != std::string::npos || trimmed.find("@param") != std::string::npos) {
                size_t pos = trimmed.find("param");
                std::string p_content = trimmed.substr(pos + 5);
                std::stringstream ss(p_content);
                std::string p_name, p_desc;
                ss >> p_name;
                std::getline(ss, p_desc);
                current_item.params.push_back({ p_name, format_inline_elements(trim_comment_line(p_desc)) });
            }
            else if (trimmed.find("\\warning") != std::string::npos || trimmed.find("@warning") != std::string::npos) {
                size_t pos = trimmed.find("warning");
                current_item.warnings.push_back(format_inline_elements(trimmed.substr(pos + 7)));
            }
            else if (trimmed.find("\\see") != std::string::npos || trimmed.find("@see") != std::string::npos) {
                size_t pos = trimmed.find("see");
                current_item.sees.push_back(trimmed.substr(pos + 3));
            }
            else if (trimmed.find("\\file") != std::string::npos) {
                current_item.type = "FILE";
                current_item.signature = trimmed.substr(trimmed.find("\\file") + 5);
                has_explicit_signature = true;
            }
            else if (trimmed.find("\\fn") != std::string::npos) {
                current_item.type = "FUNCTION";
                current_item.signature = trimmed.substr(trimmed.find("\\fn") + 3);
                has_explicit_signature = true;
            }
            else if (trimmed.find("\\def") != std::string::npos) {
                current_item.type = "MACRO";
                current_item.signature = trimmed.substr(trimmed.find("\\def") + 4);
                has_explicit_signature = true;
            }
            else if (trimmed.find("\\var") != std::string::npos) {
                current_item.signature = trimmed.substr(trimmed.find("\\var") + 4);
                if (current_item.signature.find("typedef") != std::string::npos) current_item.type = "TYPEDEF";
                else current_item.type = "VARIABLE";
                has_explicit_signature = true;
            }
            else {
                if (!trimmed.empty()) {
                    if (!current_item.details.empty()) current_item.details += " ";
                    current_item.details += format_inline_elements(trimmed);
                }
            }
        }
    }
    return items;
}

std::string generate_live_html_and_save() {
    std::string html;
    html += "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n";
    html += "<title>Lain-header-file-document-generator</title>\n";

    html += "<style>\n";
    html += "body { font-family: 'Segoe UI', sans-serif; margin: 40px; background: #f8f9fa; color: #333; }\n";
    html += ".file-section { background: #2b3e50; color: white; padding: 15px; margin-top: 40px; border-radius: 6px; }\n";
    html += ".card { background: white; padding: 20px; margin-top: 15px; border-radius: 6px; box-shadow: 0 2px 5px rgba(0,0,0,0.05); border-left: 5px solid #007acc; }\n";
    html += ".badge { display: inline-block; padding: 4px 8px; font-size: 0.8em; font-weight: bold; border-radius: 4px; color: white; margin-right: 10px; }\n";
    html += ".bg-func { background: #007acc; } .bg-macro { background: #e83e8c; } .bg-var { background: #28a745; } .bg-file { background: #6f42c1; } .bg-type { background: #fd7e14; }\n";
    html += "code { font-family: monospace; font-size: 1.1em; background: #f1f1f1; padding: 2px 6px; border-radius: 4px; }\n";
    html += ".param-table { width: 100%; border-collapse: collapse; margin-top: 10px; }\n";
    html += ".param-table td { padding: 8px; border: 1px solid #ddd; }\n";
    html += ".param-table tr:nth-child(even){ background-color: #f9f9f9; }\n";
    html += ".warning-box { background: #fff3cd; border-left: 5px solid #ffc107; color: #856404; padding: 12px; margin-top: 10px; border-radius: 4px; }\n";
    html += ".see-box { font-size: 0.9em; color: #555; margin-top: 10px; background: #e2e3e5; padding: 8px; border-radius: 4px; }\n";
    html += "</style>\n";

    html += "<script>\n";
    html += "  let currentVersion = null;\n";
    html += "  setInterval(() => {\n";
    html += "    fetch('/version').then(res => res.text()).then(version => {\n";
    html += "        if (currentVersion === null) currentVersion = version;\n";
    html += "        else if (currentVersion !== version) window.location.reload();\n";
    html += "    });\n";
    html += "  }, 1000);\n";
    html += "</script>\n</head>\n<body>\n";
    html += "<h1>Lain-header-file-document-generator</h1><hr>\n";

    bool valid_files_found = false;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(".")) {
        if (entry.is_regular_file() && entry.path().extension() == ".h") {
            std::string file_path = entry.path().string();

            if (!check_lain_signature(file_path)) {
                continue;
            }

            auto doc_items = parse_header_file(file_path);
            if (doc_items.empty()) continue;

            valid_files_found = true;
            html += "<div class=\"file-section\"><h2>Dosya: " + file_path + "</h2></div>\n";

            for (const auto& item : doc_items) {
                std::string badge_cls = "bg-func";
                if (item.type == "MACRO") badge_cls = "bg-macro";
                else if (item.type == "VARIABLE") badge_cls = "bg-var";
                else if (item.type == "FILE") badge_cls = "bg-file";
                else if (item.type == "TYPEDEF") badge_cls = "bg-type";

                html += "<div class=\"card\">\n";
                html += "  <h3><span class=\"badge " + badge_cls + "\">" + item.type + "</span><code>" + item.signature + "</code></h3>\n";

                if (!item.brief.empty()) {
                    html += "  <p><strong>Ozet:</strong> " + item.brief + "</p>\n";
                }
                if (!item.details.empty()) {
                    html += "  <p><strong>Detaylar:</strong> " + item.details + "</p>\n";
                }

                if (!item.params.empty()) {
                    html += "  <h4>Parametreler:</h4>\n";
                    html += "  <table class=\"param-table\">\n";
                    for (const auto& p : item.params) {
                        html += "    <tr><td style=\"font-weight:bold; width:15%;\">" + p.first + "</td><td>" + p.second + "</td></tr>\n";
                    }
                    html += "  </table>\n";
                }

                for (const auto& w : item.warnings) {
                    html += "  <div class=\"warning-box\"><strong>Uyari:</strong> " + w + "</div>\n";
                }

                if (!item.sees.empty()) {
                    html += "  <div class=\"see-box\"><strong>Ayrica Bakiniz:</strong> ";
                    for (size_t i = 0; i < item.sees.size(); ++i) {
                        html += "<code>" + item.sees[i] + "</code>" + (i < item.sees.size() - 1 ? ", " : "");
                    }
                    html += "</div>\n";
                }

                html += "</div>\n";
            }
        }
    }

    if (!valid_files_found) {
        html += "<p>Tarama bolgesinde en ust satirinda '// lain-was-here' imzasi barindiran gecerli bir .h dosyasi bulunamadi.</p>";
    }

    html += "</body>\n</html>\n";

    std::ofstream out_file("index.html");
    if (out_file.is_open()) {
        out_file << html;
        out_file.close();
    }

    return html;
}

std::string get_project_version() {
    std::string state = "";
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(".")) {
            if (entry.is_regular_file() && entry.path().extension() == ".h") {
                std::string path_str = entry.path().string();
                if (check_lain_signature(path_str)) {
                    auto lwt = std::filesystem::last_write_time(entry);
                    state += path_str + std::to_string(lwt.time_since_epoch().count());
                }
            }
        }
    }
    catch (...) {}
    return state.empty() ? "0" : state;
}

void handle_session(tcp::socket socket) {
    beast::error_code ec;
    beast::flat_buffer buffer;
    http::request<http::string_body> req;
    http::read(socket, buffer, req, ec);
    if (ec) return;

    http::response<http::string_body> res;
    res.version(req.version());
    res.keep_alive(req.keep_alive());

    if (req.target() == "/version") {
        res.result(http::status::ok);
        res.set(http::field::content_type, "text/plain");
        res.body() = get_project_version();
    }
    else {
        res.result(http::status::ok);
        res.set(http::field::content_type, "text/html; charset=utf-8");
        res.body() = generate_live_html_and_save();
    }

    res.prepare_payload();
    http::write(socket, res, ec);
    if (ec) return;
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

int main() {
    try {
        auto const address = net::ip::make_address("127.0.0.1");
        unsigned short port = 8080;
        net::io_context ioc{ 1 };
        tcp::acceptor acceptor{ ioc, {address, port} };

        std::cout << "========================================================\n";
        std::cout << " Sunucu Baslatildi: http://127.0.0.1:" << port << "\n";
        std::cout << " Calisilan Program: Lain-header-file-document-generator\n";
        std::cout << " Not: Sadece en ustunde '// lain-was-here' olan dosyalar taranir.\n";
        std::cout << " Yerel dosya 'index.html' eszamanli guncellenmektedir.\n";
        std::cout << "========================================================\n";

        while (true) {
            tcp::socket socket{ ioc };
            acceptor.accept(socket);
            handle_session(std::move(socket));
        }
    }
    catch (std::exception const& e) {
        std::cerr << "Hata: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}