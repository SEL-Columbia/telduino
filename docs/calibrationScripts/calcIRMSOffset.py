#!/usr/bin/env python
# encoding: utf-8
"""
calcIRMSOffset.py

calculates the IRMS offset based on 2 actual IRMS values that are input into the current meter

Created by amadduri on 2010-11-18.
Copyright (c) 2010 UC Berkeley. All rights reserved.
"""

import sys
import os


def main():
	i1 = 0.210
	i2 = .138
	irms0_1 = 94350.0
	irms0_2 = 62585.0
	r = i1/i2
	offset = (r**2*irms0_2**2-irms0_1**2)/(1-r**2)/32768.
	print "for i1 = ", i1, " and i2 = ", i2
	print "irms0_1 is: ", irms0_1, " and irms0_2 is: ", irms0_2
	print "IRMS offset is: ", offset


if __name__ == '__main__':
	main()

