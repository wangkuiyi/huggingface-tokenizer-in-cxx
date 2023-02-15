import torch

import sys
sys.path.append('./tokenizer')
import bpe

from transformers import GPT2LMHeadModel

tokenizer = bpe.Tokenizer()
model = GPT2LMHeadModel.from_pretrained("gpt2")

prompt_text = "Hello, my dog is cute"
input = tokenizer.encode(prompt_text)
# mask = [0] * 4 + [1] * len(input)
# input = [0] * 4 + input
mask = [1] * len(input)

input = torch.tensor([input], dtype=torch.int32)
mask = torch.tensor([mask], dtype=torch.float32)
for _ in range(10):
    print(input)
    print(mask)
    outputs = model(input_ids=input, attention_mask=mask)
    next_token_logits = outputs.logits[:, -1]
    val, idx = torch.topk(next_token_logits, 1)
    input = torch.cat([input, idx], axis=1)
    mask = torch.cat([mask, torch.tensor([[1]], dtype=torch.float32)], dim=1)
text = tokenizer.decode(input[0].tolist())
print(text)

# output_sequences = model.generate(
#     input_ids=input_ids,
#     max_length=20,
#     top_k=1,
#     num_return_sequences=1,
# )
# text = tokenizer.decode(output_sequences[0], clean_up_tokenization_spaces=True)
# print(text)
