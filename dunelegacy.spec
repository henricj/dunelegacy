Summary:	A Dune II clone
Summary(pl.UTF-8):	Zaktualizowany klon gry Dune2
Name:		dunelegacy
Version:	0.96.4
Release:	0.1
License:	GPL v2+
Group:		X11/Applications/Games/Strategy
Source0:	http://www.myway.de/richieland/%{name}-%{version}-src.tar.bz2
URL:		http://dunelegacy.sourceforge.net/
BuildRequires:	SDL2_mixer-devel
BuildRequires:	SDL2-devel
BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:	libstdc++-devel
BuildRequires:	pkgconfig
BuildRequires:	desktop-file-utils
BuildRoot:	%{tmpdir}/%{name}-%{version}-root-%(id -u -n)

%description
Lead one of three interplanetary houses, Atreides, Harkonnen or Ordos,
in an attempt to harvest the largest amount of spice from the sand
dunes. Exchange your spice stockpiles for credits through refinement
and build an army capable of thwarting attempts of the other houses to
stop your harvesting!

Dune Legacy is an effort by a handful of developers to revitalize the
first-ever real-time strategy game. The original game was the basis
for the hugely successful Command and Conquer series, and the gameplay
has been replicated an extended to a wide variety of storylines and
series.

NOTE: Original Dune 2 game files are needed.

%description -l pl.UTF-8
Poprowadź jedną z trzech międzyplanetarnych rodzin, Atrydów,
Harkonnenów lub Ordosów, w wyścigu w zebraniu jak największej
ilości przyprawy z pustynnych wydm. Wymień zapasy przyprawy na
kredyty w procesie udoskonalania i stwórz armię zdolną powstrzymać
próby innych rodzin w zmuszeniu cię do zaprzestania zbierania
przyprawy!

Dune Legacy jest podjętą przez grupę utalentowanych programistów
próbą ożywienia pierwszej strategii czasu rzeczywistego. Gra była
wzorem dla odnoszącej olbrzymie sukcesy serii Command and Conquer, a
styl gry został powielony w dużej ilości innych gier.

UWAGA: Potrzebne są pliki wchodzące w skład Dune 2.

%prep
%setup -q -n %{name}-%{version}

%build
%{__aclocal}
%{__autoconf}
%{__automake}
%configure
%{__make}

%install
rm -rf $RPM_BUILD_ROOT
desktop-file-install --dir ${RPM_BUILD_ROOT}%{_datadir}/applications %{name}.desktop
install -D -p -m 0644 %{name}-128x128.png ${RPM_BUILD_ROOT}%{_datadir}/pixmaps/%{name}.png

%{__make} install \
  DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(644,root,root,755)
%doc README ToDo.txt
%attr(755,root,root) %{_bindir}/%{name}
%dir %{_datadir}/%{name}
%{_datadir}/%{name}/*
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.png
%{_mandir}/man6/dunelegacy.6*

%define date	%(echo `LC_ALL="C" date +"%a %b %d %Y"`)
%changelog
* Fri Jul 22 2011 Richard Schaller
- This spec-file is based on the http://cvs.pld-linux.org/cgi-bin/cvsweb/packages/dunelegacy/dunelegacy.spec?rev=1.8.2.1
