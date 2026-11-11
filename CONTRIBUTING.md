# Contributing to Picolibc

We welcome contributions of all forms, including patches to code and
documentation, issue reports, or just kind words.

## Getting Current Source Code

Picolibc sources are maintained in git and hosted in two places.

 1. [keithp.com](https://keithp.com/cgit/picolibc.git/). This is the
    'canonical' source location.

 2. [github](https://github.com/picolibc/picolibc). This should
    exactly mirror the code at keithp.com. I've placed it here to
    make contributing to Picolibc easier for people familiar with github.

You can create a local copy of the repository with git clone:

	$ git clone git://keithp.com/git/picolibc.git

or

	$ git clone https://github.com/picolibc/picolibc.git

## Meson

Picolibc uses the [meson](https://mesonbuild.com/) build tool. You'll
need to make sure that's installed before you start trying to build
the software.

## Building Picolibc from Source

As Picolibc is designed to be used for embedded systems, getting that
configured correctly can be a challenge. Learn more about that in the
[build](doc/build.md) page.

## Policy on the use of Large Language Models (LLMs) and AI tooling

We expect everyone contributing to picolibc to fully own their
contribution, be able to reason about it, be able to explain why
things were done a particular way and act as the full owner of that
code. AI tools are treated the same as traditional tooling like sed,
awk or coccinelle.

For the purpose of this project, AI tools CANNOT be treated as author,
co-author or be credited in any way that would suggest any ownership
over the contribution.

Contributions are expected to be idiomatic to the project and should
be written to ease human review. Anything coming from an automated
tool must be carefully edited by the submitter to make it clear,
correct and concise. This includes commit messages, which should be as
brief as practical. People review every submission to the project,
please be respectful of their time by taking at least as much time
when preparing them.

Unguided use of those tools or the inability to prove understanding of
the code contributed will result in a loss of trust in that
contributor by project maintainers which can then lead to exclusion
from any further contribution to the project.

As with any other submissions, authors are responsible for doing due
diligence and ensuring their submissions are compatible with the
project’s licenses as documented in COPYING.picolibc

## Formatting Source Code

Picolibc uses clang-format with a custom template. Format checking is
included the CI process, and as any submission with CI errors will not
be considered, you'll need to make sure your contributions are
correctly formatted.

The CI process produces a patch indicating how the code would need to
be changed to pass. This can be useful if you do not have access to a
compatible version of clang-format.

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
[Picolibc Issue Tracker](https://github.com/picolibc/picolibc/issues)

## Mailing List

Picolibc has a mailing list, hosted at keithp.com. It's largely unused
because we're unable to keep the subscription process automated.
This is a public list, with public archives. Participants are expected
to abide by the [Picolibc Code of Conduct](CODE_OF_CONDUCT.md).

## Code of Conduct

Picolibc uses the [Contributor Covenant](https://www.contributor-covenant.org/),
which you'll find in the source tree as [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).
Please help make picolibc a kind and welcoming environment by following
those rules.
