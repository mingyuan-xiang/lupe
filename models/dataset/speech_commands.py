from torchaudio.datasets import SPEECHCOMMANDS
import torchaudio.transforms as audio_transform
from torch.nn.functional import pad
import torch
from torch.utils.data import Dataset
from pathlib import Path
import numpy as np
import pickle
import json


class SpeechCommands(Dataset):
    """
    Download and process SpeechCommands Dataset
    Preserve in pickles for datasets

    Code partially sourced from
    https://pytorch.org/tutorials/intermediate/speech_command_recognition_with_torchaudio.html
    """

    labels_to_index = {
        "yes": 0,
        "no": 1,
        "up": 2,
        "down": 3,
        "left": 4,
        "right": 5,
        "on": 6,
        "off": 7,
        "stop": 8,
        "go": 9,
        "unknown": 10,
    }

    def __init__(self, root: str, subset: str):
        super(SpeechCommands, self).__init__()
        self.root = Path(root)
        self.root.mkdir(exist_ok=True, parents=True)
        meta = self.root / "meta.json"
        pickle_dir = self.root / "pickles"
        if not meta.exists():
            self._setup(self.root, meta, pickle_dir)

        self._load(pickle_dir / subset)

    def _setup(self, root: Path, meta: Path, pickle_dir: Path):
        orig_pickle_dir = pickle_dir

        # values from the tiny ml speech command example
        n_fft = 480
        win_length = 480
        hop_length = 320
        n_mels = 40
        n_mfcc = 12

        mfcc_transform = audio_transform.MFCC(
            sample_rate=16000,
            n_mfcc=n_mfcc,
            melkwargs={
                "n_fft": n_fft,
                "n_mels": n_mels,
                "hop_length": hop_length,
                "win_length": win_length,
                "center": False,
                "mel_scale": "htk",
                "normalized" : True,
                "power" : 1,
            },
        )

        def to_pickle(data, targets, file: Path):
            d = {"labels": targets, "data": data}
            with file.open("wb") as f:
                pickle.dump(d, f)

        # just some dummy data for meta
        simplified = {}

        for subset in ["training", "validation", "testing"]:
            # setup dataset
            speech = SPEECHCOMMANDS(root, download=True, subset=subset)

            data = []
            targets = []
            pickle_index = 0
            pickle_dir = orig_pickle_dir / subset
            pickle_dir.mkdir(exist_ok=True, parents=True)

            label_count = {x: 0 for x in self.labels_to_index.keys()}
            for waveform, sample_rate, label, _, _ in speech:
                # figure out labels and targets
                if label not in self.labels_to_index:
                    label = "unknown"
                label_count[label] += 1
                targets.append(self.labels_to_index[label])

                # ensure everyone is 16000 elements long
                if waveform.shape[-1] != 16000:
                    waveform = pad(waveform, (0, 16000 - waveform.shape[-1]), value=0)
                # Apply mfcc and transform to numpy
                mfcc = mfcc_transform(waveform)
                mfcc: np.ndarray = mfcc[0].numpy().T
                mfcc = mfcc.reshape(1, mfcc.shape[0], mfcc.shape[1])
                data.append(mfcc)

                if len(data) % 10000 == 0:
                    to_pickle(
                        np.stack(data), targets, pickle_dir / f"{pickle_index}.pckl"
                    )
                    data = []
                    targets = []
                    pickle_index += 1

            to_pickle(np.stack(data), targets, pickle_dir / f"{pickle_index}.pckl")

            simplified[subset] = label_count

        with meta.open("w") as f:
            json.dump(simplified, f)

    def _load(self, pickle_dir: Path):
        self.data = []
        self.targets = []
        for pckl_file in pickle_dir.iterdir():
            with pckl_file.open("rb") as f:
                entry = pickle.load(f, encoding="latin1")
                self.data.append(entry["data"])
                self.targets.extend(entry["labels"])
        self.data = np.vstack(self.data).reshape(-1, 1, 49, 12)

    def __getitem__(self, index):
        return torch.from_numpy(self.data[index]), self.targets[index]

    def __len__(self):
        return len(self.targets)
