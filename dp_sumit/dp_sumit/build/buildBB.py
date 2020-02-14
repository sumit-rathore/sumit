#!/usr/bin/env python

# Guidelines for build script BuildBB.py

# By default it runs in debug mode
# Pass '-release' for an official release, it will create debug and non-debug files and also mcs files for BB and GE. 
# Please copy paste mcs files manually in released folder and rename.
# Don't forget to update FPGA version in build_configuration file.
# '-p' releases pgmbb and blackbird but copies GE binaries from last official release, dont forget to pass '-release' for official release.
# '-r' to build ROM in debug, pass '-r' and '-release' both for official ROM release

from build_common import *
import build_configuration as bc

# default
startStep = '0'
releaseMessage_original = 'Releasing project'

# Check for release flag
if '-b' in sys.argv:
    # build BB only and copy PgmBB binary and GE binary
    releaseProject = 'Blackbird Software'
elif '-g' in sys.argv:
    # build BB and GE and copy PgmBB binary
    releaseProject = 'Blackbird Software'
elif '-p' in sys.argv:
        # build BB and PgmBB and copy GE binary
    releaseProject = 'Blackbird Software'
#elif '-a' in sys.argv:
    # build GE, PgmBB, and BB
    #releaseProject = 'Blackbird Software'
elif '-r' in sys.argv:
    # self release - no BB, GE, or PGMBB operations
    releaseProject = 'ROM Software'
else:
    releaseProject = 'Blackbird Software'

# check for step number
for arg in sys.argv:
    if str.isdigit(arg):
        startStep = int(arg)

# Check for debug mode
debugMode=True
releaseMessage = releaseMessage_original + ' in debug mode'
if '-release' in sys.argv:
    # Debug mode -- don't commit/push anything
    debugMode=False
    releaseMessage = releaseMessage_original

print releaseMessage + ' "' + releaseProject + '" at step ' + repr(startStep)

# These values probably won't change
bbFwReleaseTag = 'BBSW_v%02d_%02d_%02d' % (bc.bbReleaseNum[0], bc.bbReleaseNum[1], bc.bbReleaseNum[2])
bbFwReleaseStr = '%d.%d.%d' % (bc.bbReleaseNum[0], bc.bbReleaseNum[1], bc.bbReleaseNum[2])
geReleaseTag = 'BBGESW_v%02d_%02d_%02d' % (bc.geReleaseNum[0], bc.geReleaseNum[1], bc.geReleaseNum[2])
geReleaseStr = '%d.%d.%d' % (bc.geReleaseNum[0], bc.geReleaseNum[1], bc.geReleaseNum[2])

# These tags are for Git only
pgmBbReleaseTag = 'BBPGMBBSW_v%02d_%02d_%02d' % (bc.pgmBbReleaseNum[0], bc.pgmBbReleaseNum[1], bc.pgmBbReleaseNum[2])
pgmBbreleaseStr = '%d.%d.%d' % (bc.pgmBbReleaseNum[0], bc.pgmBbReleaseNum[1], bc.pgmBbReleaseNum[2])
romReleaseTag = 'BBROMSW_v%02d_%02d' % (bc.romReleaseNum[0], bc.romReleaseNum[1])
romReleaseStr = '%d.%d' % (bc.romReleaseNum[0], bc.romReleaseNum[1])

bbge = False

# Set generic tags
if releaseProject == 'ROM Software':
    releaseTag = romReleaseTag
    releaseStr = romReleaseStr
    pgmbbReleaseTag = pgmBbReleaseTag
    pgmbbReleaseString = pgmBbreleaseStr
    releaseFromBranch = bc.romReleaseFromBranch
    previousReleaseTag = bc.previousRomReleaseTag
    geDir = ''
    bbgeReleaseTag = ''
    bbgeReleaseStr = ''
    bbge = False
    releaseNum = (bc.romReleaseNum[0], bc.romReleaseNum[1], '')
    bbgeReleaseNum = ('','','')
    majorRev = 'BOOTLOADER_MAJOR_REVISION'
    minorRev = 'BOOTLOADER_MINOR_REVISION'
    debugRev = ''
    bbgeMajorRev = ''
    bbgeMinorRev = ''
    bbgeDebugRev = ''
    buildArgList = '-r "make V=1 D=1"'

# This covers BB release, GE/BB release, PGMBB/BB release
# and all three, BB/GE/PGMBB release
if releaseProject == 'Blackbird Software':
    releaseTag = bbFwReleaseTag
    releaseStr = bbFwReleaseStr
    pgmbbReleaseTag = pgmBbReleaseTag
    pgmbbReleaseString = pgmBbreleaseStr
    releaseFromBranch = bc.bbReleaseFromBranch
    previousReleaseTag = bc.previousBbFwReleaseTag
    geDir = 'bb_ge'
    bbgeReleaseTag = geReleaseTag
    bbgeReleaseStr = geReleaseStr
    releaseNum = (bc.bbReleaseNum[0], bc.bbReleaseNum[1], bc.bbReleaseNum[2])
    bbgeReleaseNum = (bc.geReleaseNum[0], bc.geReleaseNum[1], bc.geReleaseNum[2])
    majorRev = 'SOFTWARE_MAJOR_REVISION'
    minorRev = 'SOFTWARE_MINOR_REVISION'
    debugRev = 'SOFTWARE_DEBUG_REVISION'
    fpgaMajorVer = 'FPGA_MAJOR_VER'
    fpgaMinorVer = 'FPGA_MINOR_VER'
    fpgaMinMinorVer ='FPGA_MINMINOR_VER'
    fpgaYear = 'FPGA_YEAR'
    fpgaMonth = 'FPGA_MONTH'
    fpgaDate = 'FPGA_DATE'
    fpgaHour = 'FPGA_HOUR'
    fpgaMinute = 'FPGA_MINUTE'
    fpgaSec = 'FPGA_SEC'
    
    if '-g' in sys.argv:
        bbge = True
        # GE built separately before BB
        # copy binary will take place before building BB
        # binary location is based on BB's release tag as this is the directory name
        # Remember, -b means ONLY BB, no GE or PGMBB is built, this script will
        # build them before calling the build for BB
        bbgeMajorRev = 'SOFTWARE_MAJOR_REVISION'
        bbgeMinorRev = 'SOFTWARE_MINOR_REVISION'
        bbgeDebugRev = 'SOFTWARE_DEBUG_REVISION'
        buildArgList = '-b "make V=1 D=1" -pbb_bin %s' % bc.pgmBbReleaseBinaryBBReleaseTag
        buildArgList_iso = '-only_iso "make V=1 D=1" -pbb_bin %s' % bc.pgmBbReleaseBinaryBBReleaseTag
        buildArgList_usb = '-only_usb "make V=1 D=1" -pbb_bin %s' % bc.pgmBbReleaseBinaryBBReleaseTag

    if '-p' in sys.argv:
        bbge = False
        buildArgList = '-b "make V=1 D=1" -ge_bin %s' % bc.geReleaseBinaryBBReleaseTag
        buildPlugArgList = '-plug_release "make V=1 D=1" -ge_bin %s' % bc.geReleaseBinaryBBReleaseTag
        buildArgList_iso = '-only_iso "make V=1 D=1" -ge_bin %s' % bc.geReleaseBinaryBBReleaseTag
        buildArgList_usb = '-only_usb "make V=1 D=1" -ge_bin %s' % bc.geReleaseBinaryBBReleaseTag
    
    if '-b' in sys.argv:
        bbge = False
        buildArgList = '-b "make V=1 D=1" -ge_bin %s -pbb_bin %s' % (bc.geReleaseBinaryBBReleaseTag, bc.pgmBbReleaseBinaryBBReleaseTag)
        buildPlugArgList = '-plug_release "make V=1 D=1" -ge_bin %s -pbb_bin %s' % (bc.geReleaseBinaryBBReleaseTag, bc.pgmBbReleaseBinaryBBReleaseTag)
        buildArgList_iso = '-only_iso "make V=1 D=1"-ge_bin %s -pbb_bin %s' % (bc.geReleaseBinaryBBReleaseTag, bc.pgmBbReleaseBinaryBBReleaseTag)
        buildArgList_usb = '-only_usb "make V=1 D=1"-ge_bin %s -pbb_bin %s' % (bc.geReleaseBinaryBBReleaseTag, bc.pgmBbReleaseBinaryBBReleaseTag)

    #if '-a' in sys.argv:
    if (not '-p' in sys.argv) and (not '-g' in sys.argv) and (not '-b' in sys.argv):
        bbge = True
        bbgeMajorRev = 'SOFTWARE_MAJOR_REVISION'
        bbgeMinorRev = 'SOFTWARE_MINOR_REVISION'
        bbgeDebugRev = 'SOFTWARE_DEBUG_REVISION'
        # automatically builds GE then PGMBB then BB
        buildArgList = '-release "make V=1 D=1"'
        buildArgList_iso = '-iso_release "make V=1 D=1"'
        buildArgList_usb = '-usb_release "make V=1 D=1"'

releasesDirRoot = '/data/engdev/designs/blackbird/released/sw/maverick/'
releasesRomDirRoot = '/data/engdev/designs/blackbird/released/rom/'
releaseDstDir = os.path.join(releasesDirRoot, releaseTag)
releaseRomDstDir = os.path.join(releasesRomDirRoot, releaseTag)
smbCmd = 'cd Projects\Blackbird\Firmware\Released\maverick; mkdir %s; cd %s; prompt; recurse; mput *' % (releaseTag, releaseTag)
gitRepo = 'icron_git_uri:p_bb_sw'
buildDirName = 'build_dir'
releaseNotesOutline = '\n\n%s v%s\n=========================\n\nSummary:\n\nImportant: \n\nMajor Changes: \n\nKnown Issues:\n\n\nEndOfSummary\n\n\n' % (releaseProject, releaseStr)
curDir = os.getcwd()
releaseDir = os.path.join(curDir, releaseTag)
buildDir = os.path.join(releaseDir, buildDirName)
simLinkDirName = 'release'
simLinkDir = os.path.join(releaseDir, simLinkDirName)

if __name__ == '__main__':
    p0 = AggregateStep('Blackbird Build')

    p0.appendStep(PythonStep(
            lambda: os.system('%s %s' % (os.getenv('EDITOR'), '%s/build_configuration.py' % curDir)),
            'Opening build_configuration.py to update variables'))

    if debugMode:
        p0.appendStep(CommandStep(
        'rm -rf "%s"' % (releaseTag),
        execDir=curDir, validationFunction=commandSuccessValidator))

    # Create local build directory
    p0.appendStep(CommandStep(
        'mkdir "%s"' % (releaseTag),
        execDir=curDir, validationFunction=commandSuccessValidator))

    # Create local release files directory
    p0.appendStep(CommandStep(
        'mkdir "%s"' % (simLinkDirName),
        execDir=releaseDir, validationFunction=commandSuccessValidator))

    # Clone into <BBSW_vxx_yy_zz>\build_dir
    p0.appendStep(CommandStep(
        'git clone --recursive -b %s "%s" "%s"' % (releaseFromBranch, gitRepo, buildDirName),
        execDir=releaseDir, validationFunction=commandSuccessValidator))

    # Check for valid build
    p0.appendStep(CommandStep(
        'git status',
        execDir=buildDir, validationFunction=gitStatusClearValidator))
    
    if not debugMode:
    # Prepend release notes summary outline into release_notes.txt
    # Prepend git log into release_notes.txt after outline
    # Release Author to use git log to assist in writing release notes summary
    # then delete git log (or script to delete git log?) from release_notes.txt
        # -- write outline into temp file
        p0.appendStep(CommandStep(
            'echo "%s" > releaseTemp.txt' % (releaseNotesOutline),
            execDir=buildDir, validationFunction=commandSuccessValidator))

        # -- append git log
        p0.appendStep(CommandStep(
            'git log --graph %s...%s >> releaseTemp.txt' % (previousReleaseTag, releaseFromBranch),
            execDir=buildDir, validationFunction=commandSuccessValidator))

        # -- instruct release author to edit temp file (use editor inline?)
        p0.appendStep(PythonStep(lambda: userPrompt(
            'Git logs and a release notes template have been created for you.\n' \
            'Please use the git logs as an aid to fill in the template information.\n\n' \
            'Do NOT delete the EndOfSummary or anything below it!!!\n\n' \
            'When finished entering the summary information, save and close the file\n'),
            'Prompt to fill in release notes information'))

        p0.appendStep(PythonStep(
            lambda: os.system('%s %s' % (os.getenv('EDITOR'), '%s/releaseTemp.txt' % buildDir)),
            'Opening git log for release author to create release notes summary, save and close when finished'))

        # -- read current release_notes.txt into memory, write author's summary outline
        p0.appendStep(PythonStep(
            lambda: truncateFileAfterLine(
                os.path.join(buildDir, 'releaseTemp.txt'), 'EndOfSummary\n'),
                'Drop revision log from release notes'))

        # -- append memory copy of release_notes.txt into release_notes.txt
        p0.appendStep(CommandStep(
            'cp doc/release_notes.txt releaseOld.txt',
            execDir=buildDir, validationFunction=commandSuccessValidator))

        p0.appendStep(CommandStep(
            'cat releaseTemp.txt > doc/release_notes.txt',
            execDir=buildDir, validationFunction=commandSuccessValidator))

        p0.appendStep(CommandStep(
            'cat releaseOld.txt >> doc/release_notes.txt',
            execDir=buildDir, validationFunction=commandSuccessValidator))

        # -- remove temp file
        p0.appendStep(CommandStep(
            'rm releaseTemp.txt',
            execDir=buildDir, validationFunction=commandSuccessValidator))

        p0.appendStep(CommandStep(
            'rm releaseOld.txt',
            execDir=buildDir, validationFunction=commandSuccessValidator))

        # Commit release notes
        p0.appendStep(CommandStep(
            'git add doc/release_notes.txt',
            execDir=buildDir, validationFunction=commandSuccessValidator))

        p0.appendStep(CommandStep(
            'git commit -m "Update release notes for v%s"' % (releaseStr),
            execDir=buildDir, validationFunction=commandSuccessValidator))

    if bbge:
        p0.appendStep(CommandStep(
            'git tag %s -m "Goldenears software version %s"' % (bbgeReleaseTag, bbgeReleaseStr),
            execDir=os.path.join(buildDir, geDir), validationFunction=commandSuccessValidator))

    p0.appendStep(CommandStep(
        'git tag %s -m "Blackbird software version %s"' % (releaseTag, releaseStr),
        execDir=buildDir, validationFunction=commandSuccessValidator))

    p0.appendStep(CommandStep(
        'cp -f "/data/engdev/designs/blackbird/released/sw/%s/build_dir/bb_ge/inc/options.h" "./bb_ge/inc/options.h"' % (bc.geReleaseBinaryBBReleaseTag),
        execDir=buildDir, validationFunction=commandSuccessValidator))

    # Update ProgramBB release numbers if we're releasing it
    if ('-p' in sys.argv):
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+$/\\1%d/\' inc/options.h' % ('PGMBB_MAJOR_REVISION', releaseNum[0]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+$/\\1%d/\' inc/options.h' % ('PGMBB_MINOR_REVISION', releaseNum[1]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        if releaseNum[2] != '':
            p0.appendStep(CommandStep(
                'sed --in-place \'s/^\(#define %s\s\+\)\w\+$/\\1%d/\' inc/options.h' % ('PGMBB_DEBUG_REVISION', releaseNum[2]),
                execDir=buildDir, validationFunction=commandSuccessValidator))

    # Update all release numbers for no arguement
    if (not '-p' in sys.argv) and (not '-g' in sys.argv) and (not '-b' in sys.argv) and (not '-r' in sys.argv):
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+$/\\1%d/\' inc/options.h' % ('PGMBB_MAJOR_REVISION', releaseNum[0]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+$/\\1%d/\' inc/options.h' % ('PGMBB_MINOR_REVISION', releaseNum[1]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        if releaseNum[2] != '':
            p0.appendStep(CommandStep(
                'sed --in-place \'s/^\(#define %s\s\+\)\w\+$/\\1%d/\' inc/options.h' % ('PGMBB_DEBUG_REVISION', releaseNum[2]),
                execDir=buildDir, validationFunction=commandSuccessValidator))
  
    # Update release numbers
    p0.appendStep(CommandStep(
        'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\1%d/\' inc/options.h' %(majorRev, releaseNum[0]),
        execDir=buildDir, validationFunction=commandSuccessValidator))
    p0.appendStep(CommandStep(
        'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\1%d/\' inc/options.h' % (minorRev, releaseNum[1]),
        execDir=buildDir, validationFunction=commandSuccessValidator))
    if releaseNum[2] != '':
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\1%d/\' inc/options.h' % (debugRev, releaseNum[2]),
            execDir=buildDir, validationFunction=commandSuccessValidator))

    # Update ge's release numbers if we're releasing it
    if bbge:
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\1%d/\' inc/options.h' % (bbgeMajorRev, bbgeReleaseNum[0]),
            execDir=os.path.join(buildDir, geDir), validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\1%d/\' inc/options.h' % (bbgeMinorRev, bbgeReleaseNum[1]),
            execDir=os.path.join(buildDir, geDir), validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\1%d/\' inc/options.h' % (bbgeDebugRev, bbgeReleaseNum[2]),
            execDir=os.path.join(buildDir, geDir), validationFunction=commandSuccessValidator))
    
    if (not '-r' in sys.argv):
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\10x%x/\' src/imain.c' %(fpgaMajorVer, bc.fpgaVer[0]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\10x%x/\' src/imain.c' % (fpgaMinorVer, bc.fpgaVer[1]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\10x%x/\' src/imain.c' % (fpgaMinMinorVer, bc.fpgaVer[2]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\10x%d/\' src/imain.c' %(fpgaYear, bc.fpgaTime[0]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\10x%d/\' src/imain.c' % (fpgaMonth, bc.fpgaTime[1]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\10x%d/\' src/imain.c' % (fpgaDate, bc.fpgaTime[2]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\10x%d/\' src/imain.c' %(fpgaHour, bc.fpgaTime[3]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\10x%d/\' src/imain.c' % (fpgaMinute, bc.fpgaTime[4]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
        p0.appendStep(CommandStep(
            'sed --in-place \'s/^\(#define %s\s\+\)\w\+/\\10x%d/\' src/imain.c' % (fpgaSec, bc.fpgaTime[5]),
            execDir=buildDir, validationFunction=commandSuccessValidator))
    
    # Build firmware image(s)
    # for GE and BB don't use shell script, falsely generates errors according to Python
    # if GE release - build GE first, separately
    if '-g' in sys.argv:
        p0.appendStep(CommandStep(
            './ge_bb_icron.sh -ge  %s >> ../buildLog.txt 2>&1' % 'make V=1 D=1',
            execDir=buildDir, validationFunction=""))

    # if PgmBB release - build PgmBB First, separately
    if '-p' in sys.argv:
        if '-iso' in sys.argv:
            p0.appendStep(CommandStep(
                './ge_bb_icron.sh -pgmbb_iso %s >> ../buildLog.txt 2>&1' % 'make V=1 D=1',
                execDir=buildDir, validationFunction=""))
        else:
            p0.appendStep(CommandStep(
                './ge_bb_icron.sh -pgmbb %s >> ../buildLog.txt 2>&1' % 'make V=1 D=1',
                execDir=buildDir, validationFunction=""))

    # if BB/GE/PGMBB release - pass no args - shell will automatically build GE then PGMBB then BB
    # if BB only release - pass arg for -?_bin <BBSW_vXX_YY_ZZ> in addition to -b "m clean" m
    if '-r' in sys.argv:
        p0.appendStep(CommandStep(
            './ge_bb_icron.sh -r %s >> ../buildLog.txt 2>&1' % '"m clean" "m V=1 D=1"',
            execDir=buildDir, ))
    else:
        if '-iso' in sys.argv: 
            p0.appendStep(CommandStep(
                './ge_bb_icron.sh %s>> ../buildLog.txt 2>&1' % buildArgList_iso,
                execDir=buildDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/cobs_current_raven.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/cobs_current_raven_debug.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/cobs_golden.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/cobs_golden_debug.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/ExCOM.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/ExCOM_debug.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/91-00376_raven.mcs .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/91-00379.mcs .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
        elif '-usb' in sys.argv:
            p0.appendStep(CommandStep(
                './ge_bb_icron.sh %s>> ../buildLog.txt 2>&1' % buildArgList_usb,
                execDir=buildDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/cobs_current_raven.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/cobs_current_raven_debug.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/cobs_golden.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/cobs_golden_debug.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/ExCOM.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/ExCOM_debug.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/91-00376_raven.mcs .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/91-00379.mcs .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))            
        else:
            p0.appendStep(CommandStep(
            './ge_bb_icron.sh %s >> ../buildLog.txt 2>&1' % buildPlugArgList,
            execDir=buildDir, validationFunction=commandSuccessValidator))

            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/cobs_plug_test.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/cobs_plug_debug_test.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))                        
            
            p0.appendStep(CommandStep(
            './ge_bb_icron.sh %s >> ../buildLog.txt 2>&1' % buildArgList,
                execDir=buildDir, validationFunction=commandSuccessValidator))

            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/cobs_golden.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/cobs_current.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/ExCOM.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/cobs_golden_debug.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/cobs_current_debug.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_debug_bin/ExCOM_debug.icron .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/91-00377.mcs .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/91-00378.mcs .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'cp ../build_dir/blackbird_a7_bin/91-00379.mcs .',
                execDir=simLinkDir, validationFunction=commandSuccessValidator))
    p0.appendStep(CommandStep(
        'cp ../build_dir/doc/release_notes.txt .',
        execDir=simLinkDir, validationFunction=commandSuccessValidator))
    p0.appendStep(CommandStep(
        '(echo Creating release build on $(date); echo Using:; sparc-elf-gcc -v;) > info.txt 2>&1',
        execDir=simLinkDir, validationFunction=commandSuccessValidator))
    if releaseProject == 'ROM Software':
        p0.appendStep(CommandStep(
            'cp ../build_dir/rom/blackbird_rom_bin/blackbird_rom.bin .',
            execDir=simLinkDir, validationFunction=commandSuccessValidator))

    # Final steps in git
    if not debugMode:

        p0.appendStep(PythonStep(lambda: userPrompt(
        'Please do a manual review of the build directory.  After this step, ' \
        'the build tags will be pushed to the central git repository.'),
        'Prompt to do top-level review'))
        if bbge:
            p0.appendStep(CommandStep(
                'git push --tags',
                execDir=os.path.join(buildDir, geDir), validationFunction=commandSuccessValidator))

        p0.appendStep(CommandStep(
            'git push',
            execDir=buildDir, validationFunction=commandSuccessValidator))

        p0.appendStep(CommandStep(
            'git push --tags',
            execDir=buildDir, validationFunction=commandSuccessValidator))

        p0.appendStep(PythonStep(lambda: userPrompt(
            'Close any editors that have files in the release directory open in ' \
            'order to prevent marking their temporary files as read-only.'),
            'Prompt to close editors'))
        if releaseProject == 'ROM Software':
                    # Copy all folder/files to engdev's release folder
            p0.appendStep(CommandStep(
                'cp -R . %s' % releaseRomDstDir,
                execDir=releaseDir, validationFunction=commandSuccessValidator))
            p0.appendStep(CommandStep(
                'chmod -R a-w .',
                execDir=releaseRomDstDir, validationFunction=commandSuccessValidator))

        else:
        # Copy all folder/files to engdev's release folder
            p0.appendStep(CommandStep(
                'cp -R . %s' % releaseDstDir,
                execDir=releaseDir, validationFunction=commandSuccessValidator))

            p0.appendStep(CommandStep(
                'chmod -R a-w .',
                execDir=releaseDstDir, validationFunction=commandSuccessValidator))

        # Create link for latest release
            # p0.appendStep(CommandStep(
            #     'rm current',
            #     execDir=releasesDirRoot, validationFunction=commandSuccessValidator))
            # p0.appendStep(CommandStep(
            #     'ln -s %s current' % (releaseDstDir, ),
            #     execDir=releasesDirRoot, validationFunction=commandSuccessValidator))

        # # Copy files to USB/Projects/Blackbird/Firmware
        p0.appendStep(PythonStep(lambda: userPrompt(
            'After the smbclient command is issued, smbclient will wait for you\n' \
            'to enter your network password, followed by the Enter key.'),
            'Prompt to enter network password'))

        p0.appendStep(CommandStep(
            'smbclient //YVRWSRV01.maxim-ic.internal/USB -I 10.0.4.28 -c "%s"' % (smbCmd),
            execDir=simLinkDir, validationFunction=commandSuccessValidator))

    if startStep != '0':
        p0.runFromStep(startStep)
    else:
        p0.run()

    print 'Build completed'

