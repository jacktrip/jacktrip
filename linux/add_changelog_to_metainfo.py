#!/usr/bin/env python3

import sys
import yaml
from jinja2 import Template

changelog_file = sys.path[0] + "/../docs/changelog.yml"
with open(changelog_file) as f:
    file_array = yaml.load(f, Loader=yaml.FullLoader)
    releases = { "releases": file_array}

if len(sys.argv) < 3:
    exit('Not enough arguments')

template_file = sys.argv[1]
output_file = sys.argv[2]

with open(template_file) as f2:
    template = Template(f2.read())

with open(output_file, "w") as f_out:
    f_out.write(template.render(releases))

