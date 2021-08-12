from pprint import pprint
import csv
import math
from random import randint
from multiprocessing import Process, Queue
import sys

def ModInverse(a, n):
	""" Calculates the modular inverse of a mod n.
		http://www.wikiwand.com/en/Extended_Euclidean_algorithm#/Modular_integers
	"""
	(t, new_t, r, new_r) = 0, 1, int(n), int(a)
	while new_r != 0:
		quotient = r//new_r
		(t, new_t) = (new_t, t - quotient * new_t)
		(r, new_r) = (new_r, r - quotient * new_r)
	if r > 1:
		raise ArithmeticError("ERROR: %d is not invertible modulo %d. \n r was: %d, new_r was %d " % (a, n, r, new_r))
	if t < 0:
		t = t + n
	return t

def MongomeryProduct(a, b,n,nprime,r):
	""" Montgomery product."""
	t = a * b
	m = t * nprime % r
	u = (t + m*n)/r
	return (u-n,True) if (u >= n) else (u,False)

def rsa(m, d, n, nPrime, r):
	""" Sign a message using the provided key.
		This is used to detect whether we have guessed the correct key.
		We sign a random message from the data set, and if we end up with a
		signature that matches the corresponding signature in the data set,
		we are done.
	"""
	mm = (m*r)%n
	x_bar = (1*r)%n
	k = len(d)
	sub_count = 0
	for i in range(0, k):
		sub = False
		x_bar, tmp = MongomeryProduct(x_bar,x_bar, n, nPrime, r)
		if d[i]=='1':
			x_bar, sub = MongomeryProduct(mm, x_bar, n, nPrime, r)

		sub_count += int(sub)
	x, tmp = MongomeryProduct(x_bar, 1, n, nPrime, r)
	return x, sub_count


def rsa_sim(m, d, n, nPrime, r, j):
	""" Simulates rsa signing with the current derived key.
		Calculates whether a subtraction was made during step4
		in the final Montgomery multiplication. 
	"""
	mm = (m*r)%n
	x_bar = (1*r)%n
	k = len(d)
	dd = d[:j]
	dd += '1'
	k = len(dd)
	sub = False
	for i in range(0, k):
		x_bar, tmp = MongomeryProduct(x_bar,x_bar, n, nPrime, r)
		#sub = True
		if dd[i]=='1':
			x_bar, sub = MongomeryProduct(mm, x_bar, n, nPrime, r)
			#print sub
	x, tmp = MongomeryProduct(x_bar, 1, n, nPrime, r)
	return x, sub

def do_sim(q_t, q_f, mlist, d, n, nPrime, r, bit):
	t = []
	f = []
	for m in mlist:
		c, bucket = rsa_sim(m[0], d, n, nPrime, r, bit)
		if bucket:
			t.append(m)
		else:
			f.append(m)
	q_t.put(t)
	q_f.put(f)

de