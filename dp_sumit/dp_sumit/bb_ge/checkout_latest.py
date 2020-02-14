#!/usr/bin/env python

from modules import *
import subprocess

for (component, trackingBranch, remote) in submoduleConfig:
    print '------- PROCESSING ' + component + ' -------'
    subprocess.call(
        '(cd %s && git checkout %s && git merge --ff-only %s/%s)' %
        (component, trackingBranch, remote, trackingBranch), shell=True)

