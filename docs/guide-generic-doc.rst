Generic Documentation Guide
===========================

A few pointers and best practices put together to make writing documentation quick and painless.

For reStructuredText documentation, we will deal with 2 major scenarios:

- How to write documentation in rst for use in Github (in place of md)

- How to write documentation in rst for use in Sphinx or ReadTheDocs deployment

While looking at both, we will also see how to write the Github oriented doc to also serve as doc for Sphinx documentation, to minimize repetition of same content in different places, reducing maintenance required to only one place.

For other documentation related stuff, we will look at good tools to use, such as for flowcharts, diagrams, waveforms etc.

.. contents::

Terminology
-----------

It can be confusing to read about many different tools/standards while reading about documentation in general, and rst in particular.

This will attempt to simplify the jargon that we may come across and provide a broad understanding of rst and its related tools.

| reStructuredText is a plain-text format (specification, more accurately) for marking up documents.
| This means that there are specific ways to mark parts of a document to mean things, such as to indicate a "Page Title" or "Section Header" or "List" etc.
| This information can be then converted to a visual format such as HTML/LaTeX/PDF etc. which will render the content according to the markup.

| The standard tool (processor, more accurately) to convert a rst document into one of the visual formats such as HTML/LaTeX/PDF etc. is :code:`docutils`.
| :code:`docutils` can take a rst file and output an equivalent HTML/other.
| For our current description, we are interested in the HTML output only, as this what we see on Github or other places.

| To generate a coherent documentation set using a set of multiple (interlinked) rst files, we use :code:`sphinx` which is built on top of :code:`docutils`.
| So, :code:`sphinx` is more of a document generator.

| ReadTheDocs (aka RTD) is a hosting service, a website for serving documentation, which can build and host :code:`sphinx` documentation.
| RTD is the the defacto standard hosting for most documentation today, so using :code:`sphinx` *almost* always implies using RTD as well.


Generic rst Guide
-----------------

As seen above, there are (generally) 2 usage scenarios for rst documentation.

| One is the basic single page rst documentation, which is for use as a standalone doc for a specific use case.
| This is how we think of the Github README.

| The other is to use multiple rst documents, which together create one coherent set of (interlinked) documentation.
| This is where we use :code:`sphinx` and we deal with the documentation of a software project together.

| It is important to note this as :code:`sphinx` offers many additional features (directives, theming) etc. which are not available when writing a Github README.
| Also note that Github uses its own (unknown) parser for rst and does not support everything that :code:`docutils` does, some for security reasons, others for unknown reasons.
| Remembering this will help, as we generally "google" that we want to do something specific in rst and then find that it does not work when used in a Github README, probably because it was meant to be used with :code:`sphinx` for example.
| This caveat will save a lot of grief to be aware of while searching for specific syntax of RST.

The links below can serve as an introduction to rst, docutils and sphinx:

1. https://docutils.sourceforge.io/docs/user/rst/quickstart.html

2. https://docutils.sourceforge.io/docs/user/rst/quickref.html

3. https://www.sphinx-doc.org/en/master/usage/reStructuredText/basics.html

4. https://docutils.sourceforge.io/rst.html

5. https://docutils.sourceforge.io/docs/ref/rst/directives.html


Github rst Documentation
------------------------

It is recommended to write the documentation that is visible in Github, for say, a Library component or Application etc. in the reStructuredText (rst) format rather than the Markdown (MD) format(which is the Github default).

The main reason for this is so that this documentation can then be included as is in the Sphinx based documentation for the whole project, rather than have a separate copy with the same information, which would need maintenance at 2 places.

Also, the capapbilities of the Markdown format are matched by the reStructuredText format, and in addition, the reStructuredText format is actually standardised, so will work across any hosting medium in the same way, not just Github.

The Markdown format used in Github is a "flavor" and is referred to as Github Flavored Markdown (GFM) and the syntax/semantics varies between Github and other hosting media.

While writing doc in rst, the following points would be useful:

- | Structure the docs with a specific hierarchy of headings, subheadings etc.
  | In general, we use formatting like below :
  
  ::
    
    PAGE HEADER
    ===========

    SECTION HEADER
    --------------

    SUBSECTION HEADER
    ~~~~~~~~~~~~~~~~~

    SUB-SUBSECTION HEADER
    ^^^^^^^^^^^^^^^^^^^^^

    SUB-SUB-SUBSECTION HEADER
    #########################

  So that we have a common structure to follow, as coding conventions do for code.

  If we are actually using sub-sub-subsections or further, then it would probably mean that we are trying to cover too much in one page, and it would be a good idea to split it across multiple pages.

- Use a :code:`contents` directive to make an automatic table of contents for the page, especially for longer pages.

  ::
    
    .. contents::

  is itself enough to create a table of contents automatically using the headers in the page!
  
  This is also used in the current document.

- Use inline code markup to make it simpler to indicate a command in the explanation

  For example, the following rst :

  ::
    
    Example of inline code to refer to :code:`bash` or :code:`sh` commands is preferred

  is rendered as:

  Example of inline code to refer to :code:`bash` or :code:`sh` commands is preferred

- Use bullet points and numbered lists liberally to make the doc easier to follow.

- Use Line Separators (or Transitions as per rst) liberally to generate a horizontal line for clear separation.

Useful Quick Reference Links:

- https://github.com/DevDungeon/reStructuredText-Documentation-Reference/blob/master/README.rst

- https://gist.github.com/silverrain/4155073/62dc91f95f5e4fddfe85227cb58ce7a9639661f9#file-readme-rst



Sphinx/ReadTheDocs rst Documentation
------------------------------------

While writing or adding documentation to the existing documentation set, we can use :code:`directive` to include existing rst files from the Github README docs into the content.

The best way to understand how :code:`sphinx` works would be to use the official documentation, and try out an example :

https://www.sphinx-doc.org/en/master/

Once the Sphinx docs are ready, the next step would be to host them on ReadTheDocs :

https://docs.readthedocs.io/en/stable/intro/import-guide.html

By convention, the sphinx document project for a particular software project is usually located in a :code:`docs` directory.

The major components of a :code:`sphinx` project would be :

- :code:`source` directory where all the rst files are present

- | :code:`conf.py` configuration script in the :code:`docs` directory, to set the various options for the sphinx project
  | The :code:`conf.py` is also where the :code:`sphinx extensions` are defined, for more flexibility.
  | For example, we make use of :code:`intersphinx` and :code:`breathe` extensions currently.

To enable RTD hosting, we have additional changes to ensure automatic setup of the RTD build environment to satisfy extra requirements (such as sphinx extensions).
This is reflected in a few places :

- :code:`.readthedocs.yml` in the repo root, which lets RTD know the sphinx doc structure, location of the :code:`conf.py`, sphinx version, python version, sphinx dependencies etc. to enable autobuild of documentation.

- :code:`conf.py` additions, which can be used to execute initialization commands (it is a python script after all)

- :code:`requirements.txt` which lets RTD know which python modules (sphinx extensions are python modules too) are required before the documentation can be built.


Integrating Doxygen Generated API With Sphinx
---------------------------------------------

For C/C++ code, the most reliable method to automatically generate API documentation is Doxygen.

Doxygen configuration can be setup as usual, the official manual is the best place to start reading :

https://www.doxygen.nl/manual/starting.html

Integrating Doxygen generated API with Sphinx means that we can use a rst :code:`directive` to have sphinx generate rst documentation from the Doxygen API data.

This uses a sphinx extension :code:`breathe` to link the Doxygen generated API to be used directly in rst files.

Breathe setup and configuration is covered pretty well in the official doc:

https://breathe.readthedocs.io/en/latest/quickstart.html

Note that Breathe uses the Doxygen XML output to create rst documentation, so the Doxygen configuration can be set to only enable XML output and disable all other formats.

Another example of this combination can be seen in the VTR documentation as well:

https://docs.verilogtorouting.org/en/latest/dev/c_api_doc/

Details specific to how we use all these tools in the qorc-sdk project can be found in the more targeted :code:`guide-qorc-sdk-doc.rst`.

That document assumes familiarity with rst, sphinx, RTD, Doxygen, Breathe, which we attempt to cover in this more generic guide document.


Converting Markdown to reStructuredText
---------------------------------------

A good tool to convert Github READMEs in Markdown to reStructuredText is pandoc.

https://pandoc.org/

On Ubuntu, it can be installed from the apt repository.

To convert from Github Flavored Markdown to reStructuredText, use :

::

  pandoc <MARKDOWN_FILE.md> -f gfm -t rst -o <RST_FILE.rst>

The conversion may result in broken tables, as markdown tables are not guaranteed to be rst compatible.

If so, it is time to create the table by hand, refer to the "Tables In Documentation" section.


Diagrams In Documentation
-------------------------

This is one of the areas where having a flexible, easy to use tool saves a lot of grief and time.

The recommendation is to use diagrams.net (formerly wire.io), as it is by far the easiest to use tool to quickly create diagrams of a good quality.
Also it does not require any login, and can be linked to any cloud storage of choice.

The easiest way is to just use the webapp : 

https://app.diagrams.net/

The webapp runs entirely in the browser, and nothing goes to any storage on any cloud service unless explicitly chosen.

| Once we have a diagram done, we can save it to the local machine, or to one of the cloud storage options.
| The default format of a diagram is :code:`xml`.
| The :code:`xml` can then be again uploaded into the webapp and edited.
| We can export the diagram into :code:`png` or :code:`svg` and used in the documentation as fit.
| :code:`svg` is preferred for the higher quality.

| The most interesting aspect of this is that while exporting as :code:`png` or :code:`svg` we can select to save a copy of the diagram.
| This means that the :code:`xml` content is stored embedded within the exported image itself, and there is no need to keep a separate :code:`xml` lurking around.
| Then, the exported image can directly be opened in the webapp, edited, and again exported back - so the image is also the diagram source, and fits perfectly into version control, just like code.

This is the preferred workflow with this tool.

- initially, open webapp, create diagram, export as :code:`svg` choosing to save a copy of the diagram within it.
- to edit, open webapp, open the :code:`svg`, edit, export again to :code:`svg`


Tables in Documentation
-----------------------

Tables are one of the pain points in writing documentation - getting the formatting right is a nightmare.
rst documentation allows us to write neat tables, and reference for it is below:

https://thomas-cokelaer.info/tutorials/sphinx/rest_syntax.html#tables

| If the table is complex enough, then writing the rst table is also pretty tedious.
| A better WYSIWYG way is to use the online tool for visually creating the table, and getting the rst code equivalent:

https://www.tablesgenerator.com/text_tables

Ensure to check the :code:`Use reStructuredText syntax` option and generate the rst which can then be copy-pasted into the rst doc.

This is the recommended way for native rst tables in the documentation.

An alternative method (which can be colored/themed - this is not possible in rst tables) is to create a table in diagrams.net, the same as any other diagram.

The table creation/entry/colors are easy to do, and exporting this as :code:`svg` creates a great quality table.

This is the recommended way for more complex tables or where visual impact is important.


Waveforms In Documentation
--------------------------

Waveforms are generally obtained from simulations or actual hardware.

Most common formats are the :code:`vcd` from gtkwave or from one of the Logic Analyzers such as :code:`saleae`

These would be converted to :code:`png` or :code:`svg` and included in the rst document.

For cases, where we would like to indicate a shortened, or cusomtized way, or a *specification* waveform, such as ideal case or an overview, it is recommended to use :code:`wavedrom` :

https://github.com/wavedrom/wavedrom

The easiest way to use it is the online editor at :

https://wavedrom.com/editor.html

Once we have the target waveform as desired, save the json file which describes the waveform, and export the waveform in :code:`png` or preferably :code:`svg`.

The exported image can be included in the documentation.

The json file is required to later edit the waveform as needed.

A quick tutorial showing multiple features of wavedrom :

https://wavedrom.com/tutorial.html