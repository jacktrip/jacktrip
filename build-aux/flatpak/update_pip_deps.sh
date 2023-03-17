#!/bin/bash
# https://github.com/flatpak/flatpak-builder-tools/tree/master/pip
flatpak-pip-generator --runtime='org.freedesktop.Sdk//22.08' --requirements-file='requirements.txt' --output pypi-dependencies