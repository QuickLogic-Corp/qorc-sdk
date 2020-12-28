# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'QORC-SDK'
copyright = '2020, QuickLogic Corp'
author = 'QuickLogic Corp'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'sphinx.ext.todo',
    'sphinx.ext.intersphinx',
    'breathe',
#    'exhale'
]

# todo extension stuff
todo_include_todos=True

# intersphinx extenstion stuff
intersphinx_mapping = {'ql-symbiflow': ('https://quicklogic-fpga-tool-docs.readthedocs.io/en/latest/', None)}

# breathe extension stuff
breathe_projects = {"qorc-sdk" : "../doxybuild/xml"}
breathe_default_project = "qorc-sdk"

# exhale extension stuff
#exhale_args = {
#    # These arguments are required
#    "containmentFolder":     "./qorc-sdk-api",
#    "rootFileName":          "qorc-sdk-api.rst",
#    "rootFileTitle":         "QORC SDK API",
#    "doxygenStripFromPath":  "..",
#    # Suggested optional arguments
#    "createTreeView":        True,
#    # TIP: if using the sphinx-bootstrap-theme, you need
#    # "treeViewIsBootstrap": True,
#    "exhaleExecutesDoxygen": True,
#    "exhaleUseDoxyfile":    True
#}

# Tell sphinx what the primary language being documented is.
#primary_domain = 'c'

# Tell sphinx what the pygments highlight language should be.
#highlight_language = 'c'

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']
html_logo = '_static/qorc_whitebg.png'
html_theme_options = {
    'logo_only': True,
    'display_version': False,
}

# warning : a bit of ugly code, with if and else doing the same thing!!
import subprocess, os

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

if read_the_docs_build:

    subprocess.call('doxygen', shell=True)

else:
    # we do the same in the local build too so that we only have to run make html.
    subprocess.call('doxygen', shell=True)
