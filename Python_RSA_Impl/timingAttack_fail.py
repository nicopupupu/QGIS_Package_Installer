import math, random
from pprint import pprint




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


def num2bits(num):
	bits = []
	k = math.floor(math.log2(num)) + 1
	for i in list(reversed(list(range(0,k)))):
		bits.append(num >> i & 1)
	return bits


def MongomeryProduct(a, b, nprime, r, n):
	""" Montgomery product."""
	t = a * b
	m = t * nprime % r
	u = (t + m*n)//r
	return (True, u-n) if (u >= n) else (False, u)


def ModExp(M, d, n, focus_bit):
	""" Montgomery binary exponentiation"""
	bit_list = num2bits(d)
	if(focus_bit >= len(bit_list)):
		print("error. Focus bit is too large!")
	step4 = False
	if n%2 != 1:
		raise ValueError("N must be odd!")
	(r, nprime) = nPrime(n)
	M_bar = (M * r) % n
	x_bar = 1 * r % n

	for i in range(len(bit_list)):
		e_i = bit_list[i]
		# print(i, e_i)
		_, x_bar = MongomeryProduct(x_bar, x_bar, nprime, r, n)
		if e_i == 1:
			_, x_bar = MongomeryProduct(M_bar, x_bar, nprime, r, n)
			if(i == focus_bit):
				step4 = _
	_, x = MongomeryProduct(x_bar, 1, nprime, r, n)
	return (step4, x)


def ModExpTrueCount(M, d, n):
	"""