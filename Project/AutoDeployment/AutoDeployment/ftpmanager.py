import os, ftplib, fnmatch, datetime, re

class FTPManager:
    def __init__(self, host=None, port=None, ftpid=None, ftppass=None):
        if host == None:
            print("\n\n Error - host")
            os.system("Pause")
            sys.exit()
        else:
            self.host = host

        if port == None:
            print("\n\n Error - port")
            os.system("Pause")
            sys.exit()
        else:
            self.port = port

        if ftpid == None:
            print("\n\n Error - ftpid")
            os.system("Pause")
            sys.exit()
        else:
            self.ftpid = ftpid

        if ftppass == None:
            print("\n\n Error - ftppass")
            os.system("Pause")
            sys.exit()
        else:
            self.ftppass = ftppass

    def connectftp(self):
        self.myftp = ftplib.FTP()
        self.myftp.connect(self.host, int(self.port))

        try:
            self.myftp.login(self.ftpid, self.ftppass)
        except:
            return False

        return True

    def setlocalpath(self, projectlist):
        self.localpathlist = []
        for project in projectlist:
            if project["isuse"] == "1":
                regex_1 = re.compile("[a-zA-Z_]+")
                configlist = regex_1.findall(project["configuration"])
                for config in configlist:
                    localpath = project["output_path"] + '\\' + project["name"] + '\\' + config
                    self.localpathlist.append(localpath)
                        

        return True

    def uploadfile(self, serverpath, folder):
        folderlist = []
        self.myftp.dir(serverpath, folderlist.append)

        existfolder = False
        regex_1 = re.compile("\w*?\s*?-*?.*?:*?" + folder)
        for name in folderlist:
            if regex_1.match(name) is not None:
                existfolder = True
                    
        if existfolder is False:
            self.myftp.mkd(serverpath + "//" + folder)

        self.myftp.cwd(serverpath + "//" + folder)

        for localpath in self.localpathlist:
            filelist = os.listdir(localpath)
            targetlist = [filename for filename in filelist if fnmatch.fnmatch(filename, '*.exe') or fnmatch.fnmatch(filename, '*.pdb')]
        
            for filename in targetlist:
                print("UPLOADING FILE - " + filename)
                self.myftp.storbinary('STOR '+filename, open(localpath + "\\" + filename, 'rb'))
               
            print("\n") 
                
        return True


    def run(self, projectlist=None, serverpath=None, folder=None):
        summary = ''
        
        # File header
        start = datetime.datetime.now()
        print('\n' * 5)
        summary += self.log('STARTED UPLOAD FILE - ' + start.strftime("%Y-%m-%d %H:%M:%S"))

        # Connect FTP Server
        connectOk = self.connectftp()
        if not connectOk:
            self.log('UPLOAD FILE: FAILED - CONNECT FTP SERVER', start)
            sys.exit(100)
        summary += self.log('UPLOAD FILE: SUCCEEDED - CONNECT STP SERVER', start)

        # Set Local Path
        if projectlist is not None:
            setlocalpathOk = self.setlocalpath(projectlist)
            if not setlocalpathOk:
            	self.log('UPLOAD FILE: FAILED - SET LOCAL PATH', start)
            	sys.exit(100)
            summary += self.log('UPLOAD FILE: SUCCEEDED - SET LOCAL PATH', start)
        else:
        	summary += self.log('UPLOAD FILE: NOT SPECIFIED - PROJECT LIST')


        # Upload File
        if (serverpath is not None) and (folder is not None):
            uploadOk = self.uploadfile(serverpath, folder)
            if not uploadOk:
            	self.log('UPLOAD FILE: FAILED', start)
            	sys.exit(100)
            summary += self.log('UPLOAD FILE: SUCCEEDED', start)
        else:
        	summary += self.log('UPLOAD FILE: NOT SPECIFIED - SERVER PATH, FOLDER')

        # FTP Quit
        self.myftp.quit()


        summary += self.log('UPLOAD FILE: ** FINISH **', start)
        
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