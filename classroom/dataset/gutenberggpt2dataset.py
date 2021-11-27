import os
from pathlib import Path
from random import randrange
import numpy as np
import torch
import os
from transformers import GPT2TokenizerFast
tokenizer = GPT2TokenizerFast.from_pretrained("gpt2")

user = os.environ["USER"]

def gpt2decode(tokens):
    return tokenizer.decode(tokens)

def gpt2encode(text):
    return tokenizer.encode(text)


class GutenbergGPT2Dataset:
    def __init__(self, path=None, device='cuda'):
        if path is None:
            path = f"/home/{user}/data/gutenberg.gpt2.npy"
        self.path = path
        self.device = device
        self.decode = gpt2decode
        self.encode = gpt2encode
        self._load()

    def batch(self, batch_size, example_length, offset=None):
        get_example = lambda: (lambda offset: self.data[offset:offset+example_length])(randrange(self.n_tokens-example_length))
        es = [get_example() for _ in range(batch_size)]
        return torch.tensor(
            np.stack(es).reshape(batch_size, example_length).astype(np.int32),
            dtype=torch.long,
            device=self.device)

    def __getstate__(self):
        state = self.__dict__.copy()
        del state['data']
        return state

    def __setstate__(self, state):
        self.__dict__.update(state)
        self._load()

    def _load(self):
        self.n_tokens = (Path(self.path).stat().st_size - 128)//2
        self.data = np.memmap("/home/sharker/data/gutenberg.gpt2.npy", dtype=np.uint16, mode='r', offset=128)