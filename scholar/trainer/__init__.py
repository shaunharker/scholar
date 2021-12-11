import torch
import numpy as np
import copy
import random
from time import time


class Trainer:
    """
    Encapsulates `model`, `optimizer`, `dataset`, `batch_size`, `example_length` for the purposes of training.
    """
    def __init__(self, model=None, optimizer=None, dataset=None, batch_size=None, example_length=None, device=None):
        self.model = model
        self.optimizer = optimizer
        self.dataset = dataset
        self.batch_size = batch_size
        self.example_length = example_length
        self.n = 0
        if device is None:
            device = 'cuda'
        self.device = device

    @staticmethod
    def load_from_path(path):
        """
        Load the `Trainer` object stored at `path` and return it.
        """
        trainer = Trainer()
        trainer.load(path)
        return trainer

    def load(self, path):
        """
        Load the `Trainer` object stored at `path` into `self`.
        """
        checkpoint = torch.load(path)
        self.model = checkpoint.get("model", None)
        self.optimizer = checkpoint.get("optimizer", None)
        self.dataset = checkpoint.get("dataset", None)
        self.batch_size = checkpoint.get("batch_size", None)
        self.example_length = checkpoint.get("example_length", None)
        self.n = checkpoint.get("n", 0)

    def save(self, path):
        """
        Save `self` to `path`.
        """
        checkpoint = {
            "model": self.model,
            "optimizer": self.optimizer,
            "dataset": self.dataset,
            "batch_size": self.batch_size,
            "example_length": self.example_length,
            "n": self.n}
        torch.save(checkpoint, path)

    def clone(self):
        """
        Create a clone of `self` and return it.
        """
        try:
            clone = copy.deepcopy(self)
        except Exception as e:
            print('clone exception', e)
        return clone

    def clone_model(self):
        return copy.deepcopy(self.model)

    def step(self):
        """
        Use `self.optimizer` to train `self.model` for one step using a batch obtained from `self.dataset` using training hyperparameters `self.batch_size` and `self.example_length`.
        """
        def closure():
            batch = self.dataset.batch(batch_size=self.batch_size, example_length=self.example_length, offset=None)
            losses = self.model(batch)
            losses = torch.nan_to_num(losses, nan=0.0, posinf=0.0, neginf=0.0)
            loss = torch.mean(losses.detach()).item()
            return loss

        loss = self.optimizer.step(closure)
        self.n += 1
        return loss, baseline_loss