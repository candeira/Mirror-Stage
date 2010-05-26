#!/usr/bin/python

import os

BASEDIR = "../"

def extract_string(chapter):
    """
    Line length is a number encoded as the sixth space-separated string
    Line starts after the sixth space
    """
    tokens = chapter.split(' ')
    length = tokens[5]
    index = chapter.find(length) + len(length) + 1
    end = index + int(length)
    return chapter[index:end]
    
def extract_number(filename):
    l = filename.split('.')
    return int(l[0])

def allchapters(dirs):
    """
    Iterates over game levels xtracting & yielding full text of level description
    """
    for eachdir in dirs:
        for path, dirnames, filenames in os.walk(os.path.join(BASEDIR, eachdir)):
            dirnames.sort()
            filenames.sort(key=extract_number)
            for filename in filenames:
                if filename[-1] == "~":
                    continue
                print path, filename
                with open(os.path.join(path, filename)) as f:
                    chapter = f.read()
                yield (path, filename, chapter)

def print_grid(text, filepath, f):
    # for me to look at while I translate
    text = text.replace(" ", "_")
    print >> f, "#:", filepath[3:]
    print >> f
    while len(text) > 16:
        print >> f, text[:16]
        text = text[16:]
    print >> f, text
    print >> f
 

def print_pot(text, filepath, f):
    print >> f, "#:", filepath[3:]
    print >> f, 'msgid "%s"' % text
    print >> f, 'msgstr ""'
    print >> f

pot = open("chapters.pot", 'w')
grids = open("grids.txt", 'w')
for (path, filename, chapter) in allchapters(("chapters", "custom")):
    line = extract_string(chapter)
    filepath = os.path.join(path, filename)
    print_grid(line, filepath, grids)
    print_pot(line, filepath, pot)
pot.close()
grids.close()
    
