#include "bpe.h"

std::wstring utf8_to_wstring(const std::string& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.from_bytes(str);
}

std::string wstring_to_utf8(const std::wstring& str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
  return myconv.to_bytes(str);
}

std::string utf8(wchar_t c) {
  std::wstring w(1, c);
  return wstring_to_utf8(w);
}

void bytes_to_unicode(std::unordered_map<uint8_t, wchar_t>* b2u,
                      std::unordered_map<wchar_t, uint8_t>* u2b) {
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

void load_merge_rules(std::istream& ins, BPERanks* bpe_ranks) {
  bpe_ranks->clear();

  std::string line;
  int n = 0;
  while (std::getline(ins, line)) {
    if (n > 0) {               // Skip the version comment.
      int d = line.find(" ");  // merges.txt file use ASCII space
      bpe_ranks->insert({{utf8_to_wstring(line.substr(0, d)),
                          utf8_to_wstring(line.substr(d + 1))},
                         n - 1});
    }
    n++;
  }
}

void get_pairs(const std::wstring& word,
               std::vector<std::pair<std::wstring, std::wstring>>* pairs) {
  pairs->clear();

  if (word.size() < 2) return;

  wchar_t previous = word[0];
  for (int i = 1; i < word.size(); i++) {
    pairs->push_back({std::wstring(1, previous), std::wstring(1, word[i])});
    previous = word[i];
  }
}

void bpe(const std::wstring& token, BPERanks& bpe_ranks,
         std::vector<std::wstring>* result) {
  std::set<int> merged;  // records indices in pairs that were merged.
  auto _left = [](int i, std::set<int>& merged) {
    for (int j = i - 1; j >= -1; j--) {
      if (merged.find(j) == merged.end()) return j;
    }
    return -1;
  };
  auto _right = [](int i, int cap, std::set<int>& merged) {
    for (int j = i + 1; j < cap; j++) {
      if (merged.find(j) == merged.end()) return j;
    }
    return cap;
  };

  std::vector<std::pair<std::wstring, std::wstring>> pairs;
  get_pairs(token, &pairs);

  while (true) {
    int min_score = INT_MAX;
    int to_merge = -1;  // indices into pairs.

    for (int i = 0; i < pairs.size(); ++i) {
      if (merged.find(i) == merged.end()) {  // pair i is not merged.
        auto iter = bpe_ranks.find(pairs[i]);
        int score = iter != bpe_ranks.end() ? iter->second : INT_MAX;
        if (score < min_score) {
          min_score = score;
          to_merge = i;
        }
      }
    }

    if (to_merge == -1) break;

    merged.insert(to_merge);
    std::wstring merge_into = pairs[to_merge].first + pairs[to_merge].second;

    int l = _left(to_merge, merged);
    if (l >= 0) pairs[l].second = merge_into;
    int r = _right(to_merge, pairs.size(), merged);
    if (r < pairs.size()) pairs[r].first = merge_into;
  }  // end while (true)

  if (merged.size() == pairs.size())
    result->push_back(token);
  else {
    for (int i = 0; i < pairs.size(); ++i) {
      if (merged.find(i) == merged.end()) {
        if (_left(i, merged) < 0) result->push_back(pairs[i].first);
        result->push_back(pairs[i].second);
      }
    }
  }
}

void _tokenize(const std::string& text, RE2& re, BPERanks& bpe_ranks,
               std::unordered_map<uint8_t, wchar_t>& b2u,
               std::vector<std::string>* result) {
  re2::StringPiece input(text);
  std::string token;
  while (RE2::FindAndConsume(&input, re, &token)) {
    std::wstring wtoken;
    byte_encode_token(token, b2u, &wtoken);

    std::vector<std::wstring> bpe_tokens;
    bpe(wtoken, bpe_ranks, &bpe_tokens);

    for (auto ws : bpe_tokens) {
      result->push_back(wstring_to_utf8(ws));
    }
  }
}

void tokenize(const std::string& text, RE2& re, BPERanks& bpe_ranks,
              std::unordered_map<uint8_t, wchar_t>& b2u,
              std::vector<std::string>* result) {
  const std::string eot("<|endoftext|>");
  size_t s = 0;
  size_t i = text.find(eot);
  while (i != std::string::npos) {
    _tokenize(text.substr(s, i - s), re, bpe_ranks, b2u, result);
    result->push_back(eot);
    s = i + eot.size();
    i = text.find(eot, s);
  }
  _tokenize(text.substr(s), re, bpe_ranks, b2u, result);
}

void load_vocab(std::istream& ins, std::unordered_map<std::string, int>* t2i,
                std::unordered_map<int, std::string>* i2t) {
  t2i->clear();
  i2t->clear();

  std::string line;
  std::string token;
  int n = 0;
  while (std::getline(ins, line)) {
    if (n % 2 == 0) {
      token = line;
    } else {
      t2i->insert({token, std::stoi(line)});
      i2t->insert({std::stoi(line), token});
    }
    n++;
  }
}

//==============================================================================
// Tokenizer states
//==============================================================================
struct Tokenizer {
  RE2 re;
  BPERanks bpe_ranks;
  std::unordered_map<uint8_t, wchar_t> b2u;
  std::unordered_map<wchar_t, uint8_t> u2b;
  std::unordered_map<std::string, int> t2i;
  std::unordered_map<int, std::string> i2t;
  
  Tokenizer(const char* bpe_merges_path, const char* vocab_txt_path)
      : re("('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| "
           "?[^\\s\\p{L}\\p{N}]+|\\s+\\(?!\\S\\)|\\s+)") {
    std::fstream merges(bpe_merges_path, std::ios::in);
    load_merge_rules(merges, &bpe_ranks);

    bytes_to_unicode(&b2u, &u2b);

    std::fstream vocab_txt(vocab_txt_path, std::ios::in);
    load_vocab(vocab_txt, &t2i, &i2t);
  }
};
