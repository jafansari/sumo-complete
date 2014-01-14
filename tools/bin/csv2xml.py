#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
@file    csv2xml.py
@author  Jakob Erdmann
@date    2013-12-08
@version $Id$

Convert csv files to selected xml input files for SUMO

SUMO, Simulation of Urban MObility; see http://sumo-sim.org/
Copyright (C) 2008-2013 DLR (http://www.dlr.de/) and contributors

This file is part of SUMO.
SUMO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
"""

import os,sys
import csv

from collections import defaultdict
from optparse import OptionParser
import xml.sax

class LevelHandler(xml.sax.handler.ContentHandler):
    """A handler which knows the current nesting level"""
    def __init__(self):
        self.lvl = -1

    def startElement(self, name, attrs):
        self.lvl += 1

    def endElement(self, name):
        self.lvl -= 1

class AttrFinder(LevelHandler):
    def __init__(self):
        LevelHandler.__init__(self)
        self.lvlmax = self.lvl
        self.attrs = []
        self.knownAttrs = set()
        self.tagAttrs = defaultdict(set) # tag -> set of attrs
        self.renamedAttrs = {} # (name, attr) -> renamedAttr

    def startElement(self, name, attrs):
        LevelHandler.startElement(self, name, attrs)
        self.lvlmax = max(self.lvl, self.lvlmax)
        if self.lvl > 0:
            for a in attrs.keys():
                if not a in self.tagAttrs[name]:
                    self.tagAttrs[name].add(a)
                    if a in self.knownAttrs:
                        if not (name, a) in self.renamedAttrs:
                            anew = "%s_%s" % (name, a)
                            print("warning: renaming attribute %s of tag %s to %s" % (
                                a, name, anew))
                            self.renamedAttrs[(name, a)] = anew
                            self.knownAttrs.add(anew)
                            self.attrs.append(anew)
                    else:
                        self.knownAttrs.add(a)
                        self.attrs.append(a)


class CSVWriter(LevelHandler):
    def __init__(self, attrs, renamedAttrs, output_level, outfile, options):
        LevelHandler.__init__(self)
        self.attrs = attrs
        self.renamedAttrs = renamedAttrs
        self.output_level = output_level
        self.outfile = outfile
        self.options = options
        self.currentValues = defaultdict(lambda : "")

    def startElement(self, name, attrs):
        LevelHandler.startElement(self, name, attrs)
        if self.lvl > 0:
            for a, v in attrs.items():
                a = self.renamedAttrs.get((name, a), a)
                self.currentValues[a] = v
            if self.lvl == self.output_level:
                self.outfile.write(self.options.separator.join(
                    [self.currentValues[a] for a in self.attrs]) + "\n")


def get_options():
    optParser = OptionParser(usage="Usage: " + sys.argv[0])
    optParser.add_option("-v", "--verbose", action="store_true",
            default=False, help="Give more output")
    #optParser.add_option("-d", "--delimiter", default="\t",
    #         help="the field separator of the input file")
    #optParser.add_option("-q", "--quotechar", default="\"",
    #         help="the quoting character for fields")
    optParser.add_option("-d", "--delimiter", 
             help="the field separator of the input file")
    optParser.add_option("-n", "--nodes", 
             help="convert the given csv-file into plain.nod.xml format")
    optParser.add_option("-e", "--edges", 
             help="convert the given csv-file into plain.edg.xml format")
    optParser.add_option("-c", "--connections", 
             help="convert the given csv-file into plain.con.xml format")
    optParser.add_option("-r", "--routes", 
             help="convert the given csv-file into rou.xml format")
    optParser.add_option("-f", "--flows", 
             help="convert the given csv-file into rou.xml format with flows")
    options, args = optParser.parse_args()
    if len(args) != 0:
        optParse.print_help()
        sys.exit()
    return options 


def row2xml(row, tag):
    return ('    <%s %s/>\n' % (tag,
        ' '.join(['%s="%s"' % (a, v) for a,v in row.items() if v != ""])))

def row2vehicle_and_route(row, tag):
    if "route" in row:
        return row2xml(row, tag)
    else:
        edges = row.get("edges", "MISSING_VALUE")
        return ('    <%s %s>\n        <route edges="%s"/>\n    </%s>\n' % (
            tag, 
            ' '.join(['%s="%s"' % (a, v) for a,v in row.items() if v != ""]),
            edges, tag))

def write_xml(csvfile, toptag, tag, ext, options, printer=row2xml):
    outfilename = os.path.splitext(csvfile)[0] + ext
    dialect = csv.Sniffer().sniff(open(csvfile).read(1024))
    with open(outfilename, 'w') as outputf:
        outputf.write('<%s>\n' % toptag)
        with open(csvfile, 'rb') as inputf:
            for row in csv.DictReader(inputf, dialect=dialect):
                outputf.write(printer(row, tag))
        outputf.write('</%s>\n' % toptag)


def main():
    options = get_options()
    if options.nodes:
        write_xml(options.nodes, 'nodes', 'node', '.nod.xml', options)
    if options.edges:
        write_xml(options.edges, 'edges', 'edge', '.edg.xml', options)
    if options.connections:
        write_xml(options.connections, 'connections', 'connection', '.con.xml', options)
    if options.routes:
        write_xml(options.routes, 'routes', 'vehicle', '.rou.xml', options,
            row2vehicle_and_route)
    if options.flows:
        write_xml(options.flows, 'routes', 'flow', '.rou.xml', options,
            row2vehicle_and_route)


if __name__ == "__main__":
    main()
