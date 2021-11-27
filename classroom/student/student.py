import torch
import numpy as np
import copy
import random
from time import time
from ..dataset.utf8 import utf8encode
from ..dataset.utf8 import utf8decode
from ..dataset import BytesDataset
from .baselinecomparison import BaselineComparison

class Student:
    """
    Encapsulates `model`, `optimizer`, `dataset`, `batch_size`, `example_length` for the purposes of training.
    Stores training metrics (`time`, `times`, `grades`) generated by calls to `study`.
    Writes lists for plotting performance and other experimental data.
    ### Notes:
    * `save` and `load` serialize to and from disk
    * `clone` creates a clone which is a deepcopy except for `self.parent`, which is not a copy.
    * `mutate` mutates the student by altering `self.batch_size` by a randomly chosen factor and similiary altering optimizer learning rates.
    """
    def __init__(self, model=None, optimizer=None, dataset=None, batch_size=None, example_length=None, device=None):
        self.model = model
        self.optimizer = optimizer
        self.dataset = dataset
        self.batch_size = batch_size
        self.example_length = example_length

        self.step = 0
        self.time = 0.0
        self.times = []
        self.grades = []

        self.parent = None

        self.baseline = None
        self.baseline_grades = []
        self.predicted_grades = []
        self.shaping = lambda batch, losses: torch.mean(losses)
        if device is None:
            device = 'cuda'
        self.device = device

    @staticmethod
    def load_from_path(path):
        """
        Load the `Student` object stored at `path` and return it.
        """
        student = Student()
        student.load(path)
        return student

    def load(self, path):
        """
        Load the `Student` object stored at `path` into `self`.
        """
        checkpoint = torch.load(path)
        self.model = checkpoint.get("model", None)
        self.optimizer = checkpoint.get("optimizer", None)
        self.dataset = checkpoint.get("dataset", None)
        self.batch_size = checkpoint.get("batch_size", None)
        self.example_length = checkpoint.get("example_length", None)
        self.step = checkpoint.get("step", 0)
        self.time = checkpoint.get("time", 0.0)
        self.times = checkpoint.get("times", [])
        self.grades = checkpoint.get("grades", [])
        self.parent = checkpoint.get("parent", None)
        self.baseline = checkpoint.get("baseline", None)
        self.baseline_grades = checkpoint.get("baseline_grades", [])
        self.predicted_grades = checkpoint.get("predicted_grades", [])

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
            "step": self.step,
            "time": self.time,
            "times": self.times,
            "grades": self.grades,
            "parent": self.parent,
            "baseline": self.baseline,
            "baseline_grades": self.baseline_grades,
            "predicted_grades": self.predicted_grades}
        torch.save(checkpoint, path)

    def clone(self):
        """
        Create a clone of `self` and return it. The clone's `parent` and `baseline` attributes (if present) will be the same reference as the original. Everything else will be a deep copy.
        """
        tmp1 = self.parent
        self.parent = None
        if self.baseline is not None:
            tmp2 = self.baseline.baseline_model
            self.baseline.baseline_model = None
        try:
            clone = copy.deepcopy(self)
        except Exception as e:
            print('clone exception', e)
        self.parent = tmp1
        clone.parent = tmp1
        if self.baseline is not None:
            self.baseline.baseline_model = tmp2
            clone.baseline.baseline_model = tmp2
        return clone

    def reset_baseline(self):
        if self.baseline is None:
            cloned_model = copy.deepcopy(self.model)
            self.baseline = BaselineComparison(cloned_model)
        else:
            for (p, q) in zip(self.baseline.baseline_model.parameters(), self.model.parameters()):
                p.data = q.detach().clone()
        self.time_of_last_baseline = self.time

    def study(self):
        """
        Use `self.optimizer` to train `self.model` for one step using a batch obtained from `self.dataset` using training hyperparameters `self.batch_size` and `self.example_length`.
        Add/append the resulting training data to `self.time`, `self.times`, `self.grades`, `self.baseline_grades`, and `self.predicted_grades`.
        """
        def closure():
            batch = self.dataset.batch(batch_size=self.batch_size, example_length=self.example_length, offset=None)
            losses = self.model(batch)
            losses = torch.nan_to_num(losses, nan=0.0, posinf=0.0, neginf=0.0)
            shaped_loss = self.shaping(batch, losses)
            shaped_loss.backward()
            if self.baseline is not None:
                predicted_losses, baseline_losses = self.baseline.update(batch, self.step, losses)
                baseline_losses = torch.nan_to_num(baseline_losses, nan=0.0, posinf=0.0, neginf=0.0)
            else:
                predicted_losses = None
                baseline_losses = None
            return losses, baseline_losses, predicted_losses
        start = time()
        losses, baseline_losses, predicted_losses = self.optimizer.step(closure)
        elapsed = time() - start
        self.step += 1
        self.time += elapsed
        self.times.append(elapsed)
        grade = 1.0 - torch.mean(losses.detach()).item()
        self.grades.append(grade)
        if self.baseline is not None:
            baseline_grade = 1.0 - torch.mean(baseline_losses.detach()).item()
            predicted_grade = 1.0 - torch.mean(predicted_losses.detach()).item()
        else:
            baseline_grade = grade
            predicted_grade = 0.0
        self.baseline_grades.append(baseline_grade)
        self.predicted_grades.append(predicted_grade)

    def parameter_histograms(self):
        """
        Return a dictionary the keys of which are the names of parameters
        as returned by `self.model.named_parameters()` and the values of
        which are pairs (X, Y) which give the pdf of the distribution of
        individual parameter values.
        ### Example
        ```python
        H = student.parameter_histograms()
        plots = [Plot(x="value",y=f"pdf",**{key: H[key]}) for key in H]
        plots[0]
        ```
        """
        pd = {name: p for (name, p) in self.model.named_parameters()}
        H = {}
        for (name, p) in pd.items():
            n = torch.numel(p)
            bins = math.floor(math.sqrt(n))
            data = p.detach().cpu().numpy().reshape(-1)
            Y, X = np.histogram(data, bins=int(len(data)**(1/2)), density=True)
            H[name] = (X, Y)
        return H

    def mutate(self):
        """
        Mutate `self` by randomly altering `self.batch_size` and `self.optimizer.lr[p]`
        """
        r = random.choice([0.9, 0.95, 1.05, 1.1])
        self.batch_size = int(r*self.batch_size)
        for p in self.optimizer.lr:
            lr = self.optimizer.lr[p](0)
            lr = lr * random.choice([0.9, 0.95, 1.05, 1.1])
            if lr == 0.0:
                lr = 1e-8
            self.optimizer.lr[p] = lambda n: lr * random.choice([0.9, 0.95, 1.05, 1.1])

    def stats(self):
        student = self
        n = len(student.times)-1
        time = student.time #sum(student.times[:n])
        mean_grade = np.mean(np.array(student.grades[n//2:n]))
        mean_baseline_grade = np.mean(np.array(student.baseline_grades[n//2:n]))
        mean_improvement = mean_grade - mean_baseline_grade
        mean_predicted_grade = np.mean(np.array(student.predicted_grades[n//2:n]))
        accuracy = 1.0 - abs(mean_predicted_grade - mean_grade)/(mean_grade)
        message = '\n'.join([
            f"lr                      = {student.optimizer.state['LM.F.layers.0.weight']['lr'](n)}",
            f"batch_size              = {student.batch_size}",
            f"example_length          = {student.example_length}",
            f"n                       = {n}",
            f"time                    = {int(time)}s",
            f"steps per second        = {(n/time)}",
            f"mean_baseline_grade     = {mean_baseline_grade}",
            f"mean_grade              = {mean_grade}",
            f"mean_predicted_grade    = {mean_predicted_grade}",
            f"accuracy                = {accuracy}",
            f"mean_improvement        = {mean_improvement}",
            f"mean_improvement / step = {mean_improvement/n}",
            f"est steps to grade 1.0  = {int((1-mean_grade)/(mean_improvement/n))} steps",
            f"est time to grade 1.0   = {int((1-mean_grade)/(mean_improvement/time))} seconds",
        ])
        print(message)

    def bpc(self, lag=1024):
        return sum(8*(1.0 - grade) for grade in self.grades[-lag:])/lag

    @torch.no_grad()
    def autocomplete(self, prompt=None, n_generate=128, n_ctx=None, temp=1.0, encode=None, decode=None, output=None):
        """
        Autocomplete using the model

        ## Args
        * `prompt: str` an optional prompt to begin with
        * `n_generate: int` the number of bytes/tokens to generate
        * `n_ctx: int` the number of bytes/tokens in the context window
        * `encode` the function that can turn an str into a sequence of bytes/tokens suitable for the model.
        defaults to utf8encode
        * `decode` the function that can turn the sequences of bytes/tokens used by the model to a str
        defaults to utf8decode
        * `output: Optional[List[int]]` a list to stream the output bytes/tokens to (as `int`s; they will not be decoded to `str`).

        ## TODO
        * make streaming autocomplete with streamed characters (i.e. length 1 strings) using asyncio
        """
        Categorical = torch.distributions.Categorical
        if n_ctx is None:
            n_ctx = self.model.n_ctx
        if encode is None:
            encode = utf8encode
        if decode is None:
            decode = utf8decode
        if prompt is None:
            prompt = decode(self.dataset.batch(1, 2*n_ctx, offset=None).tolist()[0])  # kludge
        x = encode(prompt)
        x = x[-n_ctx:]
        prompt = decode(x)
        print(f"=== Prompt ===\n{prompt}\n=== Autocompletion ===\n")

        def sampler(x):
            x = list(x)
            for _ in range(n_generate):
                probs = self.model.inference(torch.tensor(x, dtype=torch.long, device=self.device).unsqueeze(0)).view(-1)[-self.model.n_vocab_out:]
                if temp > 0:
                    y = Categorical(probs=probs**(1.0/temp)).sample().item()
                else:
                    y = torch.argmax(probs).item()
                x = (x + [y])[-n_ctx:]
                if output is not None:
                    output.append(y)
                yield y
        return decode(list(sampler(x)))
