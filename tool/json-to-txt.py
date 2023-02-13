import sys
import json

vocab_file = sys.argv[1]

with open(vocab_file) as f:
    d = json.load(f)
    for k, v in d.items():
        print(k)
        print(v)
