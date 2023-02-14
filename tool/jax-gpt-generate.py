# Conda is recommended to install the dependencies required.
#
#   conda install transformers flax

from transformers import FlaxGPT2LMHeadModel
import jax
import jax.numpy as jnp

import sys
sys.path.append('./tokenizer')
import bpe

t = bpe.Tokenizer()
model = FlaxGPT2LMHeadModel.from_pretrained("gpt2")

def predict_word(input_ids, atten_mask = None):
    if atten_mask == None:
        atten_mask = jnp.ones(input_ids.shape, dtype=int)
    print(f"input_ids = {input_ids}")
    print(f"atten_mask = {atten_mask}")
    outputs = model(input_ids=input_ids, attention_mask=atten_mask)
    next_token_logits = outputs.logits[:, -1]
    val, idx = jax.lax.top_k(next_token_logits, 1)
    input_ids = jnp.concatenate([input_ids, idx], axis=1)
    return input_ids

def text_gen(text:str, length:int) -> str:
    input = jnp.array([t.encode(text)], dtype=int)
    for _ in range(length):
        input = predict_word(input)
    return t.decode(input[0].tolist())

test_text = "Hello, my dog is cute"

def test_text_gen():
    print("def test_text_gen():")
    # assert(text_gen("Hello, my dog is cute", 10) == "Hello, my dog is cute. I'm not sure if she's a puppy")
    print(text_gen(test_text, 10))

def test_gen_with_fix_input_shape():
    print("test_gen_with_fix_input_shape():")
    ids = t.encode(test_text)
    # mask = [0 for _ in range(4)] + [1 for _ in range(len(ids)) ]
    mask = [1] * 10
    input = [50256] * 4 + ids
    
    input = jnp.array([input], dtype=int)
    mask = jnp.array([mask], dtype=int)
    for _ in range(10):
        input= predict_word(input, mask)
        mask = jnp.concatenate([mask, jnp.array([[1]])], axis=1)
    print(t.decode(input[0].tolist()))
    
def test_right_padding():
    print("test_right_padding():")
    ids = t.encode(test_text)
    # mask = [0 for _ in range(4)] + [1 for _ in range(len(ids)) ]
    mask = [1] * len(ids) + [0] * 4
    input = ids + [0] * 4
    
    input = jnp.array([input], dtype=int)
    mask = jnp.array([mask], dtype=int)
    for _ in range(10):
        input= predict_word(input, mask)
        mask = jnp.concatenate([mask, jnp.array([[1]])], axis=1)
    print(t.decode(input[0].tolist()))

if __name__ == '__main__':
    # test_gen_with_fix_input_shape()
    # test_text_gen()
    test_right_padding()

