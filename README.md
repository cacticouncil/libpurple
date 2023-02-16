# libpurple
Fork of Pidgin repository with just libpurple

To build libpurple run the following commands
```
do-release-upgrade
sudo apt-get install build-essential libgirepository1.0-dev python3-pip libgtk-4-bin libgtk-4-common valac libgtk-4-dev libgtk-4-doc mercurial libgumbo-dev libcmark-dev help2man libjson-glib-1.0-0 libjson-glib-dev liblua5.1-0-dev lua5.4 lua5.3 liblua5.3-dev lua-lgi liblua5.4-dev libglib-object-introspection-perl python-gi-dev libsasl2-dev libcanberra-dev libgtk-3-dev libsecret-1-dev libnice-dev
pip3 install --upgrade meson
git clone https://github.com/cacticouncil/libpurple
cd libpurple
meson build
cd build
ln -s .. libpurple
meson compile
```
