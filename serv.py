import socket
import sys

if __name__ == '__main__':
	s = socket.socket()
	s.bind(('0.0.0.0', int(sys.argv[1])))
	s.listen(5)

	while True:
		print("Ready...")
		c, addr = s.accept()
		print("[!] Connection from::", addr)
		print("Receiving...")
		l = c.recv(65540)
		buff = b''
		try:

			while (l):
				buff += l
				l = c.recv(65540)
		except:
			print("[!] Socket incorrectly closed... Attempting to save anyway")

		finally:

			with open('woot.dmp', 'wb') as f:
				curr_offset = 0
				curr_size = 0

				while curr_offset < len(buff):
					wr_offset = int.from_bytes(buff[curr_offset:curr_offset+4], "little")
					curr_offset += 4
					wr_size = int.from_bytes(buff[curr_offset:curr_offset+4], "little")
					curr_offset += 4
					#print("[i] Offset::", wr_offset, "-- Size::", wr_size)

					f.seek(wr_offset)
					f.write(buff[curr_offset:curr_offset+wr_size])
					curr_offset += wr_size

		print("Done!")
