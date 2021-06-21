#!/bin/env python3

from torch.utils.data import DataLoader, TensorDataset
from torch import nn
import numpy as np
import torch
import matplotlib.pyplot as plt


class Net(nn.Module):
    def __init__(self):
        super().__init__()
        self.net = nn.Sequential(
            nn.Linear(1, 1024),
            nn.ReLU(inplace=True),
            nn.Tanh(),
            nn.Linear(1024, 1),
        )

    def forward(self, X):
        return self.net(X)


LEARNING_RATE = 1e-3
EPOCHS = 1000
BATCH_SIZE = 100
P = {
    1: 2,  # Piotr Kochański       289372
    2: 0,  # Filip Korzeniewski    293070
}


def f(x):
    return torch.sin(x * np.sqrt(P[1] + 1)) + torch.cos(x * np.sqrt(P[2] + 1))


if __name__ == "__main__":
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    net = Net().to(device)
    # the optimizer - Adam is type of SDG
    optim = torch.optim.Adam(net.parameters(), lr=LEARNING_RATE)
    # mean square error loss function
    criterion = nn.MSELoss()

    # 1000 samples from [-10, 10], float32, shape (1000, 1)
    X = torch.from_numpy(np.linspace(-10, 10, 1000)).float().unsqueeze(1)
    Y = f(X)

    dataloader = DataLoader(
        TensorDataset(X, Y), batch_size=BATCH_SIZE, shuffle=True, pin_memory=True
    )

    for epoch in range(1, EPOCHS + 1):
        for batch, expected in dataloader:
            loss = criterion(net(batch.to(device)), expected.to(device))
            optim.zero_grad()
            loss.backward()
            optim.step()

        if epoch % 100 == 0:
            print(f"epoch #{epoch}: {loss.item()}")

    predict = net(X.to(device))

    # Plot showing the difference between predicted and real data
    x, y = X.detach().cpu().numpy(), Y.detach().cpu().numpy()
    plt.plot(x, y, label="factual")
    plt.plot(x, predict.detach().numpy(), label="predicted")
    plt.title("function")
    plt.xlabel("x")
    plt.ylabel("f(x)")
    plt.legend()
    plt.savefig(fname="result.png")
