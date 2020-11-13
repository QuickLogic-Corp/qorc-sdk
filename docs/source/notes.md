# Overview of tasks for documentation of qorc-sdk

There are 4 parts to the documentation to be handled.

1. docs created specifically as a source for the `readthedocs` structure

2. docs which are already in qorc-sdk, currently as markdown files, for example the readme of the `qf_bootloader` app - these should still be available as part of the github repo so that users can look at the readme of various components, and at the same time, we should be able to re-use the content for the `readthedocs` structure, which makes it easier to maintain

3. docs which are not part of the qorc-sdk
   here, we have 2 categories:
   1. repos like the `TinyFPGAProgrammer` which do not have their own separate `readthedocs` structure, and hence can be assimilated as part of the qorc-sdk docs.
   For these, I think the best way is to have a `docs-build-setup-repo.sh` which will basically clone those repos inside the qorc-sdk tree, and we link those docs as part of the main qorc-sdk `readthedocs` structure

   2. repos like the QuickLogic FPGA Toolchain, which do have their own separate `readthedocs` structure and hence cannot be directly assimilated. We will link to the public `readthedocs` structure using **intersphinx** and keep the repeated content to a bare minimum.
   
4. API documentation
   we would prefer to use doxygen to document the source code, and then use a doxygen-sphinx bridge to derive `readthedocs` content, rather than using doxygen to generate HTML directly.
   The options I see we have for this are:
   1. doxygen + doxyrest - simple to setup and use, opensource
      https://github.com/vovkos/doxyrest

   2. doxygen + breathe(optional) - a bit more complex first time, but produces very good results suitable for `readthedocs`
   https://breathe.readthedocs.io/en/latest/

   3.  doxygen + breathe + exhale
   This is advanced, but doesnt give great output unless the whole project is neatly documented, hence cannot be used as of now.
   https://exhale.readthedocs.io/en/latest/

   
# Plan for the documentation

1. Create the docs needed for the basic stuff - setup of environment, toolchain and boards, as this will be new content (already available as part of the getting-started)

2. Move the current markdown docs already in github to rst format, preserving the content, or augmenting the content where there seems to be more information needed

3. Add more readme rst for things which do not have documentation such as `qf_advancedfpga` for example

4. Decide which repos can be included as a sub repo (or git submodule itself) of the qorc-sdk (such as the `TinyFPGAProgrammer`) and link their docs similar to the qorc-sdk docs spread across directories

5. Link the docs of independent repos (currently we see only the FPGA Toolchain) using intersphinx so qorc-sdk documentation can refer to those easily

6. Setup doxygen for API documentation - and then use one of the options for linking to sphinx to generate a separate `API Guide` section in the qorc-sdk `readthedocs` structure
   Here, I don't see doxygen configuration in the qorc-sdk yet, we can pick up things from the previous open-platform code where possible, otherwise create and document the components one by one.
   As this would be a large effort, we can break it down component wise and prioritize- such as HAL first, followed up the Libraries in order and so on.

# Notes on specific implementation progress

1. `qorc-sdk` documentation has the `docs/source` which is the root of the sphinx docs.

2. we `..include::` the rst files from various parts of the `qorc-sdk` so that we can use them as base docs for github, as well as for part of the sphinx docs.  
   One important point to take note here, is that if any external files are used in such rst files, then we need to ensure that it is available to both the main rst itself, as well as when referenced from the *wrapper* rst file in the sphinx docs tree.

3. we currently (re)use the docs from the following external repos:
   1. https://github.com/QuickLogic-Corp/TinyFPGA-Programmer-Application
   2. https://github.com/QuickLogic-Corp/quick-feather-dev-board
   3. https://github.com/QuickLogic-Corp/quicklogic-fpga-toolchain

   For this, we clone these 3 repos in the qorc-sdk tree, and then build the sphinx docs.

   We have wrapper rst files which include the specific rst files from their existing path relative to qorc-sdk.

   For an example, refer to: `docs/source/ql-development-boards/quickfeather/development-board-quickfeather.rst` which wraps the actual rst file from the `quick-feather-dev-board` repo cloned into the qorc-sdk tree: `quick-feather-dev-board/README.rst`

4. `TinyFPGA-Programmer-Application` specific notes

5. `quick-feather-dev-board` specific notes
   1. the main rst file refers to certain images in the repo's own `img/` directory.
      These images must be copied to the sphinx docs source dir as well.

      Hence, we copy the `img/` dir in the `quick-feather-dev-board` repo to `docs/source/ql-development-boards/quickfeather/img` before a sphinx build is invoked.

6. `quicklogic-fpga-toolchain` specific notes
   The FPGA Toolchain has its own sphinx generated docs hosted at `readthedocs`.
   
   So, we set it into the intersphinx mapping, so we can in the future, directly refer to any object in the indpendent documentation there.

   Just note that the intersphinx mapping path should point to a path where there is a sphinx inventory file, in our case it is at: `https://quicklogic-fpga-tool-docs.readthedocs.io/en/latest/objects.inv` and hence the path in the intersphinx mapping is `https://quicklogic-fpga-tool-docs.readthedocs.io/en/latest`

# Further Reading for rST and SPHINX
1. pandoc

   This is the most useful tool while converting from mardown to rst.  
   One of the few things it cannot automatically do is convert markdown tables to rst tables 100%.
   For tables, it is recommended to code by hand, the 2 quickest options are:
   1. Use https://www.tablesgenerator.com/text_tables to generate, check the `Use reStructuredText syntax` option while generating.
      This can also be used to generate markdown tables, rather than manual input, use the https://www.tablesgenerator.com/markdown_tables link.
   2. for rst, we can also use a csv file, and include this into the rst file. However, note that this cannot be used with github rendered rst files, but can be used with sphinx. We use the option 1, unless we have a really complex table structure.
   
2. rst cheatsheet

   https://thomas-cokelaer.info/tutorials/sphinx/rest_syntax.html
   
   http://openalea.gforge.inria.fr/doc/openalea/doc/_build/html/source/sphinx/rest_syntax.html
   
3. a collection of stuff useful for sphinx/rst

   https://github.com/yoloseem/awesome-sphinxdoc
   
   https://www.ericholscher.com/blog/2016/jul/1/sphinx-and-rtd-for-writers/
   
   
   
