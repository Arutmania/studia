import numpy as np
import time
import argparse
import sys

# BIG ENTERPRISE DESIGN PATTERNS BENEATH


class Loss:
    """quadratic vector form"""

    def __init__(self, A, b, c):
        self.A, self.b, self.c = A, b, c

    def __call__(self, x):
        return x.T @ self.A @ x + self.b.T @ x + self.c

    def gradient(self, x):
        return (self.A.T + self.A) @ x + self.b

    def hessian(self, x):
        return self.A.T + self.A


"""learning rate"""
RATE = 0.1

# a method is a callable taking loss function and current x, returning next x
def simple_gradient_descent_method(loss, x):
    return x - RATE * loss.gradient(x)


def newtons_method(loss, x):
    return x - RATE * np.linalg.inv(loss.hessian(x)) @ loss.gradient(x)


# a condition is a callable taking algorithm and returning whether it should continue
class IterationCondition:
    """terminate algorithm after `max` iterations"""

    def __init__(self, max):
        self.max = int(max)

    def __call__(self, context):
        return self.max > context.iteration


class TimeCondition:
    """terminate algorithm after `max` process nanoseconds"""

    def __init__(self, max):
        self.max = int(max)

    def __call__(self, context):
        return self.max > time.perf_counter_ns() - context.starttime

    @staticmethod
    def parse(input):
        if input.endswith("ns"):
            return TimeCondition.microseconds(input[:-2])
        elif input.endswith("ms"):
            return TimeCondition.miliseconds(input[:-2])
        elif input.endswith("s"):
            return TimeCondition.seconds(input[:-1])
        else:
            return TimeCondition(input)

    @staticmethod
    def seconds(max):
        return TimeCondition.miliseconds(int(max) * 1000)

    @staticmethod
    def miliseconds(max):
        return TimeCondition.microseconds(int(max) * 1000)

    @staticmethod
    def microseconds(max):
        return TimeCondition(int(max) * 1000)


class ValueCondition:
    """terminate algorithm after `value` with desired `eps` precision is reached"""

    def __init__(self, value, eps=np.finfo(float).eps):
        self.value, self.eps = float(value), eps

    def __call__(self, context):
        return np.abs(context.loss(context.x) - self.value) > 1.5 * self.eps


class Algorithm:
    def __init__(self, loss, x0, method, condition):
        self.loss = loss
        self.x0 = x0
        self.method = method
        self.condition = condition

    def run(self):
        self.iteration = 0
        self.x = self.x0
        self.starttime = time.perf_counter_ns()

        while self.condition(self):
            self.x = self.method(self.loss, self.x)
            self.iteration += 1

        return self.x, self.loss(self.x)


def parse_matrix(input):
    """
    input to parse array should be a string consisting of real numbers with
    elements in a matrix row separated by commas and rows separated
    by semicolons as:
        '1, 2, 3; 4, 5, 6; 7, 8, 9'
    it returns a numpy.ndarray
    """
    # remove whitespace
    input = "".join(input.split())

    # split rows by ';' and row elements by ','
    matrix = np.array([[float(e) for e in r.split(",")] for r in input.split(";")])

    # if a column matrix flatten to a vector
    if 1 in matrix.shape:
        return matrix.flatten()
    return matrix


# https://stackoverflow.com/a/34110323
def is_positive_definite(A):
    M = np.array(A)
    return np.all(np.linalg.eigvals(M + M.T) > 0)


def setup_parser():
    parser = argparse.ArgumentParser()

    # coefficients
    parser.add_argument(
        "-A",
        type=parse_matrix,
        required=True,
        help="2nd order coefficient (NxN matrix)",
    )
    parser.add_argument(
        "-b",
        type=parse_matrix,
        required=True,
        help="1st order coefficient (N-dimensional vector)",
        metavar="b",
    )
    parser.add_argument(
        "-c",
        type=float,
        required=True,
        help="0th order coefficient (scalar)",
        metavar="c",
    )

    # method
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--gradient",
        action="store_const",
        const=simple_gradient_descent_method,
        help="use simple gradient descent method to minimize function",
        dest="method",
    )
    group.add_argument(
        "--newton",
        action="store_const",
        const=newtons_method,
        help="use newton's method to minimize function",
        dest="method",
    )

    # starting point
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "-x0", type=parse_matrix, help="starting value for x", metavar="x0"
    )
    group.add_argument(
        "--uniform",
        type=float,
        nargs=2,
        help="initialize x0 from uniform distribution [l, u]",
        metavar=("l", "u"),
    )

    # condition
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "--iterate",
        type=IterationCondition,
        help="number N of iterations the algorithm runs",
        metavar="N",
        dest="condition",
    )
    group.add_argument(
        "--time",
        type=TimeCondition.parse,
        help="""
        amount of nanoseconds N the algorithm runs (process time) 
        additionally you can use 's', 'ms' or 'ns' suffix to specify 
        seconds, miliseconds or nanoseconds
        """,
        metavar="N",
        dest="condition",
    )
    group.add_argument(
        "--value",
        type=ValueCondition,
        help="algorithm is run until desired value N is reached",
        metavar="N",
        dest="condition",
    )

    # batch mode
    parser.add_argument(
        "--batch", type=int, help="restart optimization N times", metavar="N"
    )

    return parser


def parse_args(parser):
    args = parser.parse_args()

    # verify sizes
    # if uniform create uniform vector of specified size
    try:
        if args.A.shape[0] != args.A.shape[1]:
            raise ValueError("A should be a square NxN matrix")
        if not is_positive_definite(args.A):
            raise ValueError("A should be a positive-definite matrix")
        A = args.A
        n = A.shape[0]
        if args.b.ndim != 1 or args.b.shape[0] != n:
            raise ValueError("b should be N-dimensional vector")
        b = args.b
        if args.x0 is not None:
            if x0.ndim != 1 or x0.shape[0] != n:
                raise ValueError("x0 should be a N-dimensional vector")
            x0 = lambda: args.x0
        else:
            x0 = lambda: np.random.uniform(args.uniform[0], args.uniform[1], n)
    except ValueError as e:
        sys.exit(e)

    return A, b, args.c, x0, args.method, args.condition, args.batch


if __name__ == "__main__":
    parser = setup_parser()
    A, b, c, x0, method, condition, batch = parse_args(parser)

    if batch is None:
        x, y = Algorithm(Loss(A, b, c), x0(), method, condition).run()
        print(f"found solution is {x} and function value is {y}")
    else:
        solutions, values = [], []
        for _ in range(batch):
            x, y = Algorithm(Loss(A, b, c), x0(), method, condition).run()
            solutions.append(x)
            values.append(y)
        print("mean of found solutions:", np.mean(solutions))
        print("mean of function values:", np.mean(values))
        print("standard deviation of found solutions:", np.std(solutions))
        print("standard deviation of function values:", np.std(values))
