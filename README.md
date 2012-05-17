# fsyncbomb 

This tool generates a 100% write I/O workload which simulates a write-heavy
database workload.  It streams a given pattern to a sequence of files, followed
by an optional fsync(3C) after writing each file.


# Usage

    fsyncbomb [-cfqt] [-n num] [-s size]

    -c: only create files, don't continuously write data
    -f: fsync(3C) after every file write
    -n: number of data files (default 10)
    -q: quiet mode, suppress all output
    -s: size of each data file (default 10m)
    -t: files truncated before writing

If -c is specified, the tool will write each file and exit.  If -c is not
specified, the tool will run forever and loop over each file in order.


# Status

This tool has been tested on SmartOS (an illumos distribution provided by
Joyent).  It should work unmodified on other illumos distributions.  In
addition, given the simple nature of this program, it should work on other Unix
platforms, as well as GNU/Linux.

This tool is a work in progress, and there is some duplicated code which can be
refactored.


# See Also

This tool was derived from iopattern (http://github.com/pijewski/iopattern).  At
some point, I may recombine the funcionality of both tools into one single tool.
The arcstress tool (http://github.com/pijewski/arcstress) also shares some
common elements.
