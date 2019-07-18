import os, subprocess, datetime, re, shlex, fnmatch

class MSBuilder:
    def __init__(self, builpathlist=None, projectlist=None):
		# The following dictionary holds the location of the various
		#	msbuild.exe paths for the .net framework versions
        if builpathlist == None:
            print("\n\n None Path - msbuild.exe")
            os.system("Pause")
            sys.exit()
        else:
            self.builpathlist = builpathlist
        
        if projectlist == None:
            print("\n\n None Info - projectlist")
            os.system("Pause")
            sys.exit()
        else:
            self.projectlist = projectlist   

    def setversion(self, buildversion):
        # Set version at resource file
        for project in self.projectlist:
            if project["isuse"] == "1":
                rcfile = project["project_path"] + "\\" + project["name"] + ".rc"
                try:
                    with open(rcfile, "r+", encoding="ISO-8859-1") as f:
                        rc_content = f.read()
                except FileNotFoundError:
                    return False

                # first part
                # FILEVERSION 6,0,20,163         
                # PRODUCTVERSION 6,0,20,163
                regex_1 = re.compile(r"\b(FILEVERSION|FileVersion|PRODUCTVERSION|ProductVersion) \d+,\d+,\d+,\d+\b", re.MULTILINE)

                # second part
                # VALUE "FileVersion", "6,0,20,163"
                # VALUE "ProductVersion", "6,0,20,163"
                regex_2 = re.compile(r"\b(VALUE\s*\"FileVersion\",\s*\"|VALUE\s*\"ProductVersion\",\s*\").*?(\")", re.MULTILINE)

                temp = buildversion
                version = r"\1 " + temp
                pass_1 = re.sub(regex_1, version, rc_content)
                temp = re.sub(",", ".", temp)
                pass_2 = re.sub(regex_2, r"\g<1>" + temp + r"\2", pass_1)

                # overwrite
                with open(rcfile, "r+", encoding="ISO-8859-1") as f:
                    f.seek(0)
                    f.write(pass_2)

        return True

    def build(self):      
        for project in self.projectlist:
            if project["isuse"] == "1":
                regex_1 = None
                projectextension = None
                buildpath = None
                filelist = os.listdir(project["project_path"])

                for filename in filelist:
                   if fnmatch.fnmatch(filename, '*.vcxproj'):
                       regex_1 = re.compile("SF_\w*.vcxproj")
                       buildversion = "4.0"
                       projectextension = ".vcxproj"
                       break
                   elif fnmatch.fnmatch(filename, '*.vcproj'):
                       regex_1 = re.compile("SF_\w*.vcproj")
                       buildversion = "2.0"
                       projectextension = ".vcproj"
                       # break가 없는 이유는 둘다 존재할 경우, vcxproj가 있으면 4.0으로 빌드하기 위함 

                for build in self.builpathlist:
                    if build["version"] == buildversion:
                        buildpath = build["path"]
                        break

                slnpath = project["sln_path"]
                projectpath = project["project_path"] + "\\" + project["name"] + projectextension
                result = regex_1.findall(projectpath)

                regex_2 = re.compile("[a-zA-Z_]+")
                configlist = regex_2.findall(project["configuration"])
 
                for config in configlist:
                    print("PROGRESSING BUILD - " + str(result) + " [" + config + "]\n\n")
                    
                    if buildversion == "4.0":
                        # Ensure msbuild file exists
                        if not os.path.isfile(buildpath):
                            raise Exception('MsBuild.exe not found. path=' + buildpath)
                        arg1 = '/t:Rebuild'
                        arg2 = '/p:Configuration=' + config
                        arg3 = '/clp:Summary;WarningsOnly'
                        arg4 = '/maxcpucount'
                        p = subprocess.call([buildpath, projectpath, arg1, arg2, arg3, arg4])
                    else:   
                        # Ensure devenv file exists
                        if not os.path.isfile(buildpath):
                            raise Exception('devenv.exe not found. path=' + buildpath)
                        arg1 = '/rebuild'
                        arg2 = config
                        arg3 = '/project'
                        arg4 = '/projectconfig'
                        p = subprocess.call([buildpath, slnpath, arg1, arg2, arg3, projectpath, arg2])
                    if p == 1: return False	# exit early
                    print("\n")
                        
        return True
     
    def validate(self, projectPath):
        packFile = os.path.dirname(projectPath) + '\\packages.config'
        if os.path.isfile(packFile):
        	f = open(packFile)
        	xml = f.read()
        	f.close()
        	print(xml)
        	match = re.search(r'version="0.0.0.0"', xml)
        	if match:
        		# Found a non-versioned package being used by this project
        		return False
        else:
        	print('No "packages.config" file was found. path=' + packFile)
        	
        return True
        
    def run(self, buildversion=None):
        summary = ''
        
        # File header
        start = datetime.datetime.now()
        print('\n' * 3)
        summary += self.log('STARTED BUILD - ' + start.strftime("%Y-%m-%d %H:%M:%S"))
        
        # Set version at rc file
        if buildversion is not None:
            settingOK = self.setversion(buildversion)
            if not settingOK:
            	self.log('BUILD: FAILED - FILE NOT FOUND', start)
            	sys.exit(100)
            summary += self.log('BUILD: SUCCEEDED - SET VERSION', start)
        else:
        	summary += self.log('BUILD: NOT SPECIFIED - BUILD VERSION', start)
        
        # Build the project
        buildOk = self.build()
        if not buildOk:
        	self.log('BUILD: FAILED', start)
        	sys.exit(100)
        summary += self.log('BUILD: SUCCEEDED', start)

        # Build footer
        #stop = datetime.datetime.now()
        #diff = stop - start
        summary += self.log('BUILD: *** FINISH ***', start)
        
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