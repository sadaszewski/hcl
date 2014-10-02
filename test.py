import sys

sys.path.append('.')

from pyhcl import *
import numpy as np


def main():
	ary = np.ones([10, 10, 10], dtype=np.uint8)
	data = hcl_compress(ary, 'JpegSequence', {'quality': 75})
	print data.tostring()
	with open('test.out', 'wb') as f:
		f.write(data.tostring())
	ret = hcl_decompress(data)
	print ret


if __name__ == '__main__':
	main()