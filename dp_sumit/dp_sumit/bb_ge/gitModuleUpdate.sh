#!/bin/sh

# This script iterates over all of the submodules and updates them to the version referenced by the
# main repository.  This is similar to git submodule update except that it tries to checkout the
# named branch specified in modules.py rather than checking out a detached head.  The script will
# not clobber dirty submodules and will only move the named branch when it is safe to do so without
# creating detached heads.

# args, TODO: could do some basic validation of the args & # of args
component=$1
remote=$2
trackingBranch=$3

# Target
commit=$(git ls-tree HEAD -- "$component" | awk '{print $3}')


# Helper function to display an error, and ensure error code is set correctly
showError () {
    echo "FAIL: $component: $@" >&2
    return 1
}

showWarning () {
    echo "WARNING: $component: $@" >&2
    return 1
}

tryBranchUpdate() {
    # First try to ensure that $trackingBranch exists by doing:
    #   git branch --list $trackingBranch and see if output is non-empty
    foundBranch=$(git branch --list "$trackingBranch" | sed 's/^\** \+//g')
    if [ "$foundBranch" != "$trackingBranch" ]
    then
        # Branch doesn't exist, try to create it by tracking the remote
        git branch --track "$trackingBranch" "$remote/$trackingBranch" > /dev/null 2>&1 || {
            showError "Couldn't create branch $trackingBranch to track $remote/$trackingBranch."
            return 1;
        }
    fi
    git merge-base --is-ancestor $trackingBranch $commit ||
    { git merge-base --is-ancestor $commit $trackingBranch &&
      git merge-base --is-ancestor $trackingBranch "$remote/$trackingBranch"; } ||
    showWarning "The target branch ($trackingBranch) cannot safely be moved to commit $commit." &&
    { git checkout -B "$trackingBranch" > /dev/null 2>&1 &&
      echo "SUCCESS: $component: On $trackingBranch" ||
      showError "Failed to move $trackingBranch to $commit."; };
}

cd $component > /dev/null 2>&1 || showError "Unable to cd to component" &&
    { (git fetch $remote; git status | grep 'working directory clean'; ) > /dev/null 2>&1 || showError "Workspace dirty"; }
if [ $? -eq 0 ]
then
    # workspace clean
    gitHead=$(git symbolic-ref HEAD 2>/dev/null)
    if [ $? -eq 0 ]
    then
        branch=$(echo "$gitHead" | sed 's/^.*\///g')
        if [ "$branch" != "$trackingBranch" ]
        then
            showError "On branch $branch, instead of $trackingBranch"
            exit 1
        fi
    fi
    { git checkout "$commit" > /dev/null 2>&1 || showError "Unable to checkout commit: $commit"; } && tryBranchUpdate
fi

