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
	v1 = 0.12
	v2 = 2.57
	vrms0_1 = 19200.0
	vrms0_2 = 413000.0
	r = v1/v2
	offset = (r**2*vrms0_2**2-vrms0_1**2)/(1-r**2)/32768.
	print "for v1 = ", v1, " and v2 = ", v2
	print "vrms0_1 is: ", vrms0_1, " and vrms0_2 is: ", vrms0_2
	print "IRMS offset is: ", offset


if __name__ == '__main__':
	main()

