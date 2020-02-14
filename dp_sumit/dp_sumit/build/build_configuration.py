#!/usr/bin/env python

# Edit these values
bbReleaseNum = (0, 0, 34) # NOTE: This must of the form [0-99].[0-99].[0-99]
geReleaseNum = (0, 1, 5) # NOTE: This must of the form [0-99].[0-9].[0-9] for VHub device descriptor version reporting
pgmBbReleaseNum = (0, 0, 29)
romReleaseNum = (12, 0)

fpgaVer = (0x15, 03, 0)
fpgaTime = (2019, 05, 21, 14, 43, 43)           #YY, MM, DD, HR, MIN, SEC

# Update this tag ONLY if a new rom is being released.
# If not updated, the script will fail.
# NOTE: Make sure that this tag always points to the latest ROM 
currentRomReleaseTag = 'BBROMSW_v12_00'


# Update these tags to get previous release notes from master and update current
previousBbFwReleaseTag = 'BBSW_v00_00_33'
previousGeFwReleaseTag = 'BBGESW_v00_01_04'
previousPgmBbReleaseTag = 'BBPGMBBSW_v00_00_27'
previousRomReleaseTag = 'BBROMSW_v12_00'


# The branch to clone the code from. 
# The script does not include local changes or local commits. 
# NOTE: Make sure that everything is pushed to a branch before running the script.
bbReleaseFromBranch = 'release'
bbReleaseFromBranch = 'release'
geReleaseFromBranch = 'release'
pgmBbReleaseFromBranch = 'release'
romReleaseFromBranch = 'release'


# for binaries -- don't forget to update these
# Must reference MainFW BB not tag specific to Release above
geReleaseBinaryBBReleaseTag = 'maverick/BBSW_v00_00_33'
pgmBbReleaseBinaryBBReleaseTag = 'maverick/BBSW_v00_00_33'



