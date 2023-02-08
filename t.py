import transformers
import builtins
from os import path

def load_gpt2_tokenizer() -> transformers.GPT2Tokenizer:
  builtins.open, tmp_open = open, builtins.open
  gpt2_dir = "/Users/y/w/gpt2cpp/assets"
  tokenizer = transformers.GPT2Tokenizer(
      vocab_file=path.join(gpt2_dir, 'vocab.json'),
      merges_file=path.join(gpt2_dir, 'merges.txt'))
  builtins.open = tmp_open
  return tokenizer

tknzr = load_gpt2_tokenizer()
print(tknzr("zero one two three four"))
