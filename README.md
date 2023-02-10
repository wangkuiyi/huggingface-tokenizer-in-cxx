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

## Known Issue

RE2 can match Unicode letters from all languages using rules like `\p{L}`. But it doesn't work with look-ahead syntax like `(?!...)`. This would make the C++ version act a little differently than the Python versions when there are more than one space between two words. Please tell me about any C++ regular expression libraries that can handle both Unicode and look-ahead.

This does not seem a big issue because among the 15,231,221 lines in the [lyrics text dataset](https://www.kaggle.com/datasets/neisse/scrapped-lyrics-from-6-genres) of 54 languages, the C++ tokenizer generates different output from the HuggingFace one for only 4 lines. And the reason are all due to successive two whitespaces.

```bash
cmake --build /tmp/b && /tmp/b/bin/bpe_test | diff - <(python tool/t.py)
```
```
/, dis, ney, /,, L, ilo, Ġ&, ĠSt, itch, Ġ-, Ġ, Ġ, He, ĠMe, le, ĠNo, ĠLil, o, ,, /, dis, ney, /, l, ilo, -, st, itch, -, he, -, me, le, -, no, -, l, ilo, ., html, ,", V, ocal, Ġ1, :
/, dis, ney, /,, L, ilo, Ġ&, ĠSt, itch, Ġ-, Ġ, ĠHe, ĠMe, le, ĠNo, ĠLil, o, ,, /, dis, ney, /, l, ilo, -, st, itch, -, he, -, me, le, -, no, -, l, ilo, ., html, ,", V, ocal, Ġ1, :

/disney/,Lilo & Stitch -  He Mele No Lilo,/disney/lilo-stitch-he-mele-no-lilo.html,"Vocal 1:

Let, Ġyour, Ġvoice, Ġbe, Ġall, ĠI, ĠâĢ, ¨, H, ear, Ġnow
Let, Ġyour, Ġvoice, Ġbe, Ġall, ĠI, Ġ, âĢ, ¨, H, ear, Ġnow

Let your voice be all I  Hear now

Let, Ġyour, Ġvoice, Ġbe, Ġall, ĠI, ĠâĢ, ¨, H, ear, Ġnow
Let, Ġyour, Ġvoice, Ġbe, Ġall, ĠI, Ġ, âĢ, ¨, H, ear, Ġnow

Let your voice be all I  Hear now

/, m, um, /,, P, rophe, cies, Ġ&, Ġ, Ġ, Re, vers, ed, ĠMemories, ,, /, m, um, /, prop, he, cies, -, re, vers, ed, -, mem, ories, ., html, ,", (, L, al, ala, Ġl, al, ala, Ġl, al, ala, Ġ:, Ġl, ala, )
/, m, um, /,, P, rophe, cies, Ġ&, Ġ, ĠRe, vers, ed, ĠMemories, ,, /, m, um, /, prop, he, cies, -, re, vers, ed, -, mem, ories, ., html, ,", (, L, al, ala, Ġl, al, ala, Ġl, al, ala, Ġ:, Ġl, ala, )

/mum/,Prophecies &  Reversed Memories,/mum/prophecies-reversed-memories.html,"(Lalala lalala lalala : lala)
```

