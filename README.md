# Qt ProcessTree ipc
A launcher process experiment for Qt to force launched instances as child instances enforcing a single parent process.

The goal is to investigate wether this helps out with the Dock Icon wildgrow on MacOS for a multi-instance application.
See https://github.com/musescore/MuseScore/issues/12647 for context.

[!Program Start Sequence](MS-ProcessTree.svg)

As this projects builds on/tests for/borrows from MuseScore, it is under the same GPL3 license.
