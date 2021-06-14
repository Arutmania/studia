import gym
import numpy as np
import argparse

# training program:
#   should accept algo parameters (no. of episodes, learning rate, epsilon, gamma, decay)
#   display some quality metric (e.g. episode reward) during traning, and save trained model
# visualisation program:
#   load model and display environment state while the loaded model controls actions of the agent
# model


EPISODES = 50_000
ALPHA = 0.7
GAMMA = 0.618
EPS_MAX = 1.0
EPS_MIN = 0.01
DECAY = 0.01


def setup_parser():
    """return parser with added arguments"""
    parser = argparse.ArgumentParser()

    parser.add_argument(
        "--episodes", type=int, default=EPISODES, help="number of episodes"
    )
    parser.add_argument("--alpha", type=float, default=ALPHA, help="learning rate")
    parser.add_argument("--gamma", type=float, default=GAMMA, help="discount rate")
    parser.add_argument(
        "--eps-max", type=float, default=EPS_MAX, help="max value for epsilon"
    )
    parser.add_argument(
        "--eps-min", type=float, default=EPS_MIN, help="min value for epsilon"
    )
    parser.add_argument("--decay", type=float, default=DECAY, help="decay rate")

    return parser


def parse_args(parser):
    """validate and return supplied cmd arguments from parser"""
    args = parser.parse_args()

    for value, option in [
        (args.episodes, "number of episodes"),
        (args.alpha, "learning rate"),
        (args.gamma, "discount rate"),
        (args.eps_max, "max epislon value"),
        (args.eps_min, "epsilon min value"),
        (args.decay, "decay rate"),
    ]:
        if value <= 0:
            parser.error(f"{option} must be positive")

    if args.eps_max <= args.eps_min:
        parser.error("max epsilon value must be more than min epsilon value")

    return args.episodes, args.alpha, args.gamma, args.eps_max, args.eps_min, args.decay


if __name__ == "__main__":
    episodes, alpha, gamma, eps_max, eps_min, decay = parse_args(setup_parser())
    env = gym.make("Taxi-v3")
    Q = np.zeros((env.observation_space.n, env.action_space.n))

    # the larger the gamma the smaller the discount,
    #  this means that the learning agent cares more about the long term reward

    # maximum number of episodes should be limited to 200
    TIMESTAMPS = 200

    epsilon = eps_max

    for episode in range(1, episodes + 1):
        s = env.reset()
        total = 0
        for timestamp in range(TIMESTAMPS):
            # exploitation vs exploration
            if epsilon < np.random.uniform(0, 1):
                a = np.argmax(Q[s, :])
            else:
                a = env.action_space.sample()

            # take step
            sp, r, done, info = env.step(a)
            total += r

            # bellman equation
            Q[s, a] = Q[s, a] + alpha * (r + gamma * np.max(Q[sp, :]) - Q[s, a])
            s = sp

            if done:
                break

        epsilon = eps_min + (eps_max - eps_min) * np.exp(-decay * episode)
        if episode % 1000 == 0:
            print(
                "finished episode #{:>5} after: {:>3} timestamps, episode reward: {:>3}".format(
                    episode,
                    timestamp,
                    total,
                )
            )

    env.close()
    np.save("model.npy", Q)
