# Linenoise - NonBlocking

A minimal, zero-config, BSD licensed, readline replacement used in Redis,
MongoDB, Android and many other projects.

* Single and multi line editing mode with the usual key bindings implemented.
* History handling.
* Completion.
* Hints (suggestions at the right of the prompt as you type).
* Multiplexing mode, with prompt hiding/restoring for asynchronous output.
* About ~850 lines (comments and spaces excluded) of BSD license source code.
* Only uses a subset of VT100 escapes (ANSI.SYS compatible).

## Can a line editing library be 20k lines of code?

Line editing with some support for history is a really important feature for command line utilities. Instead of retyping almost the same stuff again and again it's just much better to hit the up arrow and edit on syntax errors, or in order to try a slightly different command. But apparently code dealing with terminals is some sort of Black Magic: readline is 30k lines of code, libedit 20k. Is it reasonable to link small utilities to huge libraries just to get a minimal support for line editing?

So what usually happens is either:

 * Large programs with configure scripts disabling line editing if readline is not present in the system, or not supporting it at all since readline is GPL licensed and libedit (the BSD clone) is not as known and available as readline is (real world example of this problem: Tclsh).
 * Smaller programs not using a configure script not supporting line editing at all (A problem we had with `redis-cli`, for instance).

The result is a pollution of binaries without line editing support.

So I spent more or less two hours doing a reality check resulting in this little library: is it *really* needed for a line editing library to be 20k lines of code? Apparently not, it is possibe to get a very small, zero configuration, trivial to embed library, that solves the problem. Smaller programs will just include this, supporting line editing out of the box. Larger programs may use this little library or just checking with configure if readline/libedit is available and resorting to Linenoise if not.

## Terminals, in 2010.

Apparently almost every terminal you can happen to use today has some kind of support for basic VT100 escape sequences. So I tried to write a lib using just very basic VT100 features. The resulting library appears to work everywhere I tried to use it, and now can work even on ANSI.SYS compatible terminals, since no
VT220 specific sequences are used anymore.

The library is currently about 850 lines of code. In order to use it in your project just look at the *example.c* file in the source distribution, it is pretty straightforward. The library supports both a blocking mode and a multiplexing mode, see the API documentation later in this file for more information.

Linenoise is BSD-licensed code, so you can use both in free software and commercial software.

## Tested with...

 * ...

## Let's push this forward!

Patches should be provided in the respect of Linenoise sensibility for small
easy to understand code.

Sorry but I made huge API changes that are not able to go upstream likely.

# The API

Synchronous API is deprecated.

## Nonblocking API

TBD.
