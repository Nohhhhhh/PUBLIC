import os, json, collections, pprint, colorama, time
from colorama import init, Fore, Style
import msbuilder
import ftpmanager
import tortoisesvnmanager

colorama.init()

####################### 국가 정보 파일 로드 #######################
filepath_Info = os.path.dirname(os.path.realpath(__file__)) #파일이 위치한 디렉토리
filepath_Info = os.path.join(filepath_Info, "AD/Info.json")

with open(filepath_Info, encoding="utf-8") as data_file:
    jsondata_Info = json.load(data_file, object_pairs_hook=collections.OrderedDict)



print(colorama.Fore.CYAN + '\n*-----------------------------------------------------------------------------------------------*')
print('|                                                                                               |')
print('|                                      Auto Deployment                                          |')
print('|                                                                                               |')
print('|                                                                       ' + colorama.Style.BRIGHT + colorama.Fore.BLACK + 'created by sungjun ' + colorama.Fore.WHITE + 'noh' + colorama.Fore.BLACK +  '.' + colorama.Fore.CYAN + colorama.Style.NORMAL + ' |')
print('*-----------------------------------------------------------------------------------------------*')
print(colorama.Fore.RESET)
time.sleep(0.5)


####################### 국가 선택 #######################
print(Fore.CYAN + ' \nCountry Type (1 ~ %d)\n' % int(jsondata_Info["country"][0]["count"]))
for namecount in range(0, int(jsondata_Info["country"][0]["count"])):
    tempname = "country" + str(namecount + 1)
    print('  %d. %s' % (namecount + 1, str(jsondata_Info["country"][0][tempname])))
print(Fore.RESET)
    
selectcountry = ""
while True:
    countrytype = input('Select Country Type: ')
    if not countrytype.isdigit():
        print('Please input in the correct number. (1 ~ %d)\n' %int(jsondata_Info["country"][0]["count"]))
        continue
    countrytype = int(countrytype)
    if (countrytype < 1) | (countrytype > int(jsondata_Info["country"][0]["count"])):
        print('Please input in the correct number. (1 ~ %d)\n' %int(jsondata_Info["country"][0]["count"]))
    else:
        tempname = "country" + str(countrytype)
        selectcountry =  str(jsondata_Info["country"][0][tempname])
        break

############## Json load ##############
filepatherror = False
jsondecodeerror = False
filename = "AD/Info_" + selectcountry + ".json"

filepath_Info = os.path.dirname(os.path.realpath(__file__)) #파일이 위치한 디렉토리
filepath_Info = os.path.join(filepath_Info, filename)

with open(filepath_Info, encoding="utf-8") as data_file:
    jsondata_Info = json.load(data_file, object_pairs_hook=collections.OrderedDict)

############## MS Builder ##############
ms = msbuilder.MSBuilder(jsondata_Info["BUILD_INFO"]["build_path"], jsondata_Info["BUILD_INFO"]["build_project"])
ms.run(jsondata_Info["BUILD_INFO"]["build_version"])

############## FTP Upload ##############
if "1" == jsondata_Info["FTP_INFO"]["ftp_able"]:
    ftp = ftpmanager.FTPManager(jsondata_Info["FTP_INFO"]["ftp_connectinfo"]["host"], jsondata_Info["FTP_INFO"]["ftp_connectinfo"]["port"], jsondata_Info["FTP_INFO"]["ftp_connectinfo"]["id"], jsondata_Info["FTP_INFO"]["ftp_connectinfo"]["pass"])
    ftp.run(jsondata_Info["BUILD_INFO"]["build_project"], jsondata_Info["FTP_INFO"]["ftp_serverpath"], jsondata_Info["FTP_INFO"]["ftp_folder"])


############## SVN Commit ##############
if "1" == jsondata_Info["SVN_INFO"]["svn_able"]:
    svn = tortoisesvnmanager.TortoiseSVNManager(jsondata_Info["SVN_INFO"]["tortoisesvn_path"])
    svn.run(jsondata_Info["BUILD_INFO"]["build_version"], jsondata_Info["BUILD_INFO"]["build_project"], jsondata_Info["SVN_INFO"]["svn_commit_msg"])


print(colorama.Fore.CYAN + '\n\n\n*-----------------------------------------------------------------------------------------------*')
print('|                                          The End...                                           |')
print('|                                                                                               |')
print('|                                                                       ' + colorama.Style.BRIGHT + colorama.Fore.BLACK + 'created by sungjun ' + colorama.Fore.WHITE + 'noh' + colorama.Fore.BLACK +  '.' + colorama.Fore.CYAN + colorama.Style.NORMAL + ' |')
print('*-----------------------------------------------------------------------------------------------*')
print(colorama.Fore.RESET)

os.system("Pause")