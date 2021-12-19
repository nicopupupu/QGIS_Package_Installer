RSA Timing attack implementation
================================

We implement a timing attack on the RSA algorithm, to recover the private key.
We then implement RSA using Montgomery Powering Ladder as a countermeasure, and show that the timing attack now is ineffective.

Building and running
-------------------

To create and sign a number of messages, build and run the C program:
```
# Requires cmake, make, and C++11 compiler support.
$ cd build
$ cmake .. && make
$ ./csv <p> <q> <e> <number of messages> # for example 97 103 31 10000

```

After a while you will see a file called data.csv in the same folder. 

To run the att