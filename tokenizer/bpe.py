import regex as re
import json
import os

def bytes_to_unicode():
    bs = (
        list(range(ord("!"), ord("~") + 1))
        + list(range(ord("¡"), ord("¬") + 1))
        + list(range(ord("®"), ord("ÿ") + 1))
    )
    cs = bs[:]
    n = 0
    for b in range(2**8):
        if b not in bs:
            bs.append(b)
            cs.append(2**8 + n)
            n += 1
    cs = [chr(n) for n in cs]
    return dict(zip(bs, cs))


def get_pairs(word):
    """
    Return set of symbol pairs in a word.

    Word is represented as tuple of symbols (symbols being variable-length strings).
    """
    pairs = set()
    prev_char = word[0]
    for char in word[1:]:
        pairs.add((prev_char, char))
        prev_char = char
    return pairs


class Tokenizer:
    def __init__(self, assert_dir:str):
        self.unk_token = "<|unknown token|>"

        self.pat = re.compile(
            r"""'s|'t|'re|'ve|'m|'ll|'d| ?\p{L}+| ?\p{N}+| ?[^\s\p{L}\p{N}]+|\s+(?!\S)|\s+"""
        )

        with open(os.path.join(assert_dir, "vocab.json"), encoding="utf-8") as vocab_handle:
            self.encoder = json.load(vocab_handle)
        self.decoder = {v: k for k, v in self.encoder.items()}

        self.byte_encoder = bytes_to_unicode()
        self.byte_decoder = {v: k for k, v in self.byte_encoder.items()}

        self.cache = {}
        with open(os.path.join(assert_dir,"merges.txt"), encoding="utf-8") as merges_handle:
            bpe_merges = merges_handle.read().split("\n")[1:-1]
        bpe_merges = [tuple(merge.split()) for merge in bpe_merges]
        self.bpe_ranks = dict(zip(bpe_merges, range(len(bpe_merges))))

    def bpe(self, token):
        if token in self.cache:
            return self.cache[token]
        word = tuple(token)
        pairs = get_pairs(word)
        if not pairs:
            return token

        while True:
            bigram = min(pairs, key=lambda pair: self.bpe_ranks.get(pair, float("inf")))
            if bigram not in self.bpe_ranks:
                break
            first, second = bigram
            new_word = []
            i = 0
            while i < len(word):
                try:
                    j = word.index(first, i)
                except ValueError:
                    new_word.extend(word[i:])
                    break
                else:
                    new_word.extend(word[i:j])
                    i = j

                if word[i] == first and i < len(word) - 1 and word[i + 1] == second:
                    new_word.append(first + second)
                    i += 2
                else:
                    new_word.append(word[i])
                    i += 1

            new_word = tuple(new_word)
            word = new_word
            if len(word) == 1:
                break
            else:
                pairs = get_pairs(word)
        word = " ".join(word)
        self.cache[token] = word
        return word

    def _tokenize(self, text):
        """Tokenize a string."""
        bpe_tokens = []
        for token in re.findall(self.pat, text):
            token = "".join(
                self.byte_encoder[b] for b in token.encode("utf-8")
            )  # Maps all our bytes to unicode strings, avoiding control tokens of the BPE (spaces in our case)
            bpe_tokens.extend(bpe_token for bpe_token in self.bpe(token).split(" "))
        return bpe_tokens

    def tokenize(self, text):
        bpe_tokens = []
        eot = "<|endoftext|>"
        s = 0
        i = text.find(eot)
        while i != -1:
            bpe_tokens.extend(self._tokenize(text[s:i]))
            bpe_tokens.append(eot)
            s = i + len(eot)
            i = text.find(eot, i + len(eot))
        bpe_tokens.extend(self._tokenize(text[s:]))
        return bpe_tokens

    def encode(self, text):
        return [self.encoder[token] for token in self.tokenize(text)]

    def decode(self, indices):
        text = ''.join([self.decoder.get(index) for index in indices])
        return bytearray(self.byte_decoder[c] for c in text).decode('utf-8')


def test_tokenizer():
    t = Tokenizer()
    # with open("/tmp/sample.txt") as f:
    #     for line in f:
    #         lst = t._tokenize(line[:-1]) # Remove the trailing '\n'.
    #         print(*lst, sep=', ') # Do no quote strings.
    candidates = [
        "this is <|endoftext|> else<|endoftext|>",
        "<|endoftext|> else<|endoftext|>",
        "this is <|endoftext|> else",
        "this is <|endoftext|>else",
        "this is else",
    ]
    for s in candidates:
        assert t.decode(t.encode(s)) == s
