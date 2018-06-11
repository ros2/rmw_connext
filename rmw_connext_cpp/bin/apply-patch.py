import argparse
import re

_hdr_pat = re.compile("^@@ -(\d+),?(\d+)? \+(\d+),?(\d+)? @@.*$")


def apply_patch(s, patch, revert=False):
    """
    Apply unified diff patch to string s to recover newer string.

    If revert is True, treat s as the newer string, recover older string.
    """
    s = s.splitlines(True)
    p = patch.splitlines(True)
    t = ''
    i = sl = 0
    (midx, sign) = (1, '+') if not revert else (3, '-')
    while i < len(p) and p[i].startswith(('---', '+++')):
        i += 1  # skip header lines
    while i < len(p):
        m = _hdr_pat.match(p[i])
        if not m:
            raise Exception('Cannot process diff in line ' + str(i))
        i += 1
        l = int(m.group(midx)) - 1 + (m.group(midx + 1) == '0')
        t += ''.join(s[sl:l])
        sl = l
        while i < len(p) and p[i][0] != '@':
            if i + 1 < len(p) and p[i + 1][0] == '\\':
                line = p[i][:-1]
                i += 2
            else:
                line = p[i]
                i += 1
            if len(line) > 0:
                if line[0] == sign or line[0] == ' ':
                    t += line[1:]
                sl += (line[0] != sign)
    t += ''.join(s[sl:])
    return t


parser = argparse.ArgumentParser()
parser.add_argument('--input', nargs='+')
parser.add_argument('--patch', nargs='+')
parser.add_argument('--out', nargs='+')
args = parser.parse_args()
for i, p, o in zip(args.input, args.patch, args.out):
    with open(i, 'r') as h:
        content_in = h.read()
    with open(p, 'r') as h:
        content_patch = h.read()
    try:
        content_out = apply_patch(content_in, content_patch)
    except Exception:
        print(i, p, o)
        raise
    with open(o, 'w') as h:
        h.write(content_out)
