%define version 1.4
%define release 1

Name: nct
Summary: Tetris-like game extended by colors
Version: %{version}
Release: %{release}
Copyright: GNU GPL
Group: Amusements/Games
Source: ftp://ftp.yars.free.net/pub/software/unix/games/tetris/nct-%{version}.tar.gz
BuildRoot: /tmp/nct-root

%description
Tetris-like game, extended by colors. Light colors can be replaced by heavy
ones. Number of colors can be chosen, in case of one color we get classic game.


%prep
%setup

%build
./configure --prefix=/usr --bindir=/usr/games --localstatedir=/var
make CFLAGS="$RPM_OPT_FLAGS"
strip nct

%install
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/var/games
touch $RPM_BUILD_ROOT/var/games/nct.score

%clean
rm -rf $RPM_BUILD_ROOT

%files
%attr(2755, root, games) /usr/games/nct
%attr(664, root, games) %config /var/games/nct.score

%attr(-, root, root) %doc README
