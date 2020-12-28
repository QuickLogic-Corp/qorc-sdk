QORC SDK Specific Guide For Documentation
=========================================

It is assumed that the :code:`guide-generic-doc.rst` would have given a good background on the various aspects to be handled while creating documentation.

We look at the 2 categories of Github rst docs, and Sphinx/RTD rst docs.


.. contents::


Github rst Documentation
------------------------

These would be one-page READMEs mostly, with some topics having 2-3 pages.

For these, the pointers in the :code:`guide-generic-doc.rst` should be enough.

The other aspect would be to include these in the :code:`sphinx` documentation as well, refer to **How To Include Github rst files inside Sphinx docs** section below.


Sphinx/ReadTheDocs rst Documentation
------------------------------------

The bird's eye structure of our :code:`sphinx` documentation is as below, in the :code:`docs/` directory:

::
  
  docs
    ├── getting-started
    ├── guide-generic-doc.rst
    ├── guide-qorc-sdk-doc.rst
    ├── make.bat
    ├── Makefile
    ├── README.rst
    └── source/
            ├── common.rst
            ├── conf.py
            ├── Doxyfile
            ├── index.rst
            ├── ql-development-boards/
            ├── ql-eos-s3/
            ├── ql-faq/
            ├── ql-symbiflow/
            ├── ql-tinyfpga-programmer/
            ├── qorc-sdk/
            ├── qorc-setup/
            ├── requirements.txt
            └── _static

The :code:`sphinx` related files are:

- :code:`source/conf.py` which defines the configuration, and sphinx-extensions used.
- :code:`Makefile` and :code:`make.bat` are for building the documentation into final output (HTML in our case)

Our :code:`sphinx` docs use the :code:`breathe` sphinx-extension to derive the C/C++ API documentation.

The :code:`breathe` sphinx-extension uses :code:`Doxygen` to generate the actual XML database, and makes it usable in the rst files directly using its :code:`directives`.

:code:`Doxygen` configuration is defined in the :code:`source/Doxyfile`

Start at :code:`source/index.rst` which is the top-level file, indicating the landing page for the docs.

This includes the next level of rst files, which creates the documentation structure.


How To Build Locally
~~~~~~~~~~~~~~~~~~~~

Ensure that the following requirements are installed:

- Python3
  
- Doxygen

  ::
    
    sudo apt-get install doxygen

- Sphinx
  
  ::
    
    pip3 install sphinx

- Sphinx RTD Theme

  ::

    pip3 install pip3 install sphinx_rtd_theme

- Breathe

  ::

    pip3 install breathe

To build, goto :code:`docs` directory, and execute :code:`make html`

The HTML documents should be generated in the :code:`docs/build` directory, and open up the :code:`docs/build/index.html` in any browser to check the documentation.


ReadTheDocs Integration
~~~~~~~~~~~~~~~~~~~~~~~

The Integration to ReadTheDocs has 3 major parts to it :

- An account on ReadTheDocs, and adding the Github repo (qorc-sdk) into it, so RTD is aware of the docs integration.

- :code:`.readthedocs.yml` file in the root of the Github repo (qorc-sdk) which defines the RTD specific configuration, to enable auto-build of docs.

  :code:`source/requirements.txt` is an additional file, which is used in the :code:`.readthedocs.yml` to define the python modules that sphinx extensions depend on.

- Modification of the :code:`docs/source/conf.py` for running any initializations before actually running sphinx

  In our case, we use it for automatic invocation of :code:`Doxygen` before running :code:`Sphinx`.


The following links should help understand both parts of ReadTheDocs Integration:

- https://docs.readthedocs.io/en/latest/connected-accounts.html (connecting Github to ReadTheDocs)

- https://docs.readthedocs.io/en/latest/config-file/index.html (YAML configuration file)

- https://docs.readthedocs.io/en/latest/guides/specifying-dependencies.html (for requirements.txt)

- https://breathe.readthedocs.io/en/latest/readthedocs.html (for Breathe)

- https://docs.readthedocs.io/en/latest/guides/autobuild-docs-for-pull-requests.html (enable auto-build of docs on PR Merge in Github)


How To Include Github rst files inside Sphinx docs
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To include the Github rst files as is, within Sphinx docs structure, we use the :code:`include` directive.

The limitation of Sphinx is that, we cannot directly refer to any rst file which is outside the root of the sphinx tree, :code:`docs/source` in our case, from any :code:`toctree` directive.

To workaround this, while at the same time removing the need to duplicate the content within the sphinx docs tree, we create a "wrapper" rst file, within the sphinx docs, at the place where we want.

This "wrapper" rst then uses the :code:`include` directive to put the content of the required rst file (which can be anywhere, even outside the sphinx docs tree) into the "wrapper".


**Example Usage**

:code:`docs/source/qorc-sdk/qorc-sdk-qf-apps/qorc-sdk-qf-apps-qf-gwtestharness.rst`

This rst is a wrapper, which ensures we have a good hierarchy in the sphinx docs:

:code:`Index Page -> QORC SDK -> QF APPS -> QF GWTESTHARNESS`

It actually includes the content of the Github Readme for this app as is, using :

:code:`.. include:: /../../qf_apps/qf_gwtestharness/README.rst`

Note that the :code:`include` path above begins with a :code:`/` which indicates the sphinx docs tree root, which is :code:`docs/source` in our case.


How To Include Github rst files inside Sphinx docs - Including Images
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the Github rst uses images, which are by convention in the same place as the rst, then we have another step to perform.

The "wrapper" rst will refer to these images, and then while building the Sphinx docs, this will cause warnings, and the final Sphinx HTML will not have the images.

So, this requires us to copy the images used, if any, into the same place as the "wrapper" rst.

This is a duplication, but one that looks like we need to live with, until we find a workaround for it.


**Example Usage For the Caveat**

:code:`docs/source/ql-tinyfpga-programmer/ql-tinyfpga-programmer.rst`

This rst is a wrapper, which ensures we have a good hierarchy in the sphinx docs:

:code:`Index Page -> TinyFPGAProgrammer -> Main Page`

It actually includes the content of the Github Readme it as is, using :

:code:`.. include:: /../../TinyFPGA-Programmer-Application/README.rst`

Now, this actual rst uses an image with :code:`.. image:: qorc-flash-memory-map-addresses.svg`

This image is then copied and kept in the same place as the wrapper rst as well :

:code:`docs/source/ql-tinyfpga-programmer/qorc-flash-memory-map-addresses.svg`



The same technique can be used for any new Github rst doc which needs to be included into the sphinx/RTD documentation as well.

