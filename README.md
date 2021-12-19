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

To run the attack, copy this into `Attack/output/some_folder`, and run 

```
$ python RSAAttack.py output/<some_folder> <duration to split on>
# for example output/2ms_sleep_33bit_key 4307361
```

This runs the attack on the dataset you generated, with `duration` as the difference in average time between each set used decide whether a bit is 0 or 1. A good approach is