# JackTrip is a multi-machine audio system used for network music performance over the Internet.

See LICENSE.md for license information.

To install JackTrip as a Linux desktop application:

```
sudo cp jacktrip /usr/local/bin
mkdir -p $HOME/.local/share/applications $HOME/.local/share/icons/hicolor/scalable/apps $HOME/.local/share/icons/hicolor/48x48/apps
cp org.jacktrip.JackTrip.svg $HOME/.local/share/icons/hicolor/scalable/apps/
cp org.jacktrip.JackTrip.png $HOME/.local/share/icons/hicolor/48x48/apps/
desktop-file-install --dir=$HOME/.local/share/applications org.jacktrip.JackTrip.desktop
update-desktop-database $HOME/.local/share/applications
```

When using jacktrip without the Jack Audio Toolkit (or Pipewire), ensure that your user account has permission to schedule realtime processes.
`ulimit -r` should return a value greater than 40.

Further information and instructions are available on https://jacktrip.github.io/jacktrip/. 

Please report any security concerns to vulnerabilities@jacktrip.org
