#include <codecvt>
#include <iostream>
#include <string>

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.to_bytes(str);
}

int main() {
  wchar_t c = L'益';
  std::cout << sizeof(c) << std::endl;

  std::string text = "Hi 益\n";

  std::wcout << utf8_to_wstring(text) << std::endl;
  std::cout << wstring_to_utf8(utf8_to_wstring(text)) << std::endl;

  std::cout << "sizeof(wchar_t)    : " << sizeof(wchar_t) << "\n";
  std::cout << "sizeof(text)    : " << text.size() << "\n";

  std::cout << "text(ordinals)  :";

  for (size_t i = 0; i < text.size(); ++i) {
    std::cout << " " << static_cast<uint32_t>(text[i]);
  }

  std::cout << "\n\n";
  return 0;
}
