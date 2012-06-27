#!/usr/bin/env python
# encoding: utf-8
"""
calcIRMS.py

calculates the IRMS based on the decimal count output by the ADE

Created by amadduri on 2010-11-18.
Copyright (c) 2010 UC Berkeley. All rights reserved.
"""

import sys
import os


def main():
	iRmsCounts = 21342.0
	scaling = .0215 #V/A
	rmsRange = 1868467.
	gain = 2.
	iRmsAmps = iRmsCounts/gain*0.5/rmsRange/scaling
	print "iRMSCounts is: ", iRmsCounts
	print "IRMS (AMPS) is: ", iRmsAmps


if __name__ == '__main__':
	main()

