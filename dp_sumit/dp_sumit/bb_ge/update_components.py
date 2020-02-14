#!/usr/bin/env python

from modules import *
import subprocess

for (component, trackingBranch, remote) in submoduleConfig:
    #print '------- PROCESSING ' + component + ' -------'
    subprocess.call(
        './gitModuleUpdate.sh %s %s %s' %
        (component, remote, trackingBranch), shell=True)

