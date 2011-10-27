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
	i1 = 0.5914
	i2 = 4.4/119.9
	irms0_1 = 593667.0
	irms0_2 = 39325.0
	r = i1/i2
	offset = 1/32768.0*(i1**2*irms0_2**2-i2**2*irms0_1**2)/(i2**2-i1**2)
	slope = (i1-i2)/(irms0_1-irms0_2)*1000
	print "for i1 = ", i1, " and i2 = ", i2
	print "irms0_1 is: ", irms0_1, " and irms0_2 is: ", irms0_2
	print "IRMS offset is: ", offset, "counts"
	print "IRMS slope is: ", slope, "mAmps/count"


if __name__ == '__main__':
	main()

