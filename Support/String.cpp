//
// Created by croemheld on 01.01.2023.
//

#include <string>

namespace icarus {

bool startsWith(std::string_view Str, std::string_view Pre) {
  return Str.substr(0, Pre.size()) == Pre;
}

bool endsWith(std::string_view Str, std::string_view Suf) {
  return Str.size() >= Suf.size() && 0 == Str.compare(Str.size() - Suf.size(), Suf.size(), Suf);
}

}