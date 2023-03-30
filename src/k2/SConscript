Import(['getCPPFiles', 'env', 'project', 'build_type', 'arch'])

# precompiled header
env['GchSh'] = env.GchSh(target = '../k2_common.h.gch/'+build_type+'-'+arch, source = 'k2_common.h')[0]

# libs to link against
env.Append(LIBS = ['dl', 'rt', 'z', 'png14', 'jpeg', 'gif', 'freetype', 'xml2', 'curl', 'm', 'X11', 'ncurses', 'speex', 'speexdsp'])
if arch == 'x86':
	env.Append(LIBS=['fmodex'])
elif arch == 'x86_64':
	env.Append(LIBS=['fmodex64'])
if build_type == 'debug':
	env.Append(LIBS=['SpeedTreeRT_debug'])
else:
	env.Append(LIBS=['SpeedTreeRT'])

# cpp defines
env.Append(CPPDEFINES = ['K2_DLL', 'K2_EXPORTS'])

# runtime lib path
env.Append(LINKFLAGS = '-z origin -Wl,--enable-new-dtags')
env.Append(RPATH = [ Literal("\\$$ORIGIN/libs-"+arch) ])

# source files
src = [getCPPFiles('../k2.vcproj'), 'c_system_posix.cpp']

# create the library, install to game dir, target
if build_type == 'debug':
	libname = 'k2_debug-'+arch+'.so'
	env.SharedLibrary(target = libname, source = src)
	env.Install('#../Heroes of Newerth', 'lib'+libname)
	Alias(project + '-' + build_type + '-' + arch, 'lib'+libname)
	Alias('install-' + project + '-' + build_type + '-' + arch, '#../Heroes of Newerth/lib'+libname)
else:
	libname = 'k2-'+arch+'.so'
	env.SplitSharedLibrary(target = [ libname, libname+'.dbg' ], source = src)
	env.Install('#../Heroes of Newerth', ['lib'+libname, 'lib'+libname+'.dbg'])
	Alias(project + '-' + build_type + '-' + arch, ['lib'+libname,'lib'+libname+'.dbg'])
	Alias('install-' + project + '-' + build_type + '-' + arch, ['#../Heroes of Newerth/lib'+libname, '#../Heroes of Newerth/lib'+libname+'.dbg'])
