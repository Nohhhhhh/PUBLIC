import os,sys, xml, xml.dom.minidom, copy

### path
vcxprojPath = 'D:\\01_DF\\sf_gameserver_kor\\trunk\\Source\\SF_GameServer\\SF_GameServer.vcxproj'
unityFolderPath = 'D:\\01_DF\\sf_gameserver_kor\\trunk\\Source\\SF_GameServer\\UnityBuild'

### load xmlv
doc = xml.dom.minidom.parse(vcxprojPath)

### make config List 
configList = []
for config in doc.getElementsByTagName("ProjectConfiguration"):
    if -1 != config.getAttribute("Include").find("UnityBuild_"):
        configList.append("'$(Configuration)|$(Platform)'=='" + config.getAttribute("Include") + "'")

### make cpp List / add cpp file excluded
cppList = []    
isPCHFile = False
for ClCompiles in doc.getElementsByTagName("ClCompile"):
    tempconfigList = copy.copy(configList)
    ### cpp file excluded
    if ClCompiles.getAttribute("Include"):
        if -1 != ClCompiles.getAttribute("Include").find("UnityBuild"):
            continue

        if -1 != ClCompiles.getAttribute("Include").find("stdafx"):
            isPCHFile = True

        lastNodeFlag = False
        lastNode = doc.createTextNode("\n    ")
        for childNode in ClCompiles.childNodes:
            if "ExcludedFromBuild" == childNode.localName:
                ## change exclude option
                if -1 != childNode.getAttribute("Condition").find("UnityBuild_"):
                    childNode.firstChild.replaceWholeText("true")
                    del tempconfigList[tempconfigList.index(childNode.getAttribute("Condition"))]
            else:
                lastNodeFlag = True
                lastNode = childNode
                        
        ## add excluded option
        if 0 != len(tempconfigList):
            if lastNodeFlag:
                ClCompiles.removeChild(lastNode)

            for addNode in tempconfigList:
                txt = doc.createTextNode("\n      ")
                ClCompiles.appendChild(txt)
                addElement = doc.createElementNS(ClCompiles.namespaceURI,"ExcludedFromBuild")
                addElement.setAttribute("Condition", addNode)
                txt = doc.createTextNode("true")
                addElement.appendChild(txt)
                ClCompiles.appendChild(addElement)

            ClCompiles.appendChild(lastNode)

        cppList.append(ClCompiles.getAttribute("Include"))

### save xml
with open(vcxprojPath, "w", encoding = 'utf-8') as xml_file:
    doc.writexml(xml_file)

### make include file
if not os.path.exists(unityFolderPath):
    os.mkdir(unityFolderPath)

idx = 0
cppCnt = 0
unityFile = unityFolderPath + "\\UnityBuild_%02d.cpp" % idx
with open(unityFile, "wt") as f:
    f.write('#include "..\stdafx.h"\n')
    f.write('#include "..\stdafx.cpp"\n')
    idx += 1

unityFile = unityFolderPath + "\\UnityBuild_%02d.cpp" % idx
f = open(unityFile, "wt")
f.write('#include "..\stdafx.h"\n\n')
for cppName in cppList:
    if (0 != cppCnt) and (0 == (cppCnt % 40)):
        f.close()
        cppCnt = 0
        idx += 1
        unityFile = unityFolderPath + "\\UnityBuild_%02d.cpp" % idx
        f = open(unityFile, "wt")
        f.write('#include "..\stdafx.h"\n\n')

    f.write('#include "..\%s"\n' % cppName)
    cppCnt += 1
f.close()
