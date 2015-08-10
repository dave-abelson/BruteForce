import itertools
import time
def cksum(v):
    a = 0
    b = 0
    for i in range(len(v)):
        a = (a + v[i]) % 0xff
        b = (b + a) % 0xff
    return (b << 8) | a

def subsetsum(array, num):
    if num < 0:
        return
    if len(array) == 0:
        if num == 0:
            yield []
        return
    for solution in subsetsum(array[1:], num):
        yield solution
    for solution in subsetsum(array[1:], num - array[0]):
        yield [array[0]] + solution

field = range(65, 91) + range(97, 123)

def evens():
    for y in subsetsum(field, 765):
        if len(y) != 8:
            continue
        for i in itertools.permutations(y):
            r = 0
            for a in range(len(i)):
                r += (len(i) - a) * i[a]
            if r % 255 != 0:
                continue
            yield i
            
def fours(t):
    for x in field:
        if x > t:
            break
        for y in range(max(65, t - x - 122 - 122), 123):
            if not y in field:
                continue
            if x + y > t:
                break
            for z in range(max(65, t - x - y - 122), 123):
                if not z in field:
                    continue
                if x + y + z > t - 65:
                    break
                yield (x, y, z, t - (x + y + z))

t = time.time()
count = 0
for x in evens():
    for l in fours(875 - x[0] - x[1] - x[2] - x[3]):
        if not l[3] in field:
            continue
        c = [x[0], l[0], x[1], l[1], x[2], l[2], x[3], l[3]]
        if cksum(c) == 0xd06e:
            for r in fours(778 - x[4] - x[5] - x[6] - x[7]):
                if not r[3] in field:
                    continue
                d = [x[4], r[0], x[5], r[1], x[6], r[2], x[7], r[3]]
                if cksum(d) == 0xf00d:
                    count += 1
                    print ''.join(chr(a) for a in c + d), count, time.time() - t
            for r in fours(523 - x[4] - x[5] - x[6]):
                if not r[3] in field:
                    continue
                d = [x[4], r[0], x[5], r[1], x[6], r[2], x[7], r[3]]
                if cksum(d) == 0xf00d:
                    count += 1
                    print ''.join(chr(a) for a in c + d), count, time.time() - t
    for l in fours(620 - x[0] - x[1] - x[2] - x[3]):
        if not l[3] in field:
            continue
        c = [x[0], l[0], x[1], l[1], x[2], l[2], x[3]]
        if cksum(c) == 0xd06e:
            for r in fours(778 - x[4] - x[5] - x[6] - x[7]):
                if not r[3] in field:
                    continue
                d = [x[4], r[0], x[5], r[1], x[6], r[2], x[7], r[3]]
                if cksum(d) == 0xf00d:
                    count += 1
                    print ''.join(chr(a) for a in c + d), count, time.time() - t
            for r in fours(523 - x[4] - x[5] - x[6]):
                if not r[3] in field:
                    continue
                d = [x[4], r[0], x[5], r[1], x[6], r[2], x[7], r[3]]
                if cksum(d) == 0xf00d:
                    count += 1
                    print ''.join(chr(a) for a in c + d), count, time.time() - t
    
                         
