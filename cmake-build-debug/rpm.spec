Buildroot:      /home/ashraf/geodns/cmake-build-debug/_CPack_Packages/Linux/RPM/ipv-geodns-1.0.0-1.x86_64
Summary:        IPVision GeoDNS
Name:           ipv-geodns
Version:        1.0.0
Release:        1
License:        GPL
Group:          Development/Tools/Other
Vendor:         My Company
Prefix:         /usr/local/bin

Requires:       libmaxminddb >= 1.2
Requires:       libmaxminddb-devel >= 1.2
Requires:       sqlite >= 3.7.17
Requires:       sqlite-devel >= 3.7.17

%define _rpmdir /home/ashraf/geodns/cmake-build-debug/_CPack_Packages/Linux/RPM
%define _rpmfilename ipv-geodns-1.0.0-1.x86_64.rpm
%define _unpackaged_files_terminate_build 0
%define _topdir /home/ashraf/geodns/cmake-build-debug/_CPack_Packages/Linux/RPM

%description
Cool project solving the problems of many colleagues.

# This is a shortcutted spec file generated by CMake RPM generator
# we skip _install step because CPack does that for us.
# We do only save CPack installed tree in _prepr
# and then restore it in build.
%prep
mv $RPM_BUILD_ROOT /home/ashraf/geodns/cmake-build-debug/_CPack_Packages/Linux/RPM/tmpBBroot

%install
if [ -e $RPM_BUILD_ROOT ];
then
  rm -Rf $RPM_BUILD_ROOT
fi
mv "/home/ashraf/geodns/cmake-build-debug/_CPack_Packages/Linux/RPM/tmpBBroot" $RPM_BUILD_ROOT

%files
%defattr(-,root,root,-)
/usr/local/bin/*
/etc/ipv-geodns/*
/etc/systemd/system//*

%changelog
* Tue Jan 29 2013 John Explainer <john@mycompany.com> 1.0.1-3
- use correct maintainer address
* Tue Jan 29 2013 John Explainer <john@mycompany.com> 1.0.1-2
- fix something about the package
* Thu Jan 24 2013 John Explainer <john@mycompany.com> 1.0.1-1
- important bugfixes
* Fri Nov 16 2012 John Explainer <john@mycompany.com> 1.0.0-1
- first release
