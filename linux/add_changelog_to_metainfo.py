#!/usr/bin/env python3

import yaml
from jinja2 import Template

changelog_file = "../docs/changelog.yml"
with open(changelog_file) as f:
    file_array = yaml.load(f, Loader=yaml.FullLoader)
    releases = { "releases": file_array}

template_file = "org.jacktrip.JackTrip.metainfo.xml.in"
with open(template_file) as f2:
    template = Template(f2.read())

print(template.render(releases))
