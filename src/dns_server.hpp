#ifndef DNS_SERVER_HPP
#define DNS_SERVER_HPP

#include <string>
#include <unordered_set>

bool start_dns_server(const std::string &listen_addr, int port,
                      const std::unordered_set<std::string> &blocked,
                      bool verbose);

#endif // DNS_SERVER_HPP
