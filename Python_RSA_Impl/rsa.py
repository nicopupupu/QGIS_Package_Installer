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
	""" Montgomery product."""
	t = a * b
	m = t * nprime % r
	u = (t + m*n)//r
	return u-n if (u >= n) else u

def nPrime(n):
	""" Calculates r^{-1} and n' as used in Montgomery exponentiation"""
	# n is a k-bit number.
	# r should be 2^k
	k = math.floor(math.log2(int(n))) + 1
	r = int(math.pow(2, k))
	rInverse = ModInverse(r, n)
	nPrime = (r * rInverse -1) // n
	return (r, nPrime)

def num2bits(num):
	bits = []
	k = math.floor(math.log2(num)) + 1
	for i in list(reversed(list(range(0,k)))):
		bits.append(num >> i & 1)
	return bits


def ModExp(M, d, n):
	""" Montgomery binary exponentiation"""
	if n%2 != 1:
		raise ValueError("N must be odd!")
	(r, nprime) = nPrime(n)
	M_bar = (M * r) % n
	x_bar = 1 * r % n
	bit_list = num2bits(d)
	for e_i in bit_list:
		x_bar = MongomeryProduct(x_bar, x_bar, nprime, r, n)
		if e_i == 1:
			x_bar = MongomeryProduct(M_bar, x_bar, nprime, r, n)
	x = MongomeryProduct(x_bar, 1, nprime, r, n)
	return x

def encrypt(message):
	""" Encrypt a message using the public key in getKeys()""" 
	e, n = keys['e'], keys['n']
	number_list = string2num(message)
	blocks = num2blocks(number_list, config['block_size'])
	return [ModExp(block, e, n) for block in blocks]

def decrypt(ciphertext):
	""" Decrypt a ciphertext using the private key in getKeys()"""
	d, n = keys['d'], keys['n']
	blocks = [ModExp(block, d, n) for block in ciphertext]
	number_list = blocks2num(blocks, config['block_size'])
	return num2string(number_list)
	# return ModExp(int(ciphertext), int(d), int(n))


def sign(message):
	return (message, encrypt(message))

"""
Below functions are used for converting between String message and the corresponding 
numbers we work with in RSA
"""
def string2