
import random
from itertools import combinations
import math
import copy
random.seed(1)
 
def euclid(a, b):
    """returns the Greatest Common Divisor of a and b"""
    a = abs(a)
    b = abs(b)
    if a < b:
        a, b = b, a
    while b != 0:
        a, b = b, a % b
    return a
 
 
def coPrime(l):
    """returns 'True' if the values in the list L are all co-prime
       otherwise, it returns 'False'. """
    for i, j in combinations(l, 2):
        if euclid(i, j) != 1:
            return False
    return True
 
 
def extendedEuclid(a, b):
    """return a tuple of three values: x, y and z, such that x is
    the GCD of a and b, and x = y * a + z * b"""
    if a == 0:
        return b, 0, 1
    else:
        g, y, x = extendedEuclid(b % a, a)
        return g, x - (b // a) * y, y
 
 
def modInv(a, m):
    """returns the multiplicative inverse of a in modulo m as a
       positive value between zero and m-1"""
    # notice that a and m need to co-prime to each other.
    if coPrime([a, m]):
        linearCombination = extendedEuclid(a, m)
        return linearCombination[1] % m
    else:
        return 0
 
 
def extractTwos(m):
    """m is a positive integer. A tuple (s, d) of integers is returned
    such that m = (2 ** s) * d."""
    # the problem can be break down to count how many '0's are there in
    # the end of bin(m). This can be done this way: m & a stretch of '1's
    # which can be represent as (2 ** n) - 1.
    assert m >= 0
    i = 0
    while m & (2 ** i) == 0:
        i += 1
    return i, m >> i
 
 
def int2baseTwo(x):
    """x is a positive integer. Convert it to base two as a list of integers