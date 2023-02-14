import torch

from transformers import (
    GPT2LMHeadModel,
    GPT2Tokenizer,
)

tokenizer = GPT2Tokenizer.from_pretrained("gpt2")
model = GPT2LMHeadModel.from_pretrained("gpt2")

print(f"max_sequence_length={model.config.max_position_embeddings}")

prompt_text = "Hello, my dog is cute"
input = tokenizer.encode(prompt_text, add_special_tokens=False, return_tensors="pt")
mask = torch.ones(input.shape, dtype=torch.int32)
for _ in range(10):
    outputs = model(input_ids=input, attention_mask=mask)
    next_token_logits = outputs.logits[:, -1]
    val, idx = torch.topk(next_token_logits, 1)
    input = torch.cat([input, idx], axis=1)
    mask = torch.cat([mask, torch.tensor([[1]], dtype=torch.int32)], dim=1)
text = tokenizer.decode(input[0], clean_up_tokenization_spaces=True)
print(text)

# output_sequences = model.generate(
#     input_ids=input_ids,
#     max_length=20,
#     top_k=1,
#     num_return_sequences=1,
# )
# text = tokenizer.decode(output_sequences[0], clean_up_tokenization_spaces=True)
# print(text)
