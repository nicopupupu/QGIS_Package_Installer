
//  Simple RSA Implementation for demonstrating a timing attack.
//  * Do not use this in production! *
//  Created by Arve Nygård on 05/05/15.
//  Copyright (c) 2015 Arve Nygård. All rights reserved.
//
#include "rsa.h"
using namespace std;

/*
 * Sets which exponentiation method is to be used.
 *
 * Choose between MODEXP, MODEXP_SLEEP, and POWERLADDER.
 * Default is MODEXP.
 */
void Rsa::setExpFunc(const ExpType expType){
    switch (expType) {
        case POWERLADDER:
            ef = &Rsa::PoweringLadder;
            break;
        case MODEXP:
            ef = &Rsa::ModExp;
            break;
        case MODEXP_SLEEP:
            ef = &Rsa::ModExpSleep;
            break;
        default:
            ef = &Rsa::ModExp;
            break;
    }
}

/*
 * Binary exponentiation of M raised to the power of d (mod n)
 *
 * Default algorithm.
 * Suceptible to timing attacks.
 */
num Rsa::ModExp(const num &M, const num &d, const num &n){
    if (n%2 != 1) {
        cout << "Warning! Exponentiation failed. Modulus must be odd!";
        return 0;
    }
    num r, nprime;
    nPrime(n, r, nprime);
    num M_bar = (M * r) % n;
    num x_bar = r%n;

    long k = numBits(d) - 1; // Loop over bit indices. [0, k-1]
    for (; k >= 0 ; k--) {
        x_bar = MontgomeryProduct(x_bar, x_bar, nprime, r, n);
        if (d.GetBit(k)){
            x_bar = MontgomeryProduct(M_bar, x_bar, nprime, r, n);
        }
    }
    return MontgomeryProduct(x_bar, 1, nprime, r, n);
}

/*
 * Binary exponentiation of M raised to the power of d (mod n).
 *
 * Even more suceptible to timing attacks. (purposely).
 * Used to simulate a slow device.
 * Uses MontgomeryProductSleep.
 */
num Rsa::ModExpSleep(const num &M, const num &d, const num &n){
    if (n%2 != 1) {
        cout << "Warning! Exponentiation failed. Modulus must be odd!";
        return 0;
    }
    num r, nprime;
    nPrime(n, r, nprime);
    num M_bar = (M * r) % n;
    num x_bar = r%n;

    long k = numBits(d) - 1; // Loop over bit indices. [0, k-1]
    for (; k >= 0 ; k--) {
        x_bar = MontgomeryProductSleep(x_bar, x_bar, nprime, r, n);
        if (d.GetBit(k) == 1){
            x_bar = MontgomeryProductSleep(M_bar, x_bar, nprime, r, n);
        }
    }
    return MontgomeryProductSleep(x_bar, 1, nprime, r, n);
}

/*
 * Binary exponentiation of M raised to the power of d (mod n),
 * Using Montgomery Powering ladder
 */
num Rsa::PoweringLadder(const num &message, const num &exponent, const num &modulus){
    num R0 = 1, R1 = message;
    long t = Rsa::numBits(exponent);

    for (long i = t-1; i>=0; i--) {
        if(!exponent.GetBit(i)){
            // The bit is 0