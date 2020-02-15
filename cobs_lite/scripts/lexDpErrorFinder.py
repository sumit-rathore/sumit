import re
import os
import sys

def getTimeStamp(logLine):
    timeStampPattern = r"[0-9]{10}"
    timeStamp = re.findall(timeStampPattern, str(logLine))
    return timeStamp

if __name__ == "__main__":

    if len(sys.argv) == 3:
        MAX_LT_TIME = 1000000
        MAX_VIDEO_LT_TIME = 2000000
        MAX_VIDEO_TIME = 30000000

        logFileName = sys.argv[1]
        report_file = sys.argv[2]
        testReport = open(report_file, "a")

        LinkTrainStartPattern   = r".*Setting training pattern 1.*"
        LinkTrainEndPattern     = r".*Link Trained.*"
        VideoStreamEndPattern   = r".*DP No Video Stream status = 1.*"
        VideoFlowingPattern     = r".*Lane Count.*"
        TuSizePattern           = r".*Tu Size.*"

        currentLTStartTimeStamp = ""
        currentLTEndTimeStamp = ""
        currentEndOfVideoTimeStamp = ""

        linkTrainingComplete = False
        linkTrainingStarted = False
        videoIssues = True

        sucessFullLinkTrainings = 0
        linkTrainingFailures    = 0

        if os.path.exists(logFileName):
            with open(logFileName, 'r') as log:
                for line in log:
                    #Link training start
                    if re.findall(LinkTrainStartPattern, line):

                        readTime = re.findall(r"[0-9]{2}:[0-9]{2}:[0-9]{2}", str(line))
                        testReport.write(str(readTime[0]) + " | ")

                        linkTrainingStarted = True
                        linkTrainingComplete = False
                        currentLTStartTimeStamp = getTimeStamp(line)
                        testReport.write(currentLTStartTimeStamp[0] + " | Link Training Started\n")
                    #Link training end
                    elif re.findall(LinkTrainEndPattern, line):
                        currentLTEndTimeStamp = getTimeStamp(line)

                        readTime = re.findall(r"[0-9]{2}:[0-9]{2}:[0-9]{2}", str(line))
                        testReport.write(str(readTime[0]) + " | ")

                        if linkTrainingStarted == False:
                            testReport.write(currentLTEndTimeStamp[0] + " | Error: Link training concluded before starting\n")
                            linkTrainingFailures = linkTrainingFailures + 1
                        elif int(currentLTEndTimeStamp[0]) - int(currentLTStartTimeStamp[0]) > MAX_LT_TIME:
                            testReport.write(currentLTEndTimeStamp[0] + " | Error: Link training took " + str(int(currentLTEndTimeStamp[0]) - int(currentLTStartTimeStamp[0])) + " micro seconds\n")
                            linkTrainingFailures = linkTrainingFailures + 1
                        else:
                            testReport.write(currentLTEndTimeStamp[0] + " | Link Training finished sucessfully\n")
                            linkTrainingComplete = True
                            sucessFullLinkTrainings = sucessFullLinkTrainings + 1
                        linkTrainingStarted = False
                    #Video flowing
                    elif re.findall(VideoFlowingPattern, line):
                        videoTime = getTimeStamp(line)
                        readTime = re.findall(r"[0-9]{2}:[0-9]{2}:[0-9]{2}", str(line))
                        testReport.write(str(readTime[0]) + " | ")
                        if linkTrainingComplete == False:
                            testReport.write(videoTime[0] + " | Error: Video tried to come up without link training\n")
                            linkTrainingFailures = linkTrainingFailures + 1
                        elif int(videoTime[0]) - int(currentLTEndTimeStamp[0]) > MAX_VIDEO_LT_TIME:
                            testReport.write(videoTime[0] + " | Error: Video took " + str(int(videoTime[0]) - int(currentLTEndTimeStamp[0])) + " micro seconds to come up\n")
                            linkTrainingFailures = linkTrainingFailures + 1
                        elif int(videoTime[0]) - int(currentEndOfVideoTimeStamp[0]) > MAX_VIDEO_TIME:
                            testReport.write(videoTime[0] + " | Error: Video took " + str(int(videoTime[0]) - int(currentEndOfVideoTimeStamp[0])) + " micro seconds to come up\n")
                            linkTrainingFailures = linkTrainingFailures + 1
                        else:
                            videoIssues = False
                    #End of Video Stream
                    elif re.findall(VideoStreamEndPattern, line):
                        readTime = re.findall(r"[0-9]{2}:[0-9]{2}:[0-9]{2}", str(line))
                        testReport.write(str(readTime[0]) + " | ")

                        currentEndOfVideoTimeStamp = getTimeStamp(line)
                        linkTrainingComplete = False
                        linkTrainingStarted = False
                        testReport.write(currentEndOfVideoTimeStamp[0] + " | End of Video Stream\n")
                    #Video flow verification
                    elif re.findall(TuSizePattern, line) and videoIssues == False:
                        temp = re.findall(r"= [0-9][0-9]?", str(line))
                        tuSize = int(temp[0][2:])
                        readTime = re.findall(r"[0-9]{2}:[0-9]{2}:[0-9]{2}", str(line))
                        videoTime = getTimeStamp(line)
                        testReport.write(str(readTime[0]) + " | " + videoTime[0] + " | ")
                        if tuSize > 64 or tuSize < 16 or tuSize % 4 > 0:
                            testReport.write("Error: Tu Size is: " + str(tuSize) + ", video did not come up\n")
                            linkTrainingFailures = linkTrainingFailures + 1
                        else:
                            testReport.write("Video came up successfully\n")

                testReport.write("\n Link Trainings completed sucessfully: " + str(sucessFullLinkTrainings))
                print("\n Link Trainings completed sucessfully: " + str(sucessFullLinkTrainings))

                testReport.write("\n Link Trainings with ERRORS: " + str(linkTrainingFailures))
                print("\n Link Trainings with ERRORS: " + str(linkTrainingFailures))
                log.close()

    else:
        print(" Usage:")
        print("     lexDpErrorFinder.py [log.txt] [output.txt]")
