import sslparser

data = (
b"\x16\x03\x01\x00\xbd\x01\x00\x00\xb9\x03\x03\x8a\x80\x22\x0f\x8d" +
b"\x60\x13\x99\x8b\x4b\xfa\x96\xba\x7a\xeb\x81\x60\x80\xe4\xc7\x9e" +
b"\xd0\x4e\x18\x4e\xc5\xd5\x74\x17\x23\xb1\xa1\x00\x00\x38\xc0\x2c" +
b"\xc0\x30\x00\x9f\xcc\xa9\xcc\xa8\xcc\xaa\xc0\x2b\xc0\x2f\x00\x9e" +
b"\xc0\x24\xc0\x28\x00\x6b\xc0\x23\xc0\x27\x00\x67\xc0\x0a\xc0\x14" +
b"\x00\x39\xc0\x09\xc0\x13\x00\x33\x00\x9d\x00\x9c\x00\x3d\x00\x3c" +
b"\x00\x35\x00\x2f\x00\xff\x01\x00\x00\x58\x00\x00\x00\x0e\x00\x0c" +
b"\x00\x00\x09\x6c\x6f\x63\x61\x6c\x68\x6f\x73\x74\x00\x0b\x00\x04" +
b"\x03\x00\x01\x02\x00\x0a\x00\x0a\x00\x08\x00\x1d\x00\x17\x00\x19" +
b"\x00\x18\x00\x23\x00\x00\x00\x16\x00\x00\x00\x17\x00\x00\x00\x0d" +
b"\x00\x20\x00\x1e\x06\x01\x06\x02\x06\x03\x05\x01\x05\x02\x05\x03" +
b"\x04\x01\x04\x02\x04\x03\x03\x01\x03\x02\x03\x03\x02\x01\x02\x02" +
b"\x02\x03")

s=sslparser.Ssl()
print(s.host())
print(s.feed(data, 1))
print(s.host())

print("----")

s=sslparser.Ssl()
for datum in data:
  datum = bytes(chr(datum), encoding='latin1')
  print(s.feed(datum, 0))
  host = s.host()
  if host:
    print(host)
    break
