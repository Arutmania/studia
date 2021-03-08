import numpy as np
import time

# BIG ENTERPRISE DESIGN PATTERNS BENEATH

class Loss:
    '''quadratic vector form'''
    def __init__(self, A, b, c):
        self.A, self.b, self.c = A, b, c

    def __call__(self, x):
        return x.T @ self.A @ x + self.b.T @ x + self.c

    def gradient(self, x):
        return (self.A.T + self.A) @ x + self.b

    def hessian(self, x):
        return self.A.T + self.A

'''learning rate'''
RATE = 0.1

# a method is a callable taking loss function and current x, returning next x
def simple_gradient_descent_method(loss, x):
    return x - RATE * loss.gradient(x)

def newtons_method(loss, x):
    return x - RATE * np.linalg.inv(loss.hessian(x)) @ loss.gradient(x)

# a condition is a callable taking algorithm and returning whether it should continue
class IterationCondition:
    '''terminate algorithm after `max` iterations'''
    def __init__(self, max):
        self.max = max

    def __call__(self, context):
        return self.max < context.iteration

class TimeCondition:
    '''terminate algorithm after `max` process nanoseconds'''
    def __init__(self, max):
        self.max = max

    def __call__(self, context):
        return self.max < time.process_time_ns() - context.starttime

    @staticmethod
    def seconds(max):
        return TimeCondition.miliseconds(max * 1000)

    @staticmethod
    def miliseconds(max):
        return TimeCondition.microseconds(max * 1000)

    @staticmethod
    def microseconds(max):
        return TimeCondition(max * 1000)

class ValueCondition:
    '''terminate algorithm after `value` with desired `eps` precision is reached'''
    def __init__(self, value, eps=np.finfo(float).eps):
        self.value, self.eps = value, eps

    def __call__(self, context):
        return np.abs(context.loss(context.x) - self.value) > 1.5 * self.eps

class Algorithm:
    def __init__(self, loss, x0, method, condition):
        self.loss      = loss
        self.x0        = x0
        self.method    = method
        self.condition = condition

    def run(self):
        self.iteration = 0
        self.x         = self.x0
        self.startime  = time.process_time_ns()

        while self.condition(self):
            self.x = self.method(self.loss, self.x)
            self.iteration += 1

        return self.x, self.loss(self.x)

if __name__ == '__main__':
    pass
