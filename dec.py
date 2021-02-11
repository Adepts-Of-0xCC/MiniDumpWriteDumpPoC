import sys


with open(sys.argv[1], 'rb') as f:
    f_contents = f.read()
    with open(sys.argv[2], 'wb') as w:
            key = int(sys.argv[3], 16)
            decr = bytes(b ^  key for b in f_contents)
            w.write(decr)
