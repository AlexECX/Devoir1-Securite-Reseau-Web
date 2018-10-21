#include <string>

bool authenticate(std::string msg, std::string name, std::string mac_key);

std::string generateKey(unsigned size);

std::string encrypt(const std::string&, const std::string&);

std::string decrypt(const std::string&, const std::string&);

std::string generateMac(const std::string& message, const std::string& key);

bool verifyMAC(const std::string&, const std::string&);

std::string extractMsg(const std::string&);
