import sys

sys.path.append('.')

from pyhcl import *
import numpy as np


def main():
	ary = np.ones([100, 100, 100], dtype=np.uint8)
	data = hcl_compress(ary, 'JpegSequence', {'quality': 100})
	# print data.tostring()
	with open('test.out', 'wb') as f:
		f.write(data.tostring())
	ret = hcl_decompress(data)
	# print ret


if __name__ == '__main__':
	main()