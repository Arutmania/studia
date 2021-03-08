import numpy as np

# BIG ENTERPRISE DESING PATTERNS BENEATH

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
    def __init__(self, max):
        self.max = max
    
    def __call__(self, context):
        return self.max < context.iteration

class Algorithm:
    def __init__(self, loss, x0, method, condition):
        self.loss      = loss
        self.x0        = x0
        self.method    = method
        self.condition = condition
    
    def run(self):
        self.iteration = 0
        self.x         = self.x0
        # set time

        while self.condition(self):
            self.x = self.method(self.loss, self.x)
            self.iteration += 1
        
        return self.x, self.loss(self.x)

if __name__ == '__main__':
    pass
