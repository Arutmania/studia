import json
import copy
import random
from math import prod
import argparse
from sys import float_info
from itertools import product


class Node:
    def __init__(self, name):
        self.name = name
        self.parents = []
        self.children = []
        self.probs = {}
        self.possible_values = []
        self.value = ""

    def __repr__(self):
        repr = f"Node(\n\t'{self.name}'\n\tparents: [\n"
        for parent in self.parents:
            repr += f"\t\t{parent.name},\n"
        repr += "\t],\n\tchildren: [\n"
        for child in self.children:
            repr += f"\t\t{child.name},\n"
        repr += "\t],\n\tprobs: {\n"
        for k, v in self.probs.items():
            repr += f"\t\t{k}: {v},\n"
        repr += "\t}\n)\n"
        return repr

    __str__ = __repr__

    def add_parent(self, parent):
        self.parents.append(parent)
        parent.children.append(self)

    def add_probability(self, outcome, probability):
        """
        add outcome with its probability
        outcome is a tuple of possible values of parents and self in order
        probability is how likely is that to happen
        if last element of outcome - that is a value of self is not in self
        possible values add it
        """
        self.probs[outcome] = probability
        if outcome[-1] not in self.possible_values:
            self.possible_values.append(outcome[-1])

    def markov_blanket(self):
        """
        markov blanket for a variable is the parents of the variable,
        children of the variable and parents of the children
        """
        blanket = self.parents + self.children
        for child in self.children:
            for parent in child.parents:
                if parent not in blanket and parent is not self:
                    blanket.append(parent)
        return blanket

    def probabilities_valid(self):
        """
        check if all appropriate probabilities add up to approximately 1,
        that is for each possibility of parent outcome probability of outcomes
        for self adds up to about 1
        """
        return all(
            [
                # equal to about 1
                abs(
                    # sum of probabilities for that parent outcome
                    sum([self.probs[outcome + (value,)] for value in self.possible_values])
                    - 1
                )
                < float_info.epsilon * 1.5
                # all possible parent outcomes
                for outcome in product(
                    *[parent.possible_values for parent in self.parents]
                )
            ]
        )


class Net:
    def __init__(self, nodes):
        self.nodes = nodes

        if not self.is_acyclic():
            raise ValueError("Bayesian Net should be acyclic")
        
        if not all([node.probabilities_valid() for node in self.nodes.values()]):
            raise ValueError("Invalid probabilities")

    def __repr__(self):
        repr = "Net("
        for node in self.nodes.values():
            for line in f"{node}".splitlines():
                repr += f"\n\t{line}"
            repr += ","
        repr += "\n)\n"
        return repr

    __str__ = __repr__

    @staticmethod
    def load(filename):
        with open(filename) as file:
            data = json.load(file)

        nodes = {name: Node(name) for name in data["nodes"]}
        for name, relation in data["relations"].items():
            for parent in relation["parents"]:
                nodes[name].add_parent(nodes[parent])

            for outcome, probability in relation["probabilities"].items():
                nodes[name].add_probability(tuple(outcome.split(",")), probability)

        return Net(nodes)

    def markov_blanket(self, node):
        return self.nodes[node].markov_blanket()

    def is_acyclic(self):
        def impl(nodes):
            if not nodes:
                return True

            # no more nodes in graph => acyclic
            leaf = None
            for node in nodes.values():
                if not node.children:
                    leaf = node
                    break

            # graph has no leaf => cyclic
            if leaf is None:
                return False

            for node in nodes.values():
                for child in node.children:
                    # child == leaf
                    if child is leaf:
                        node.children.remove(leaf)

            del nodes[leaf.name]

            return impl(nodes)

        return impl(copy.deepcopy(self.nodes))

    def mcmc(self, evidence, steps, burnin=0):
        """
        run MCMC algorithm for bayesian network described by self
        - evidence is a dictionary of observed variables and their values
        - steps is amount of steps taken by the algorithm that are measured
        - burnin denotes additional steps taken beforehand that make for better
        initial values
        """

        # evidence: {name: value}

        def roulette(node):
            """
            return new value based on conditional probability of nodes in markov chain
            """

            def outcome(node, value):
                """produces a tuple representing an outcome"""
                return tuple([p.value for p in node.parents] + [value])

            def weight(value):
                """weitght with which this value should be choosen"""
                return node.probs[outcome(node, value)] * prod(
                    [
                        child.probs[outcome(child, child.value)]
                        for child in node.children
                    ]
                )

            weights = [weight(value) for value in node.possible_values]
            return random.choices(node.possible_values, weights, k=1)[0]

        def normalize(counters):
            """
            normalize by taking counts of values for each variable into
            probability of each value through dividing amount of that value by
            total amount for that variable
            """
            # counters: {variable: {value: count}}
            for samples in counters.values():
                total = sum(samples.values())
                for value, count in samples.items():
                    samples[value] = count / total

            return counters

        variables, counters = [], {}

        for name, node in self.nodes.items():
            if name in evidence:
                node.value = evidence[name]
            else:
                node.value = random.choice(node.possible_values)
                variables.append(name)  # or node?
                counters[name] = {value: 0 for value in node.possible_values}

        for _ in range(burnin):
            current = self.nodes[random.choice(variables)]
            current.value = roulette(current)

        for _ in range(steps):
            current = self.nodes[random.choice(variables)]
            current.value = roulette(current)
            counters[current.name][current.value] += 1

        return normalize(counters)


def setup_parser():
    # main.py filename -b burn-in? -i iterations -e [evidence] -q [query]?
    parser = argparse.ArgumentParser()

    parser.add_argument("filename", help="JSON file with the definition of the Net")

    parser.add_argument(
        "-i", type=int, required=True, help="number of iterations", dest="iterations"
    )

    parser.add_argument(
        "-b",
        default=0,
        type=int,
        required=False,
        help="number of burn-in iterations",
        dest="burnin",
    )

    parser.add_argument(
        "-e",
        default=[],
        action="extend",
        nargs="+",
        required=False,
        help="list of evidence values - pairs of variable value",
        dest="evidence",
    )

    parser.add_argument(
        "-q",
        default=[],
        action="extend",
        nargs="+",
        required=False,
        help="list of query variables",
        dest="query",
    )

    parser.add_argument(
        "--markov",
        required=False,
        help="""
        show markov blanket for variable, all other arguments are
        unfortunately required but ignored
        """
    )

    return parser


def parse_args(parser):
    args = parser.parse_args()
    
    it = iter(args.evidence)
    evidence = {variable: next(it) for variable in it}

    return (
        args.filename,
        args.iterations,
        args.burnin,
        evidence,
        args.query,
        args.markov,
    )


if __name__ == "__main__":
    filename, iterations, burnin, evidence, query, markov = parse_args(setup_parser())

    net = Net.load(filename)

    if markov:
        print(net.markov_blanket(markov))
    else:
        results = net.mcmc(evidence, iterations, burnin)

        if query:
            for variable in query:
                print(f"{variable}: {results[variable]}")
        else:
            for variable, result in results.items():
                print(f"{variable}: {result}")