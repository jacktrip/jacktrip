# JackTrip is a multi-machine audio system used for network music performance over the Internet.

See LICENSE.md for license information.

JackTrip requires that Qt6 is installed on your machine.

For Fedora or RedHat:

```
dnf install -y qt6-qtbase qt6-qtbase-common qt6-qtbase-gui qt6-qtsvg qt6-qtwebsockets qt6-qtwebengine qt6-qtwebchannel qt6-qt5compat
```

For Debian or Ubuntu:

```
apt install -y libqt6core6 libqt6gui6 libqt6network6 libqt6widgets6 libqt6qml6 libqt6qmlcore6 libqt6quick6 libqt6quickcontrols2-6 libqt6svg6  libqt6webchannel6 libqt6webengine6-data libqt6webenginecore6 libqt6webenginecore6-bin libqt6webenginequick6 libqt6websockets6 libqt6shadertools6 qt6-qpa-plugins qml6-module-qtquick-controls qml6-module-qtqml-workerscript qml6-module-qtquick-templates qml6-module-qtquick-layouts qml6-module-qt5compat-graphicaleffects qml6-module-qtwebchannel qml6-module-qtwebengine qml6-module-qtquick-window
```

To install JackTrip as a Linux desktop application:

```
sudo cp jacktrip /usr/local/bin
mkdir -p $HOME/.local/share/applications $HOME/.local/share/icons/hicolor/scalable/apps $HOME/.local/share/icons/hicolor/48x48/apps
cp org.jacktrip.JackTrip.svg $HOME/.local/share/icons/hicolor/scalable/apps/
cp org.jacktrip.JackTrip.png $HOME/.local/share/icons/hicolor/48x48/apps/
desktop-file-install --dir=$HOME/.local/share/applications org.jacktrip.JackTrip.desktop
update-desktop-database $HOME/.local/share/applications
```

To install the manual page for JackTrip:

```
sudo mkdir -p /usr/local/share/man/man1
sudo cp jacktrip.1.gz /usr/local/share/man/man1
```

When using jacktrip with the JACK Audio Connection Kit (or Pipewire), ensure that your user account has permission to schedule realtime processes.
`ulimit -r` should return a value greater than 40.

Further information and instructions are available on https://jacktrip.github.io/jacktrip/. 

Please report any security concerns to vulnerabilities@jacktrip.org
