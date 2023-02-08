#include <string>
#include <iostream>

#include <re2/re2.h>
#include <re2/stringpiece.h>

int main() {
  std::string w;
  std::string text = "we'd  see   you say 世界你好真实好的很啊";
  re2::StringPiece input(text);

  RE2 re("('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+)");
  assert(re.ok());  // compiled; if not, see re.error();

  std::string var;
  int value;
  while (RE2::FindAndConsume(&input, re, &w)) {
    std::cout << "token=\"" << w << "\"" << std::endl;
  }
}
