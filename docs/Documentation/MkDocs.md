# Write Documentation

This documentation of JackTrip is generated with [Material](https://squidfunk.github.io/mkdocs-material/) theme
for [MkDocs](https://www.mkdocs.org/). All pages are derived from Markdown files in the subdirectory `docs`
in JackTrip's git repository. Setup and table of contents is found in a YAML file called mkdocs.yml in
the root directory.

If you only want to edit a page you can click on the pen symbol at the top of each page.

## MkDocs preview in PRs

If you submit changes to the docs as a Pull Request, the `Render docs preview` workflow will generate a static version of the documentation, including the proposed changes. It can be downloaded from GitHub Actions for checking offline (see "Checks" tab on top of the PRs page, then select `Render docs preview` and find the archive at the bottom of that page). 

When you're working on the changes locally, it might be more convenient to test changes as they're being made by running mkdocs on your system (see below).
## Run MkDocs on Your System

MkDocs and Material for MkDocs are installed from pip:
```bash
pip install mkdocs mkdocs-material mkdocs-macros-plugin
``` 

When writing documentation it is very handy to run `mkdocs serve`. This will open
a local webserver (usually at [http://127.0.0.1:8000/](http://127.0.0.1:8000/)).
If you change Markdown files of the documentation and save them, the website automatically
updates. 
