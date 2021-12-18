Changelog
=========

{% for release in releases %}
## {{ release.Version }} {% if release.Type == 'development' %}(Development){% endif %}

{% for change in release.Description %}
- {{ change }}
{% endfor %}
{% endfor %}
