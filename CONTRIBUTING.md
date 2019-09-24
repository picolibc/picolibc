# Contributing to Picolibc

We welcome contributions of all forms, including patches to code and
documentation, issue reports, or just kind words.

## Getting Current Source Code

Snek sources are maintained in git and hosted in two places.

 1. [keithp.com](https://keithp.com/cgit/picolibc.git/). This is the
    'canonical' source location.

 2. [github](https://github.com/keith-packard/picolibc). This should
    exactly mirror the code at keithp.com. I've placed it here to
    make contributing to Picolibc easier for people familiar with github.

You can create a local copy of the repository with git clone:

	$ git clone git://keithp.com/git/picolibc.git

or

	$ git clone https://github.com/keith-packard/picolibc.git

## Meson

Picolibc uses the [meson](https://mesonbuild.com/) build tool. You'll
need to make sure that's installed before you start trying to build
the software.

## Building Picolibc from Source

As Picolibc is designed to be used for embedded systems, getting that
configured correctly can be a challenge. Learn more about that in the
[build](doc/build.md) page.

## Patch Submission

You can submit patches in a couple of ways:

 1. Mail to the picolibc list (see below). This can be tricky as it
    requires a friendly email system, and you'll have to subscribe to
    the mailing list before it will let you post anything
    (sigh). Here's a simple example sending the latest patch in your
    local repository to the list:

	$ git send-email --to picolibc@keithp.com HEAD^

 2. Generate a pull-request in github.

    1. Fork the picolibc project into your own github account
    2. Push patches to that repository
    3. While viewing your repository, click on the 'New pull request'
       button and follow the instructions there.

## Issue Tracking

We're using the issue tracker on Github for now; if you have issues,
please submit them to the
[Picolibc Issue Tracker](https://github.com/keith-packard/picolibc/issues)

## Mailing List

Picolibc has a mailing list, hosted at keithp.com. You can
[subscribe here](https://keithp.com/mailman/listinfo/picolibc).
This is a public list, with public archives. Participants are expected
to abide by the [Picolibc Code of Conduct](CODE_OF_CONDUCT.md).

## Code of Conduct

Snek uses the [Contributor Covenant](https://www.contributor-covenant.org/),
which you'll find in the source tree as [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
Please help make Snek a kind and welcoming environment by following
those rules.
