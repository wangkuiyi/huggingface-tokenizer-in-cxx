import sys

if len(sys.argv) != 4:
    sys.exit(
        f"Usage: {sys.argv[0]} tokenization_output1 tokenization_output2 tokenization_input"
    )

o1 = open(sys.argv[1])
o2 = open(sys.argv[2])
i0 = open(sys.argv[3])

while True:
    l1 = o1.readline()
    l2 = o2.readline()
    li = i0.readline()

    if not l1 or not l2 or not li:
        break

    if l1 != l2:
        print(l1[:-1])
        print(l2)
        print(li)
