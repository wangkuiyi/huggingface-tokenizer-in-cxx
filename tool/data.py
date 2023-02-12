# This script finds lyrics of Frech songs in the dataset
# https://www.kaggle.com/datasets/neisse/scrapped-lyrics-from-6-genres
#
# unzip the downloaded archive.zip file in /tmp, we get:
#   /tmp/lyrics-data.csv
#   /tmp/artists-data.csv
#
import pandas as pd

artists = pd.read_csv("/tmp/artists-data.csv")  # 4168 artists
artists = artists[artists["Popularity"] > 5]  # 297 popular artists

lyrics = pd.read_csv("/tmp/lyrics-data.csv")  # 379931 songs
lyrics = lyrics[lyrics["language"] == "fr"]

df = lyrics.merge(
    artists[["Artist", "Genres", "Link"]], left_on="ALink", right_on="Link", how="inner"
)

df = df[df["Lyric"].apply(lambda x: len(x.split(" ")) < 350)]

df = df["Lyric"]  # 317 songs

for row in df:
    # https://stackoverflow.com/a/2077944/724872
    print(" ".join(row.replace("\n", " ").split()))
