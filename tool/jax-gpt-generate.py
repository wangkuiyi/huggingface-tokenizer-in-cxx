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

def predict_word(input_ids):
    for _ in range(10):
        atten_mask = jnp.ones(input_ids.shape, dtype=int)
        outputs = model(input_ids=input_ids, attention_mask=atten_mask)
        next_token_logits = outputs.logits[:, -1]
        _, idx = jax.lax.top_k(next_token_logits, 1)
        input_ids = jnp.concatenate([input_ids, idx], axis=1)
    return input_ids

# predict_word = jax.jit(predict_word)

def play_gpt2_flax(text:str) -> str:
    input = jnp.array([t.encode(text)], dtype=int)
    output = predict_word(input)
    return t.decode(output[0].tolist())

from timeit import default_timer as timer
start = timer()
print(play_gpt2_flax("Hello, my dog is cute"))
print(f"Used {timer() - start} seconds")
