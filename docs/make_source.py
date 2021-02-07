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
    outFile.write("   :caption: %s\n" % CAT_NAME)
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