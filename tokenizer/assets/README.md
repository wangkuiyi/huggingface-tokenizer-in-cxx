The script https://github.com/iree-org/iree-jax/blob/main/models/gpt2/setup.sh reveals how to download data files required by GPT-2.  The HuggingFace tokenizer requires two files: `merges.txt` and `vocab.json`.  Our simplified Python tokenizer `tokenizer/bpe.py` also requires these two files  Our C++ version `tokenizer/bpe.{h,cc}` requires `vocab.txt`.  To get `vocab.txt`, we can run `tool/json-to-txt.py`:

```bash
python tool/json-to-txt.py tokenizer/assets/vocab.json > tokenizer/assets/vocab.txt
```
