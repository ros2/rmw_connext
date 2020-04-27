# This is a utility function to help generate consistent results from Connext rtiddsgen{_server}.
# We've seen cases where it can generate header files that do not contain the proper 'extern "C"'
# markings, which we believe is a race inside the generator itself.
# To combat this, we try up to 10 times to generate, and ensure that the header files have the
# correct markings.  Once they do, we consider it a success.

import argparse
import os
import shutil
import subprocess
import sys

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--idl-pp', required=True)
    parser.add_argument('--idl-file', required=True)
    parser.add_argument('-d', required=True)
    parser.add_argument('--max-tries', type=int, default=10)
    args = parser.parse_args()

    done = False
    count = 0
    while not done and count < args.max_tries:
        # Remove and recreate the output target directory.  This ensures that previous failed
        # attempts won't cause the next attempt to fail.
        try:
            shutil.rmtree(args.d)
        except FileNotFoundError:
            pass
        os.mkdir(args.d)

        ret = subprocess.run(args=[args.idl_pp, "-language", "C++", "-unboundedSupport", args.idl_file, "-d", args.d])
        if ret.returncode == 0:
            with open(os.path.join(args.d, args.idl_file[:-4] + 'Plugin.h'), 'r') as infp:
                for line in infp:
                    if line.startswith('extern "C" {'):
                        done = True
                        break
        count += 1

    if count == args.max_tries:
        print('Could not successfully generate Connext serialized data', file=sys.stderr)
        return 1
    return ret.returncode

if __name__ == "__main__":
    sys.exit(main())
