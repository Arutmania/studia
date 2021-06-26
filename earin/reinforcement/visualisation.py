#!/bin/env python3

import gym
import numpy as np

TIMESTAMPS = 200

if __name__ == "__main__":
    env = gym.make("Taxi-v3")
    Q = np.load("model.npy", "r")

    s = env.reset()
    total = 0
    for timestamp in range(TIMESTAMPS):
        env.render()

        a = np.argmax(Q[s, :])
        s, r, done, info = env.step(a)
        total += r

        if done:
            break

    env.close()
    print(f"finished {timestamp} timestamps with reward: {total}")
