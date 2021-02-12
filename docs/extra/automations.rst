Automations
===========

Continuous Integration
----------------------

This project has an automated continuous integration system.
Whenever a commit to a remote branch is pushed, a corresponding CircleCI
job is started. Normally, this only involves verifying if the build
is successful; however, if a tag is added, an additional job will be
started to create the tag's documentation page.

Automated Documentation
-----------------------

The documentation pages are generated from the source code using Doxygen,
Breathe and Sphinx. It is possible to create additional pages by inserting them
in docs/extra. The first part of the index page is contained in
docs/index_head.rst. The latter part is generated from the source code.
After a couple minutes, the new documentation will be available on `ReadTheDocs<https://pandos.readthedocs.io/>`.