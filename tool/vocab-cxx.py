#!/usr/bin/env python3

# This Python script converts the vocab.json file required by the GPT2
# tokenizer into an unordered_map<string, int> literal.
#
#  ./vocab-cxx.py assets/vocab.json > vocab.cc
#
import sys
import json

vocab_file = sys.argv[1]

PREEMBLE = """
#include <unordered_map>
#include <string>

std::unordered_map<std::string, int> gpt2_tokenizer_vocab = {"""

with open(vocab_file) as f:
    d = json.load(f)
    print(PREEMBLE)
    for k, v in d.items():
        print(f"""{{ "{k}", {v} }},""")
    print("};")
