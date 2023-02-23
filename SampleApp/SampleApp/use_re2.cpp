//
//  use_re2.cpp
//  RE2Sample
//
//  Created by Yi Wang on 2/12/23.
//

#include <re2/re2.h>
#include <re2/stringpiece.h>
#include <stdio.h>
#include <tokenizer/bpe.h>

#include <iostream>
#include <string>

extern "C" void test_tokenizer(const char* merges_path,
                               const char* vocab_path) {
  RE2 re(
      "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| "
      "?[^\\s\\p{L}\\p{N}]+|\\s+\\(?!\\S\\)|\\s+)");

  BPERanks bpe_ranks;
  std::fstream merges(merges_path, std::ios::in);
  load_merge_rules(merges, &bpe_ranks);

  std::unordered_map<uint8_t, wchar_t> b2u;
  std::unordered_map<wchar_t, uint8_t> u2b;
  bytes_to_unicode(&b2u, &u2b);

  std::unordered_map<std::string, int> t2i;
  std::unordered_map<int, std::string> i2t;
  std::fstream vocab_txt(vocab_path, std::ios::in);
  load_vocab(vocab_txt, &t2i, &i2t);

  std::vector<std::string> candidates = {
      "this is <|endoftext|> else<|endoftext|>",
      "<|endoftext|> else<|endoftext|>", "this is <|endoftext|> else",
      "this is <|endoftext|>else", "this is else"};
  for (auto s : candidates) {
    std::vector<int> ids;
    encode(s, re, bpe_ranks, b2u, t2i, &ids);
    assert(ids.size() > 0);
    assert(decode(ids, u2b, i2t) == s);
  }
  std::cout << "Passed testing";
}
