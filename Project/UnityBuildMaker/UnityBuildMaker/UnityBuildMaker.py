import os,sys, xml, xml.dom.minidom, copy

### path
vcxprojPath = 'D:\\test\\test.vcxproj'
unityFolderPath = 'D:\\test\\UnityBuild'

### load xml
doc = xml.dom.minidom.parse(vcxprojPath)

### make config List 
configList = []
for config in doc.getElementsByTagName("ProjectConfiguration"):
    if -1 != config.getAttribute("Include").find("UnityBuild_"):
        configList.append("'$(Configuration)|$(Platform)'=='" + config.getAttribute("Include") + "'")

### make cpp List / add cpp file excluded
cppList = []    
for ClCompiles in doc.getElementsByTagName("ClCompile"):
    tempconfigList = copy.copy(configList)
    ### cpp file excluded
    ### UnityBuild 관련 Configuration 에서 모든 cpp 파일을 빌드 제외 
    if ClCompiles.getAttribute("Include"):
        ### UnityBuild 관련 cpp 파일이면 패스
        if -1 != ClCompiles.getAttribute("Include").find("UnityBuild"):
            continue

        lastNodeFlag = False
        lastNode = doc.createTextNode("\n    ")
        for childNode in ClCompiles.childNodes:
            if "ExcludedFromBuild" == childNode.localName:
                ### change exclude option
                ### 만약 이미 빌드 예외 관련 정보가 있다면, true 로 변경
                if -1 != childNode.getAttribute("Condition").find("UnityBuild_"):
                    childNode.firstChild.replaceWholeText("true")
                    del tempconfigList[tempconfigList.index(childNode.getAttribute("Condition"))]
            else:
                lastNodeFlag = True
                lastNode = childNode
                        
        ## add excluded option
        if 0 != len(tempconfigList):
            ## 빈칸 있으면 제거
            if lastNodeFlag:
                ClCompiles.removeChild(lastNode)
            
            ## 빌드 제외 구문 추가
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
