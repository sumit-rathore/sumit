import subprocess
import os
import time
import sys


class Step(object):
    def __init__(self, description):
        self.description = description

    def run(self):
        stepNum = '.'.join([str(n) for n in AggregateStep.location])
        print 'Running Step %s: %s' % (stepNum, self.description)
        self.doRun()

    # All Step subclasses should override the doRun method to perform their main function
    def doRun():
        pass


# A step class for aggregating a collection of other steps
class AggregateStep(Step):
    location = []
    def __init__(self, description):
        Step.__init__(self, description)
        self.subSteps = []

    def appendStep(self, step):
        self.subSteps.append(step)

    def runFromStep(self, stepNum):
        AggregateStep.location.append(stepNum)
        for i in range(stepNum, len(self.subSteps)):
            AggregateStep.location[-1] = i
            try:
                self.subSteps[i].run()
            except StepException, e:
                # TODO: make sure that we are copying, not aliasing here
                e.identity = AggregateStep.location
                raise e
        AggregateStep.location = AggregateStep.location[:-1]

    def doRun(self):
        self.runFromStep(0)


class CommandStep(Step):
    def __init__(self, command, execDir=None, validationFunction=None):
        Step.__init__(self, command)
        self.command = command
        self.execDir = execDir
        self.validationFunction = validationFunction
        self.output = None
        self.exitCode = None


    def doRun(self):
        # Run the command in a shell and redirect stderr to stdout since we
        # want the output interleaved.
        p = subprocess.Popen(
            self.command, cwd=self.execDir, shell=True,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        (stdout, stderr) = p.communicate()
        self.output = stdout
        self.exitCode = p.returncode
        if(self.validationFunction):
            self.validationFunction(self)


# A python step which runs the provided Python function.
class PythonStep(Step):
    def __init__(self, function, description):
        Step.__init__(self, description)
        self.function = function

    def doRun(self):
        self.function()


class StepException(Exception):
    def __init__(self):
        # The identity is a list of ints built up by the exception handling
        # within AggregateStep.  The goal is to identify the location at which
        # the build failed
        self.identity = []


class CommandStepException(StepException):
    def __init__(self, cmd, execDir, exitCode, output):
        StepException.__init__(self)
        if(not execDir):
            execDir = os.getcwd()
        self.data = 'Command "%s" executed in "%s" and exited with code "%s"\n' \
                'Command Output:\n%s' % (cmd, execDir, exitCode, output)

    def __str__(self):
        return self.data


class PythonStepException(StepException):
    def __init__(self, s):
        self.s = s

    def __str__(self):
        return self.s


###################
# Step Validators
###################

# A step validator for use with CommandStep.  A command is considerd successful
# if the exit code was 0.
def commandSuccessValidator(commandStep):
    if(commandStep.exitCode != 0):
        raise CommandStepException(
            commandStep.command, commandStep.execDir,
            commandStep.exitCode, commandStep.output)


# A step validator for a CommandStep which runs "git status".  It is considered
# a success if there are no modified files in the workspace.
def gitStatusClearValidator(commandStep):
    # First make sure that the git status actually succeeded
    commandSuccessValidator(commandStep)

    # Older git (pre 1.8.0 ?) output the following message
    #if (not commandStep.output.strip().endswith('nothing to commit (working directory clean)')):
    # Newer git (post 1.8.0 ?) output the following
    if (not commandStep.output.strip().endswith('nothing to commit, working directory clean')):
        raise PythonStepException(
            'git status check failed.  git status output was:\n%s' \
                % (commandStep.output))


###################
# Helper functions
###################

# Helper function to prompt the user to take action
def userPrompt(prompt):
    print '--------------- USER INTERVENTION REQUIRED ---------------'
    print prompt
    time.sleep(5)
    prompt = True
    while(prompt):
        txt = raw_input('Type "C" to continue: ')
        prompt = txt != 'C'


def truncateFileAfterLine(fileToModify, lineToMatch):
    f = open(fileToModify, 'rw+')
    found = False
    while True:
        l = f.readline()
        if l == '':
            # reached EOF
            break
        if l == lineToMatch:
            found = True
            f.truncate(f.tell())
            break
    f.close()
    if not found:
        raise PythonStepException('Error: Could not find a matching line')
