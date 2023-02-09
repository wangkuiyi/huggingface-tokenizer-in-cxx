This repository is a C++ version of the Python HuggingFace tokenizers.

In the HuggingFace Transformers repo, tokenization is done with 104,603 lines of Python code. It takes 5506 lines for GPT2-specific BPE. I went through the code using the Python Debugger (pdb). It turns out that most of them do nothing but virtual methods in a complicated class hierarchy. I took 120 lines of Python code and put them in the file [`bpe.py`](src/bpe.py). This program is not a weaker or simpler version; it is the full HuggingFace BPE tokenizer.

With this extraction, I can port it to C++: [`bpe.h`](src/bpe.h), [`bpe.cc`](src/bpe.cc), and [`bpe_test.cc`](src/bpe_test.cc).

The following tech notes might help you understand the Python and C++ code:

1. [Unicode in C++ and Python](doc/u.md)
1. [Understanding HuggingFace Tokenizers](doc/0.md)
1. [Unicode-enabled Regular Expression in C++](doc/1.md)

Make sure you download the vocabulary file and the BPE merge rules from HuggingFace so you can run the programs.

```bash
curl -L https://huggingface.co/gpt2/raw/main/merges.txt -o /tmp/merges.txt
curl -L https://huggingface.co/gpt2/raw/main/vocab.json -o /tmp/vocab.json
```

To run HuggingFace vanilla BPE tokenzier for GPT2, run the following commands:

```bash
pip install transformers
python tool/t.py
```

Please run the following commands to run the extracted 120-line Python tokenizer:

```bash
python bpe.py
```

To build the C++ port, you will need CMake.

```
cmake -B /tmp/b -S .
Cmake --build /tmp/b
```

Then you can run the unit test.

```bash
/tmp/b/bin/bpe_test
```

All three of the above programs load the same `vocab.json` and `merges.txt` files, so they all work exactly like HuggingFace did when it trained and served the GPT2 model.

## Known Issues

RE2 can match Unicode letters from all languages using rules like `\p{L}`. But it doesn't work with look-ahead syntax like `(?!...)`. This would make the C++ version act a little differently than the Python versions when there are more than one space between two words. Please tell me about any C++ regular expression libraries that can handle both Unicode and look-ahead.
