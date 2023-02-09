// clang++ -std=c++20 ~/w/gpt2cpp/t.cc -I ~/w/re2/b/install/include -L ~/w/re2/b/install/lib -lre2 -o /tmp/t && /tmp/t
#include <codecvt>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <set>

#include <re2/re2.h>
#include <re2/stringpiece.h>


void test_re2() {
  std::string w;
  std::string text = "we'd annoyingly 顽皮";
  re2::StringPiece input(text);

  RE2 re("('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+\\(?!\\S\\)|\\s+)");
  assert(re.ok());  // compiled; if not, see re.error();

  std::string var;
  int value;
  while (RE2::FindAndConsume(&input, re, &w)) {
    std::cout << "token=\"" << w << "\"" << std::endl;
  }
}

std::wstring utf8_to_wstring(const std::string &str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.from_bytes(str);
}

std::string wstring_to_utf8(const std::wstring &str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.to_bytes(str);
}

std::string utf8(wchar_t c) {
  std::wstring w(1, c);
  return wstring_to_utf8(w);
}

void bytes_to_unicode(std::unordered_map<uint8_t, wchar_t> *b2u,
		      std::unordered_map<wchar_t, uint8_t> *u2b) {
  auto _insert_range = [=](int start, int end) {
    for (int c = start; c <= end; c++) {
      b2u->insert({uint8_t(c), wchar_t(c)});
    }
  };

  b2u->clear();
  _insert_range(L'!', L'~');
  _insert_range(L'¡', L'¬');
  _insert_range(L'®', L'ÿ');

  int n = 0;
  for (int b = 0; b < 256; b++) {
    if (b2u->find(uint8_t(b)) == b2u->end()) {
      b2u->insert({uint8_t(b), wchar_t(256 + n)});
      n++;
    }
  }

  if (u2b != NULL) {
    u2b->clear();
    for (auto e : (*b2u)) {
      u2b->insert({e.second, e.first});
    }
  }
}

void test_bytes_to_unicode() {
  std::unordered_map<uint8_t, wchar_t> b2u;
  std::unordered_map<wchar_t, uint8_t> u2b;
  bytes_to_unicode(&b2u, &u2b);
  assert(b2u.size() == 256);
  assert(b2u[0] == 0x100);
  assert(u2b.size() == 256);
}

// Given a token as a UTF8 string, encode each byte into an wchar_t
void byte_encode_token(const std::string& token,
		       std::unordered_map<uint8_t, wchar_t>& b2u,
		       std::wstring* result) {
  result->resize(0);
  for (char c : token) {
    wchar_t wc = b2u[uint8_t(c)];
    result->push_back(wc);
  }
}

void test_byte_encode_token() {
  std::unordered_map<uint8_t, wchar_t> b2u;
  bytes_to_unicode(&b2u, NULL);

  std::string s(" very");
  std::wstring b;
  byte_encode_token(s, b2u, &b);
  assert("Ġvery" == wstring_to_utf8(b));
}

// hash_pair_wstring is used in BPERanks to make a pair of wstrings
// hashable, so the pair can be used as the key to unordered_map.
struct hash_pair_wstring {
  size_t operator()(const std::pair<std::wstring, std::wstring>& p) const {
    auto hash1 = std::hash<std::wstring>{}(p.first);
    auto hash2 = std::hash<std::wstring>{}(p.second);
    // If hash1 == hash2, their XOR is zero.
    return (hash1 != hash2) ? hash1 ^ hash2 : hash1;
  }
};

// BPERanks maps each merge rule, which is a pair of wstrings, to its
// rank.  This mapping allows quick lookup for the optimal merge rule.
using BPERanks = std::unordered_map<std::pair<std::wstring,
					      std::wstring>,
				    int,
				    hash_pair_wstring>;

void load_merge_rules(std::istream& ins, BPERanks* bpe_ranks) {
  bpe_ranks->clear();

  std::string line;
  int n = 0;
  while (std::getline(ins, line)) {
    if (n > 0) { // Skip the version comment.
      int d = line.find(" "); // merges.txt file use ASCII space
      bpe_ranks->insert({{utf8_to_wstring(line.substr(0, d)),
	    utf8_to_wstring(line.substr(d+1))},
	  n-1});
    }
    n++;
  }
}

void test_load_merge_rules() {
  BPERanks bpe_ranks;
  std::fstream merges("/tmp/merges.txt", std::ios::in);
  load_merge_rules(merges, &bpe_ranks);
  assert(bpe_ranks.size() == 50000);

  auto iter = bpe_ranks.find({utf8_to_wstring("Ġg"), utf8_to_wstring("azed")});
  assert(iter != bpe_ranks.end());
  assert(iter->second = 49999);
}

void get_pairs(const std::wstring& word,
	       std::vector<std::pair<std::wstring, std::wstring> >* pairs) {
  pairs->clear();

  if (word.size() < 2)
    return;

  wchar_t previous = word[0];
  for (int i = 1; i < word.size(); i++) {
    pairs->push_back({std::wstring(1, previous), std::wstring(1, word[i])});
    previous = word[i];
  }
}

void test_get_pairs() {
  std::vector<std::pair<std::wstring, std::wstring> > pairs;
  get_pairs(utf8_to_wstring("very"), &pairs);
  assert(pairs.size() == 3);
  assert(wstring_to_utf8(pairs[1].first) == "e");
  assert(wstring_to_utf8(pairs[1].second) == "r");
}

void bpe(const std::wstring& token,
	 BPERanks & bpe_ranks,
	 std::vector<std::wstring> * result) {
  std::set<int> merged; // records indices in pairs that were merged.
  auto _left = [](int i, std::set<int>& merged) {
    for (int j = i-1; j >= -1; j--) {
      if (merged.find(j) == merged.end())
	return j;
    }
    return -1;
  };
  auto _right = [](int i, int cap, std::set<int>& merged) {
    for (int j = i+1; j< cap; j++) {
      if (merged.find(j) == merged.end())
	return j;
    }
    return cap;
  };
  
  std::vector<std::pair<std::wstring, std::wstring> > pairs;
  get_pairs(token, &pairs);
  
  while (true) {
    int min_score = INT_MAX;
    int to_merge = -1; // indices into pairs.

    for (int i = 0; i < pairs.size(); ++i) {
      if (merged.find(i) == merged.end()) { // pair i is not merged.
	auto iter = bpe_ranks.find(pairs[i]);
	int score = iter != bpe_ranks.end() ? iter->second : INT_MAX;
	if (score < min_score) {
	  min_score = score;
	  to_merge = i;
	}
      }
    }

    if (to_merge == -1)
      break;

    merged.insert(to_merge);
    std::wstring merge_into = pairs[to_merge].first + pairs[to_merge].second;

    int l = _left(to_merge, merged);
    if (l >= 0)
      pairs[l].second = merge_into;
    int r = _right(to_merge, pairs.size(), merged);
    if (r < pairs.size())
      pairs[r].first = merge_into;
  } // end while (true)

  if (merged.size() == pairs.size())
    result->push_back(token);
  else {
    for (int i = 0; i < pairs.size(); ++i) {
      if (merged.find(i) == merged.end()) {
	if (_left(i, merged) < 0)
	  result->push_back(pairs[i].first);
	result->push_back(pairs[i].second);
      }
    }
  }
}

void test_bpe() {
  BPERanks bpe_ranks;
  std::fstream merges("/tmp/merges.txt", std::ios::in);
  load_merge_rules(merges, &bpe_ranks);
  assert(bpe_ranks.size() == 50000);

  std::vector<std::wstring> result;
  bpe(utf8_to_wstring("annoyingly"), bpe_ranks, &result);
  assert(result == std::vector<std::wstring>({L"ann", L"oy", L"ingly"}));

  result.clear();
  bpe(utf8_to_wstring("very"), bpe_ranks, &result);
  assert(result == std::vector<std::wstring>({L"very"}));
}

int main() {
  // test_bytes_to_unicode();
  // test_re2();
  // test_load_merge_rules();
  // test_byte_encode_token();
  // test_get_pairs();
  test_bpe();
  return 0;
}
