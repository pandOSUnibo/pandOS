#  written by Jongkyu Kim (j.kim@fu-berlin.de)

import os
import sys

CAT_NAME = 'Modules'
INDEX_HEAD_PATH = 'index_head.rst'

def createIndex(inDir, outDir):
    listModules = []
    for fileName in os.listdir(inDir):
        listModules.append(fileName)
    listModules = sorted(listModules)

    outFile = open(outDir + "/index.rst", "w")

    inFile = open(INDEX_HEAD_PATH, "r")
    for line in inFile:
        outFile.write(line)
    inFile.close()
    outFile.write("\n")

    # toctree
    outFile.write(".. toctree::\n")
    outFile.write("   :caption: %s:\n" % CAT_NAME)
    outFile.write("   :titlesonly:\n")
    outFile.write("   :maxdepth: 2\n")
    #outFile.write("\n")
    for moduleName in listModules :
        noExtension = '.'.join(moduleName.split('.')[:-1])
        outFile.write("   %s\n" % (noExtension))
    #outFile.write("   :hidden:\n\n")
    outFile.close()

def createFile(original, target, outDir):
    original_name = '.'.join(original.split('.')[:-1])
    # title
    outFile = open(outDir + '/' + target, "w")
    outFile.write(original_name.upper() + "\n")
    outFile.write("=" * len(original_name) + "\n\n")

    rootDir = '/'.join(os.getcwd().split('/')[:-1])

    outFile.write(".. doxygenfile:: %s/h/%s\n" % (rootDir, original))
    outFile.write("   :project: pandOS\n\n")

    
    outFile.close()

def createFiles(inDir, outDir):
    listModules = []
    for fileName in os.listdir(inDir):
        listModules.append(fileName)
    listModules = sorted(listModules)


    for module in listModules:
        targetName = '.'.join(module.split('.')[:-1]) + '.rst'
        createFile(module, targetName, outDir)

inDir = sys.argv[1]
outDir = sys.argv[2]

createIndex(inDir, outDir)
createFiles(inDir, outDir)

"""
def generateIndex(inDir, outDir):
    listModules = []
    for fileName in os.listdir(inDir) :
        listModules.append(fileName)  
    listModules = sorted(listModules)

    # generate index.rst
    inFile = open(INDEX_TEMP, "r")
    outFile = open(outDir + "/index.rst", "w")
    for line in inFile :
        outFile.write(line)
    inFile.close()
    outFile.write("   :caption: %s:\n\n" % CAT_NAME)
    for moduleName in listModules :
        outFile.write("   %s\n" % (moduleName))
    outFile.close()

def generateRST(outDir, moduleName, listModules, listFiles) :
    if len(listModules) > 0 and os.path.isdir(outDir) == False:
        os.mkdir(outDir)

    # title
    outFile = open(outDir + ".rst","w")
    outFile.write(moduleName[0].upper() + moduleName[1:] + "\n")
    outFile.write("=" * len(moduleName) + "\n\n")

    # doxygenfile
    for fileName in listFiles :
        outFile.write(".. doxygenfile:: %s/%s\n" % (outDir[3:], fileName))
        outFile.write("   :project: myproject\n\n")

    # toctree
    outFile.write(".. toctree::\n")
    outFile.write("   :caption: %s:\n" % CAT_NAME)
    outFile.write("   :titlesonly:\n")
    outFile.write("   :maxdepth: 1\n")
    outFile.write("   :hidden:\n\n")
    for childModuleName in listModules :
       outFile.write("   %s/%s\n" % (moduleName, childModuleName) )
    outFile.close()

def generateRSTs(inDir, outDir, isRoot=False):
    listModules = []
    listFiles = []
    for fileName in os.listdir(inDir) :
        if os.path.isdir(inDir + "/" + fileName) == True:
            listModules.append(fileName)  
        else :
            fileExt = fileName.split(".")[-1]
            if fileExt == "h" or fileExt == "c" :
                print('Aggiunto %s' % fileName)
                listFiles.append(fileName)
    
    listModules = sorted(listModules)
    listFiles = sorted(listFiles)

    print isRoot, inDir, outDir, listModules, listFiles

    if isRoot == False :
        moduleName = outDir.split("/")[-1]
        generateRST(outDir, moduleName, listModules, listFiles)
    else:
        moduleName = 'h'
        generateRST(outDir, moduleName, listModules, listFiles)


    for moduleName in listModules :
        curInDir = inDir + "/" + moduleName
        curOutDir = outDir + "/" + moduleName
        generateRSTs(curInDir, curOutDir, False)

'''
Alphabet
========

.. doxygenfile:: alphabet.hpp
   :project: myproject

.. doxygenfile:: alphabet_container.hpp
   :project: myproject

.. doxygenfile:: compound_alphabet.hpp
   :project: myproject

.. toctree::
   :caption: Modules:
   :titlesonly:
   :maxdepth: 1
   :hidden:

   alphabet/aminoacid
   alphabet/gaps
   alphabet/nucleotide
'''


#    print listModules
#    print listFiles


###################
inDir = sys.argv[1]
outDir = sys.argv[2]

generateIndex(inDir, outDir)
generateRSTs(inDir, outDir, True)"""