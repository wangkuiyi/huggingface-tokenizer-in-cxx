#!/usr/bin/env python3

# This Python script converts the vocab.json file required by the GPT2
# tokenizer into an unordered_map<string, int> literal.
#
#  ./vocab-cxx.py assets/vocab.json > vocab.cc
#
import sys
import json

def cescape(s: str) -> str:
    def _cescape(c):
        if c == "\"":
            return "\\\""
        elif c == "\\":
            return "\\\\"
        else:
            return c
    return "".join([_cescape(c) for c in s])

vocab_file = sys.argv[1]

PREEMBLE = """
#include <unordered_map>
#include <string>
"""

with open(vocab_file) as f:
    d = json.load(f)
    print(PREEMBLE)

    print("std::unordered_map<std::string, int> vocab_encoder = {")
    for k, v in d.items():
        k = cescape(k)
        print(f"""{{ u8"{k}", {v} }},""")
    print("};")

    print("std::unordered_map<int, std::string> vocab_decoder = {")
    for k, v in d.items():
        k = cescape(k)
        print(f"""{{ {v}, u8"{k}" }},""")
    print("};")
