import glob
import os

Import("*")

# Note that this script is executed from within the <project>/build/<something>/ folder
curdir = os.getcwd()

# Get our project directory name
project_name = os.path.basename(os.path.dirname(os.path.dirname(curdir)))

#print curdir, project_name

# Determine Chaste libraries to link against.
# Note that order does matter!
chaste_libs = [project_name] + comp_deps['core']
#chaste_libs = [project_name, 'cancer'] + comp_deps['cancer']
#chaste_libs = [project_name, 'heart'] + comp_deps['heart']
#chaste_libs = [project_name, 'cancer', 'heart'] + comp_deps['heart']


# Look for .cpp files within the src folder
os.chdir('../..') # This is so .o files are built in <project>/build/<something>/
files, extra_cpppath = SConsTools.FindSourceFiles('src')
extra_cpppath.append('src')

# Look for source files that tests depend on under test/.
testsource, test_cpppath = SConsTools.FindSourceFiles('test', ['data'])
extra_cpppath.extend(test_cpppath)
del test_cpppath

os.chdir(curdir)

# Look for files containing a test suite
# A list of test suites to run will be found in a test/<name>TestPack.txt
# file, one per line.
# Alternatively, a single test suite may have been specified on the command
# line.
test_this_comp = False
for targ in BUILD_TARGETS:
    if str(targ) in ['projects/'+project_name, '.', Dir('#').abspath]:
        test_this_comp = True
testfiles = set()
if single_test_suite:
  if single_test_suite_dir == project_name:
    testfiles.add(single_test_suite)
    # Remove any old test output file to force a re-run
    try:
      os.remove(single_test_suite[:-4] + '.log')
    except OSError:
      pass
elif test_this_comp:
  packfiles = []
  if all_tests:
    for packfile in glob.glob('../../test/*TestPack.txt'):
      try:
        packfiles.append(file(packfile, 'r'))
      except IOError:
        pass
  else:
    for testpack in build.TestPacks():
      try:
        packfile = '../../test/'+testpack+'TestPack.txt'
        packfiles.append(file(packfile, 'r'))
      except IOError:
        pass
  for packfile in packfiles:
    try:
      for testfile in map(lambda s: s.strip(), packfile.readlines()):
        # Ignore blank lines and repeated tests.
        if testfile and not testfile in testfiles:
          testfiles.add(testfile)
      packfile.close()
    except IOError:
      pass


#print test_cpppath, testsource
#print files, testfiles, testsource

# Add test folders to CPPPATH only for this component
if extra_cpppath:
    newenv = env.Copy()
    newenv.Prepend(CPPPATH=extra_cpppath)
    # Make sure both envs reference the same dict *object*,
    # so updates in one env are reflected in all.
    newenv['CHASTE_OBJECTS'] = env['CHASTE_OBJECTS']
    env = newenv

# Libraries to link against
all_libs = ['test'+project_name] + chaste_libs + other_libs


if use_chaste_libs:
    # Build the library for this project
    project_lib = env.StaticLibrary(project_name, files)
    
    # Build the test library for this project
    test_lib = env.Library('test'+project_name, testsource)
else:
    # Build the object files for this project
    project_lib = test_lib = None
    for source_file in files + testsource:
        obj = env.StaticObject(source_file)
        key = os.path.join('projects', project_name, source_file)
        print project_name, "source", key
        env['CHASTE_OBJECTS'][key] = obj[0]

# Make test output depend on shared libraries, so if implementation changes
# then tests are re-run.
lib_deps = [project_lib, test_lib] # only this project's libraries
#lib_deps.extend(map(lambda lib: '#lib/lib%s.so' % lib, chaste_libs)) # all Chaste libs

# Collect a list of test log files to use as dependencies for the test
# summary generation
test_log_files = []

# Build and run tests of this project
if not use_chaste_libs:
    env['TestBuilder'] = \
        lambda target, source: env.Program(target, source,
                    LIBS=other_libs,
                    LIBPATH=other_libpaths)
for testfile in testfiles:
    prefix = os.path.splitext(testfile)[0]
    #print project_name, 'test', prefix
    test_hpp = os.path.join('test', testfile)
    runner_cpp = env.Test(prefix+'Runner.cpp', test_hpp)
    runner_exe = File(prefix+'Runner').abspath
    if use_chaste_libs:
        env.Program(runner_exe, runner_cpp,
                    LIBS = all_libs,
                    LIBPATH = ['#/lib', '.'] + other_libpaths)
    else:
        runner_obj = env.StaticObject(runner_cpp)
        runner_dummy = runner_exe+'.dummy'
        env.BuildTest(runner_dummy, runner_obj, RUNNER_EXE=runner_exe)
        env.AlwaysBuild(runner_dummy)
        env.Depends(runner_exe, runner_dummy)
    if not compile_only:
        log_file = env.File(prefix+'.log')
        if use_chaste_libs:
            env.Depends(log_file, lib_deps)
        else:
            env.Depends(log_file, runner_dummy)
        test_log_files.append(log_file)
        env.RunTest(log_file, runner_exe)

Return("test_log_files")