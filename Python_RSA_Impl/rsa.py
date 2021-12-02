import math, copy, random

keys = {}
config = {}

def init():
	""" NB! Keys are hardcoded for repeatability
		Public key: (e, n)
		Private key: d
	"""
	p = 72921395523034486567525736371230370633973787029153043254895253767587177948354404505015843041682240089 			# prvate
	q = 27028138044587582353904781804159356623304801440906159575368078211171173680092726609842044176970728203			# private
	e = (1<<16)+1# public
	# p = 97
	# q = 103
	# e = 31
	n = p*q 					# public
	tot_n = (p-1)*(q-1)			
	d = ModInverse(e, tot_n)	#private
	config = {'p': p,
			 'q': q,
			 'tot_n': tot_n,
			 'block_size': 83 }
	keys = {'n': n,
			'e': e,
			'd': d }

	return config, keys

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

def MongomeryProduct(a, b, nprime, r, n):
	""" Montgomery product.""