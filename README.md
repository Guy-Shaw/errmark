# errmark

### run a program and mark its stderr

`errmark` runs some program and marks stderr output
to distinguish it from stdout
when both stdout and stderr are connected to the
same file (usually a pseudo-terminal).

It does this without modifying the program
or doing any "normal" I/O redirection.

`errmark` can also make a copy of stderr.

### The Problem

Sometimes it can be hard to distinguish which messages
are coming from stdout and which from stderr.

Let's say both stdout and stderr are open to the
pseudo-terminal associated with a terminal emulator.
It is a very common case for
commands running under an interactive shell.
If stderr is redirected to some other file,
in order to separate out stderr from stdout,
that causes problems of its own.
The new behavior is not the same as when
both were open to the same device.
Also, behavior changes when stdout or stderr
are no longer open to a pseudo-terminal,
whether it be the same device or not.
For example, buffering is different.
If the buffering were the only problem,
then it could be fixed using `stdbuf` or
a 'pty' program.  But, then stdout and stderr
are still not mixed the way they are when
both are open to the same pseudo-terminal.

## Implementation

Errmark intercepts `write()` system calls,
using `ptrace(2)`,
and surrounds writes to fd/2 with some marker,
such as bold or ANSI color escape sequences.

To be more precise, `errmark` injects character sequences
whenever there are `write()` system calls,
making a transition between between fd/1 and fd/2.

### Why Use Ptrace?

One might think there would be an easier way
to solve this problem.
Three other possibilities are:

1) redirect stdout and stderr to two different pseudo-terminals,
then run the child program and, as the master side
of both pty's, monitor what is written to stout and stderr.

2) use `LD_PRELOAD` to substitute calls to `write()`.

3) write a driver for a new kind of pty-like device
that is capable of serializing the output of `write()`
system calls, and can distinguish writes
coming from different file descriptors.

The problem with the two master pty approach
is that the ordering of writes is not preserved.
It almost works, but a mix of writes to stdout
and stderr can arrive in a different order.

This is not just due to buffering of I/O
in a process, such as is done in stdio.
Even in the case of a series of raw `write()` system calls,
only the order of writes to the same device
are guaranteed to be visible in the order of the `write()`
system calls; nothing is promised about the order of
data visible between writes to different devices.

The problem with the `LD_PRELOAD` method is that
glibc can call `write()` as a result of lower-level
stdio functions, and those do not get linked
to a dynamically loaded/linked override `write()`
function.  Only explicit calls to `write()`
are overridden.


## Portability

Anything that uses ptrace() cannot expect to be portable
across machine architectures or data models  or operating systems.
The code is riddled with direct get and set of registers.
It is targeted to `x86` or `x86_64` architecture, only.
It runs on Linux only.
No attempt has been made to refrain from using GCC extensions.


## Cleanliness

`errmark` runs Valgrind-clean.  It is clean with respect to
`gcc -Wall -Wextra`.  I have not used any brand of lint on it,
also partly because of GCC extensions.

`errmark` compiles clean using gcc, g++, clang, and clang++.

## Coding style

The coding style used for `ush` can most briefly be described as Sun cstyle, but with certain changes.
They are:
    + tabs instead of spaces
    + 4 character indent, instead of 8
    + no cuddling of braces

## No limits

`errmark` has no arbitrary limits.  There is no limit imposed
by `errmark` due to fixed size buffers or arrays, etc.
Line buffers grow as needed.  Arrays for things like
arguments grow as needed.

## Examples

### fuser

```
    errmark --color=bright-green fuser .
```

This example shows how fuser intermixes stdout and stderr.

### tar

```
    errmark --color=red  --copy=/tmp/tar.err tar cvf /tmp/archive.tar .
    if [[ -s /tmp/tar.err ]]
    then
        sed 20q /tmp/tar.err
    fi
```

If tar encounters errors,
such as ownership / permission problems,
those error messages could be buried among hundreds or thousands
of lines of filenames.

`errmark` can highlight the error messages,
and, at the same time copy just the contents of stderr
to a file, making it easier to examine just the errors.

## History

The first version of `errmark` was written in 1995, by Guy Shaw.
It ran only on Solaris.

## License

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


-- Guy Shaw

   gshaw@acm.org

