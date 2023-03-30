#!/bin/bash

# not yet scripted/automated: updating/generating textures-*.s2z

set -e

cd `dirname $0`

srcdir=`pwd`
cd "${srcdir}/../Heroes of Newerth"
hondir=`pwd`
cd "${srcdir}/../linux/mojosetup-biarch"
mojosetupdir=`pwd`
cd "${srcdir}/.."
archivedir=`pwd`/archive
cd "${srcdir}"
installertemp=`pwd`/installer_temp
version=`grep "#define MAJOR_VERSION" "${srcdir}/Heroes of Newerth_shell/shell_common.h" | cut -f2`.`grep "#define MINOR_VERSION" "${srcdir}/Heroes of Newerth_shell/shell_common.h" | cut -f2`.`grep "#define MICRO_VERSION" "${srcdir}/Heroes of Newerth_shell/shell_common.h" | cut -f2`
hotfix_version=`grep "#define HOTFIX_VERSION" "${srcdir}/Heroes of Newerth_shell/shell_common.h" | cut -f2`
if [ "${hotfix_version}" != "0" ]; then
	version="$version"."$hotfix_version"
fi

if [ `which scons 2> /dev/null` ]; then
	SCONS=scons
else
	SCONS="../linux/scons/scons.py"
fi

paralleljobs=2
clean=0

create_installer ()
{
	rm -rf "${installertemp}"
	cd "${mojosetupdir}"
	rm -rf CMakeScripts CMakeFiles CMakeOutput.log CMakeCache.txt CMakeError.log mojosetup mojoluac Makefile *.so `find . -name "*.o"`
	
	mkdir -pv "${installertemp}"
	cd "${installertemp}"
	
	installername="HoNClient-${version}.sh"
	config="config.lua"
	
	# create the mojosetup directory skeleton
	mkdir -pv data
	mkdir -pv guis-x86
	mkdir -pv guis-x86_64
	mkdir -pv meta
	mkdir -pv meta/xdg-utils
	mkdir -pv scripts-x86
	mkdir -pv scripts-x86_64
	
	# create the hon directory skeleton
	cd data
	mkdir -pv base
	mkdir -pv game
	mkdir -pv game/maps
	mkdir -pv editor

	#mkdir -pv modelviewer
	#mkdir -pv modelviewer/maps
	
	# add the files
	cd "${installertemp}"
	cp -v "${hondir}"/hon{-x86,-x86_64,.sh} "${installertemp}"/data
	cp -v "${hondir}"/editor.sh "${installertemp}"/data
	cp -v "${hondir}"/hon_update-x86{,_64} "${installertemp}"/data
	cp -v "${hondir}"/{libk2,vid_gl2}-x86{,_64}.so "${installertemp}"/data
	cp -v "${hondir}"/game/{libgame_shared,cgame,game}-x86{,_64}.so "${installertemp}"/data/game
	cp -v "${hondir}"/game/*.s2z "${installertemp}"/data/game
	cp -v "${hondir}"/editor/cgame-x86{,_64}.so "${installertemp}"/data/editor
	cp -v "${hondir}"/editor/*.s2z "${installertemp}"/data/editor
	cp -v "${hondir}"/editor/customscenariocreation.doc "${installertemp}"/data/editor
	cp -vR "${hondir}"/libs-x86{,_64} "${installertemp}"/data
	cp -v "${hondir}"/{change_log,change_log_history,change_log_color,change_log_color_history,compat_ignore,tos}.txt "${installertemp}"/data
	cp -v "${hondir}"/{icon.png,ca-bundle.crt,pci.ids} "${installertemp}"/data
	cp -v "${hondir}"/game/maps/{caldavar,darkwoodvale,grimmscrossing,test,test_simple,tutorial,watchtower}.s2z "${installertemp}"/data/game/maps
	cp -v "${hondir}"/base/*.s2z "${installertemp}"/data/base
	cd "${installertemp}/data"
	wget http://ps1.hon.s2games.com/lac/x86-biarch/${version}/manifest.xml.zip
	unzip manifest.xml.zip
	rm manifest.xml.zip
	
	# create the config
	size=`du -sb "${installertemp}"/data | cut -f1`
	echo "local DATASIZE=$size" > "${installertemp}"/config.lua
	echo "local VERSION=\"$version\"" >> "${installertemp}"/config.lua
	cat "${srcdir}"/"${config}" >> "${installertemp}"/config.lua
	
	# build mojosetup
	cd "${mojosetupdir}"
	CMAKE_C_FLAGS=""
	CMAKE_CXX_FLAGS=""
	cmake \
		-DCMAKE_BUILD_TYPE=MinSizeRel \
		-DCMAKE_AR=/opt/toolchain/gcc-4.3.2-glibc-2.3.4/x86_64-unknown-linux-gnu/bin/x86_64-unknown-linux-gnu-ar \
		-DCMAKE_C_COMPILER=/opt/toolchain/gcc-4.3.2-glibc-2.3.4/x86_64-unknown-linux-gnu/bin/x86_64-unknown-linux-gnu-gcc \
		-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} \
		-DCMAKE_CXX_COMPILER=/opt/toolchain/gcc-4.3.2-glibc-2.3.4/x86_64-unknown-linux-gnu/bin/x86_64-unknown-linux-gnu-g++ \
		-DMOJOSETUP_ARCHIVE_TAR=FALSE \
		-DMOJOSETUP_ARCHIVE_TAR_BZ2=FALSE \
		-DMOJOSETUP_ARCHIVE_TAR_GZ=FALSE \
		-DMOJOSETUP_ARCHIVE_ZIP=TRUE \
		-DMOJOSETUP_BUILD_LUAC=TRUE \
		-DMOJOSETUP_CHECKSUM_CRC32=FALSE \
		-DMOJOSETUP_CHECKSUM_MD5=FALSE \
		-DMOJOSETUP_CHECKSUM_SHA1=FALSE \
		-DMOJOSETUP_GUI_GTKPLUS=TRUE \
		-DMOJOSETUP_GUI_GTKPLUS_STATIC=FALSE \
		-DMOJOSETUP_GUI_NCURSES=TRUE \
		-DMOJOSETUP_GUI_NCURSES_STATIC=FALSE \
		-DMOJOSETUP_GUI_STDIO=TRUE \
		-DMOJOSETUP_GUI_STDIO_STATIC=TRUE \
		-DMOJOSETUP_GUI_WWW=FALSE \
		-DMOJOSETUP_IMAGE_BMP=FALSE \
		-DMOJOSETUP_IMAGE_HDR=FALSE \
		-DMOJOSETUP_IMAGE_JPG=FALSE \
		-DMOJOSETUP_IMAGE_PND=TRUE \
		-DMOJOSETUP_IMAGE_PSD=FALSE \
		-DMOJOSETUP_IMAGE_TGA=FALSE \
		-DMOJOSETUP_INTERNAL_BZLIB=FALSE \
		-DMOJOSETUP_INTERNAL_ZLIB=FALSE \
		-DMOJOSETUP_LUA_PARSER=TRUE \
		-DMOJOSETUP_URL_FTP=FALSE \
		-DMOJOSETUP_URL_HTTP=FALSE
	make -j$paralleljobs
	
	strip mojosetup *.so
	mv mojosetup{,-x86_64}
	mv *.so "${installertemp}"/guis-x86_64
	for file in scripts/*.lua; do
		./mojoluac -s -o "${installertemp}"/scripts-x86_64/$(basename ${file})c ${file}
	done
	rm  "${installertemp}"/scripts-x86_64/config.luac
	"${mojosetupdir}"/mojoluac -s -o "${installertemp}"/scripts-x86_64/config.luac "${installertemp}"/config.lua

	rm -rf CMakeScripts CMakeFiles CMakeOutput.log CMakeCache.txt CMakeError.log mojoluac Makefile `find . -name "*.o"`
	
	# build mojosetup
	CMAKE_C_FLAGS="-m32"
	CMAKE_CXX_FLAGS="-m32"
	cmake \
		-DCMAKE_BUILD_TYPE=MinSizeRel \
		-DCMAKE_AR=/opt/toolchain/gcc-4.3.2-glibc-2.3.4/x86_64-unknown-linux-gnu/bin/x86_64-unknown-linux-gnu-ar \
		-DCMAKE_C_COMPILER=/opt/toolchain/gcc-4.3.2-glibc-2.3.4/x86_64-unknown-linux-gnu/bin/x86_64-unknown-linux-gnu-gcc \
		-DCMAKE_C_FLAGS=${CMAKE_C_FLAGS} \
		-DCMAKE_CXX_COMPILER=/opt/toolchain/gcc-4.3.2-glibc-2.3.4/x86_64-unknown-linux-gnu/bin/x86_64-unknown-linux-gnu-g++ \
		-DMOJOSETUP_ARCHIVE_TAR=FALSE \
		-DMOJOSETUP_ARCHIVE_TAR_BZ2=FALSE \
		-DMOJOSETUP_ARCHIVE_TAR_GZ=FALSE \
		-DMOJOSETUP_ARCHIVE_ZIP=TRUE \
		-DMOJOSETUP_BUILD_LUAC=TRUE \
		-DMOJOSETUP_CHECKSUM_CRC32=FALSE \
		-DMOJOSETUP_CHECKSUM_MD5=FALSE \
		-DMOJOSETUP_CHECKSUM_SHA1=FALSE \
		-DMOJOSETUP_GUI_GTKPLUS=TRUE \
		-DMOJOSETUP_GUI_GTKPLUS_STATIC=FALSE \
		-DMOJOSETUP_GUI_NCURSES=TRUE \
		-DMOJOSETUP_GUI_NCURSES_STATIC=FALSE \
		-DMOJOSETUP_GUI_STDIO=TRUE \
		-DMOJOSETUP_GUI_STDIO_STATIC=TRUE \
		-DMOJOSETUP_GUI_WWW=FALSE \
		-DMOJOSETUP_IMAGE_BMP=FALSE \
		-DMOJOSETUP_IMAGE_HDR=FALSE \
		-DMOJOSETUP_IMAGE_JPG=FALSE \
		-DMOJOSETUP_IMAGE_PND=TRUE \
		-DMOJOSETUP_IMAGE_PSD=FALSE \
		-DMOJOSETUP_IMAGE_TGA=FALSE \
		-DMOJOSETUP_INTERNAL_BZLIB=FALSE \
		-DMOJOSETUP_INTERNAL_ZLIB=FALSE \
		-DMOJOSETUP_LUA_PARSER=TRUE \
		-DMOJOSETUP_URL_FTP=FALSE \
		-DMOJOSETUP_URL_HTTP=FALSE
	make -j$paralleljobs
	
	strip mojosetup *.so
	mv mojosetup{,-x86}
	mv *.so "${installertemp}"/guis-x86
	for file in scripts/*.lua; do
		./mojoluac -s -o "${installertemp}"/scripts-x86/$(basename ${file})c ${file}
	done
	rm  "${installertemp}"/scripts-x86/config.luac
	"${mojosetupdir}"/mojoluac -s -o "${installertemp}"/scripts-x86/config.luac "${installertemp}"/config.lua
	
	X86_START=$(du -b run.sh | cut -f1)
	X86_SIZE=$(du -b mojosetup-x86 | cut -f1)
	X86_64_START=$(($X86_START+$X86_SIZE))
	X86_64_SIZE=$(du -b mojosetup-x86_64 | cut -f1)
	
	sed -e "s/X86_START=00000000/X86_START=$(printf %-8i $X86_START)/" -e "s/X86_SIZE=00000000/X86_SIZE=$(printf %-8i $X86_SIZE)/" -e "s/X86_64_START=00000000/X86_64_START=$(printf %-8i $X86_64_START)/" -e "s/X86_64_SIZE=00000000/X86_64_SIZE=$(printf %-8i $X86_64_SIZE)/" < run.sh > install.sh
	
	cat mojosetup-x86 >> install.sh
	cat mojosetup-x86_64 >> install.sh
	chmod +x install.sh
	cp -v install.sh "${installertemp}"
	
	# splash
	convert "${hondir}"/game/ui/elements/logo.tga -resize 50% -depth 8 "${installertemp}"/meta/splash.png
	
	# license
	sed G "${hondir}"/tos.txt > "${installertemp}"/meta/license.txt
	
	# xdg-utils (creates menu entries, etc)
	cp -v "${mojosetupdir}"/meta/xdg-utils/xdg-* "${installertemp}"/meta/xdg-utils/
	
	cd "${installertemp}"
	zip -Xr9 archive.zip data guis-* meta scripts-*
	cat archive.zip >> ./install.sh
	mv -v install.sh ../${installername}
	cd ..
	rm -rf "${installertemp}"
}

copy_libs () # ${1} = arch
{
	mkdir -p "${hondir}"/libs-${1}
	
	if [ "${1}" = "x86_64" ]; then
		libdir=/opt/toolchain/gcc-4.3.2-glibc-2.3.4/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/sys-root/lib64
		cp -puv ${libdir}/libfmodex64.so "${hondir}"/libs-${1}
	else
		libdir=/opt/toolchain/gcc-4.3.2-glibc-2.3.4/x86_64-unknown-linux-gnu/x86_64-unknown-linux-gnu/sys-root/lib
		cp -puv ${libdir}/libfmodex.so "${hondir}"/libs-${1}
	fi
	
	cp -puv ${libdir}/libstdc++.so.6 "${hondir}"/libs-${1}
	cp -puv ${libdir}/libgcc_s.so.1 "${hondir}"/libs-${1}
	cp -puv ${libdir}/libcurl.so.4 "${hondir}"/libs-${1}
	cp -puv ${libdir}/libfreetype.so.6 "${hondir}"/libs-${1}
	cp -puv ${libdir}/libspeex.so.1 "${hondir}"/libs-${1}
	cp -puv ${libdir}/libspeexdsp.so.1 "${hondir}"/libs-${1}
	cp -puv ${libdir}/libpng14.so.14 "${hondir}"/libs-${1}
	
	# then strip them
	strip --strip-unneeded "${hondir}"/libs-${1}/*.so*
}

archive_src ()
{
	rm -rf "${archivedir}/hon-${version}"

	mkdir -pv "${archivedir}/hon-${version}"
	cd "${archivedir}/hon-${version}"
	
	mkdir -pv src
	rsync -pvrC --include='*.c' --include='*.cpp' --include='*.h' --include='*.vcproj' --include='*.inl' --include='SConstruct' --include='*SConscript' --include='build.sh' -f 'H,! */' --exclude 'game**' --exclude 'MaxExporter' --exclude 'vid_d3d**' --exclude 'vid_gl' --exclude='updater**' --prune-empty-dirs "${srcdir}"/ src/
	
	mkdir -pv bin
	mkdir -pv bin/game
	mkdir -pv bin/editor
	mkdir -pv bin/modelviewer
	cp -v "${hondir}"/hon-x86* "${hondir}"/hon_update-x86*  "${hondir}"/libk2-x86*.so* "${hondir}"/vid_gl2-x86*.so* bin
	cp -v "${hondir}"/game/libgame_shared-x86*.so* "${hondir}"/game/game-x86*.so* "${hondir}"/game/cgame-x86*.so* bin/game
	cp -v "${hondir}"/editor/cgame-x86*.so* bin/editor
	cp -v "${hondir}"/modelviewer/cgame-x86*.so* bin/modelviewer
	
	cd ..
	tar cjvf hon-${version}.tar.bz2 hon-${version}
	
	rm -rf "${archivedir}/hon-${version}"
}

scons_targets=""

doupdatecvs=0
doupdatelibs=0
doinstaller=0
doarchive=0
dohelp=0

if [ "x"$1 = "x" ]; then dohelp=1; fi

while [ "x"$1 != "x" ]; do
	case $1 in
		"everything"			)	doupdatecvs=1; doresources=1; doupdatelibs=1; scons_targets="$scons_targets all-release";;
		"installer"				)	doinstaller=1;;
		"update-libs"			)	doupdatelibs=1;;
		"update-cvs"			)	doupdatecvs=1;;
		"archive"				)	doarchive=1;;
		"-h" | "-?" | "--help"	)	dohelp=1;;
		"-c"					)	clean=1;;
		"-j"					)	paralleljobs="$2"; shift;;
		"--prompt"				)	ftpprompt=1;;
		"--test"				)	dotest=1;;
		*						)	if [ ${1:0:2} = "-j" ]; then paralleljobs=${1:2:${#1}-2}; else scons_targets="$scons_targets $1"; fi;;
	esac
	shift
done

if [ $dohelp = 1 ]; then
	echo "Targets:"
	echo "  all: builds release and debug 32 and 64 bit builds"
	echo "  all-x86_64: builds release and debug 64 bit builds"
	echo "  all-x86: builds release and debug 32 bit builds"
	echo "  all-debug: builds debug 32 and 64 bit builds"
	echo "  all-release: builds release 32 and 64 bit builds"
	echo "  all-debug-x86_64: builds debug 64 bit build"
	echo "  all-debug-x86: builds debug 32 bit builds"
	echo "  all-release-x86_64: builds release 64 bit build"
	echo "  all-release-x86: builds release 32 bit build"
	echo "  everything:  updates, builds, puts on updater, creates new installers"
	echo "all can be replace with one of the following to just build that directory:"
	echo "  k2, shell, hon_shared, hon_server, hon_client, editor, modelviewer,"
	echo "  K2_Updater, vid_gl1, vid_gl2"
	echo "  installer: builds biarch installer"
	echo "  -c              clean"
	echo "  -j <jobs>       number of parallel jobs (default 2)"
	exit 0
fi

if [ $doupdatecvs = 1 ]; then
	cd "${srcdir}"
	cvs update -dP
	cd "${hondir}"
	cvs update -dP
fi

if [ "x$scons_targets" != "x" ]; then
	if [ $clean = 1 ]; then
		scons_targets="$scons_targets -c"
	fi
	cd "${srcdir}"
	$SCONS $scons_targets -j $paralleljobs
fi

if [ $doupdatelibs = 1 ]; then
	copy_libs x86
	copy_libs x86_64
fi

if [ $doinstaller = 1 ]; then
	create_installer
fi

if [ $doarchive != 0 ]; then
	archive_src
fi
