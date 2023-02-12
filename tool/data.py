# This script is from https://towardsdatascience.com/how-to-fine-tune-gpt-2-for-text-generation-ae2ea53bc272
# It processes the data from https://www.kaggle.com/datasets/neisse/scrapped-lyrics-from-6-genres
#
# unzip the downloaded archive.zip file in /tmp, we get:
#   /tmp/lyrics-data.csv
#   /tmp/artists-data.csv
#
import pandas as pd
from transformers import GPT2LMHeadModel, GPT2Tokenizer
import numpy as np
import random
import torch
from torch.utils.data import Dataset, DataLoader
from transformers import GPT2Tokenizer, GPT2LMHeadModel, AdamW, get_linear_schedule_with_warmup
from tqdm import tqdm, trange
import torch.nn.functional as F
import csv

artists = pd.read_csv('/tmp/artists-data.csv') # 4168 artists
artists = artists[artists['Popularity']>5] # 297 popular artists

lyrics = pd.read_csv('/tmp/lyrics-data.csv') # 379931 songs
lyrics = lyrics[lyrics['language']=='pt'] # 157393 Portuguese songs

df = lyrics.merge(artists[['Artist', 'Genres', 'Link']],
                  left_on='ALink', right_on='Link', how='inner') # 23362 song from popular artists.

# df = df.drop(columns=['ALink','SLink','language','Link'])

df = df[df['Lyric'].apply(lambda x: len(x.split(' ')) < 350)] # 22848 songs with proper length of lyrics.

df = df['Lyric']

# #Create a very small test set to compare generated text with the reality
# test_set = df.sample(n = 200)
# df = df.loc[~df.index.isin(test_set.index)]

# #Reset the indexes
# test_set = test_set.reset_index()
# df = df.reset_index()

# #For the test set only, keep last 20 words in a new column, then remove them from original column
# test_set['True_end_lyrics'] = test_set['Lyric'].str.split().str[-20:].apply(' '.join)
# test_set['Lyric'] = test_set['Lyric'].str.split().str[:-20].apply(' '.join)

