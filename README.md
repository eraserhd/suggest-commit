# suggest-commit

Suggest a commit message from a diff.

## Installing

Clone this repository, then:

    make && sudo make install

Once in the repository where you would like commits suggested:

    git diff |suggest-commit

If you would like git to prepopulate suggested commit messages, then copy
the `prepare-commit-msg` shell script from this directory to
`.git/hooks/prepare-commit-msg` and make sure it is executable.

And you are done!
