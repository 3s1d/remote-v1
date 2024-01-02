#!/usr/local/bin/python3
# -*- coding: utf-8 -*-

import os
import shlex
import subprocess
import sys, getopt
from time import sleep
from shutil import copy
import glob
import serial

def get_nextaddr():
	addrfile = 'next.addr'

	#create new file
	if not os.path.exists(addrfile):
		print('Creating address file: ' + addrfile)
		fp = open(addrfile, 'w')
		fp.write('0001\n')
		fp.close()

	#read next available address
	fp = open(addrfile)
	sn = int(fp.readline(), 16)
	fp.close()

	return '%04X'%sn


def inc_nextaddr():
	addrfile = 'next.addr'

	#read next available address
	fp = open(addrfile)
	sn = int(fp.readline(), 16)
	fp.close()

	#write next free addr
	fp = open(addrfile, "w")
	next_sn = sn + 1
	fp.write('%04X\n'%next_sn)
	fp.close()


def main(argv):
	print('### Remote Flasher ###')
	addr_str = get_nextaddr()
	print('Address:' + addr_str)
	high_byte = int(addr_str[:-2], 16)
	low_byte = int(addr_str[-2:], 16)
	cmd_str = '/usr/local/bin/avrdude -pt44a -cavrispmkII -Uflash:w:release/button.hex:a -Ueeprom:w:0x%02X,0x%02X:m' % (high_byte, low_byte)
	cmd_args = shlex.split(cmd_str)
	cmd_ret = subprocess.call(cmd_args)
	if cmd_ret == 0:
		inc_nextaddr()
		print('### SUCCESS ###')

if __name__ == '__main__':
	main(sys.argv[1:])