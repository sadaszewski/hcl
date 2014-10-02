hcl
===

Hyperdimensional Compression Library - imaging data compression in more than 2 dimensions


Example
===

The 100x100x100 JPEG-compressed volume weighs 45960 bytes compared to 1000000 uncompressed.

```python
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
```
