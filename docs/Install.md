# Installation
## Linux
On Linux the easiest way to install JackTrip is to use the distribution's package manager.

=== "Fedora"

    ```bash
    sudo dnf install jacktrip
    ```

=== "Debian/Ubuntu"

    ```bash
    sudo apt install jacktrip
    ```

The [GitHub releases page](https://github.com/jacktrip/jacktrip/releases) also includes a binary which should run on most Linux distributions (x64). This build is known to not look well under Wayland.

## macOS
macOS installer and application bundle are available on the [GitHub releases page](https://github.com/jacktrip/jacktrip/releases). The installer will install the JackTrip app in `/Applications`, as well as create a link to the `jacktrip` executable in `/usr/local/bin` for use in the command line.

## Windows
Windows installer and executable are available on the [GitHub releases page](https://github.com/jacktrip/jacktrip/releases). The installer will add a shortcut to your Start menu.

# Experimental builds

To keep up with the latest changes, including experimental functionality, you can access builds from the `dev` branch for Linux, macOS and Windows at [https://nightly.link/jacktrip/jacktrip/workflows/jacktrip/dev](https://nightly.link/jacktrip/jacktrip/workflows/jacktrip/dev). Please note that macOS binaries are not signed, so you need to right-click and select "Open" in order to run them.

# Build from Source
To build JackTrip yourself, follow instructions to compile for [Linux](Build/Linux.md), [MacOS](Build/Mac.md) or [Windows](Build/Windows.md).
