# libpurple
Fork of Pidgin repository with just libpurple

To build libpurple run the following commands
```
do-release-upgrade
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install gettext libjson-glib-dev  libgtk-3-dev libgirepository1.0-dev libgtk-4-dev mercurial libgumbo-dev libcmark-dev help2man valac libxml2-dev libgupnp-1.2-dev gupnp-igd-1.0 libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev liblua5.4-dev lua-lgi liblua5.3-dev libperl-dev libglib-object-introspection-perl python-gi-dev libsasl2-dev libcanberra-dev libidn11-dev meson python3-pip libgadu-dev gmime-3.0 libkf5wallet-dev gettext libxml2-dev cmake libsecret-1-dev gi-docgen libavahi-glib-dev libavahi-client-dev sassc libadwaita-1-dev libmeanwhile-dev qttools5-dev-tools
sudo pip install jinja2 toml typogrify
git clone --branch subproj https://github.com/cacticouncil/libpurple
cd libpurple
meson build
cd build
ln -s .. libpurple
meson compile
```
