import os, subprocess, datetime, re, shlex

class TortoiseSVNManager:
    def __init__(self, tortoisesvn=None):
        if tortoisesvn == None:
            print("\n\n None Path - TortoiseProc.exe")
            os.system("Pause")
            sys.exit()
        else:
            self.tortoisesvn = tortoisesvn
    
    def makecommitmsg(self, buildversion, commitmsg):
        # Make Commit Message
        try:
            with open(r'./commitmsg.txt', "w") as f:
                buildversion = re.sub(",", ", ", buildversion)
                f.write(commitmsg + "\n\n" + buildversion)
        except FileNotFoundError:
            return False

        return True

    def commit(self, projectlist):
        # Ensure TortoiseProc exists
        if not os.path.isfile(self.tortoisesvn + '\\TortoiseProc.exe'):
            raise Exception('TortoiseProc.exe not found. path=' + self.tortoisesvn + '\\TortoiseProc.exe')

        commitmsgpath = os.getcwd()
        os.chdir(self.tortoisesvn)


        for project in projectlist:
            if project["isuse"] == "1":
                print("PROGRESSING COMMIT - " + project["project_path"] + "\n")
                command = 'TortoiseProc.exe'
                command += ' /command:commit'
                command += (' /path:' + project["project_path"])
                command += (' /logmsgfile:"' + commitmsgpath + '\\commitmsg.txt"')
                command += ' /closeonend:0'
                os.system(command)

        print("\n")
        return True


    def run(self, buildversion=None, projectlist=None, commitmsg=None):
        summary = ''
        
        # File header
        start = datetime.datetime.now()
        print('\n' * 3)
        summary += self.log('STARTED SVN COMMIT - ' + start.strftime("%Y-%m-%d %H:%M:%S"))
        
        # Make Commit Message
        if (buildversion is not None) and (commitmsg is not None):
            makeOk = self.makecommitmsg(buildversion, commitmsg)
            if not makeOk:
            	self.log('COMMIT: FAILED - FILE NOT FOUND', start)
            	sys.exit(100)
            summary += self.log('COMMIT: SUCCEEDED - MAKE COMMIT MESSAGE', start)
        else:
        	summary += self.log('COMMIT: NOT SPECIFIED')

        # Commit
        if projectlist is not None:
            commitOK = self.commit(projectlist)
            if not commitOK:
            	self.log('COMMIT: FAILED', start)
            	sys.exit(100)
            summary += self.log('COMMIT: SUCCEEDED', start)
        else:
        	summary += self.log('COMMIT: NOT SPECIFIED - PROJECT LIST')
      


        summary += self.log('COMMIT: *** FINISH ***', start)
        
        # Build summary
        print('\n\n' + '-' * 80)
        print(summary)
        print('-' * 80)
        
    def log(self, message, start=None):
    	timestamp = ''
    	numsecs = ''
    	if start is not None:
    		split = datetime.datetime.now()
    		diff = split - start
    		timestamp = split.strftime("%Y-%m-%d %H:%M:%S") + '\t'
    		numsecs = ' (' + str(diff.seconds) + ' seconds)'
    	msg = timestamp + message + numsecs + '\n\n'
    	print('=' * 10 + '> ' + msg)
    	return msg