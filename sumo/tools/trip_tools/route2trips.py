#!/usr/bin/python
# This script converts SUMO routes back into SUMO trips which serve
# as input to one of the routing applications.
# It reads the routes from a file given as first parameter
# and outputs the trips to stdout.
import sys

from xml.sax import make_parser, handler

class RouteReader(handler.ContentHandler):

    def __init__(self):
        self._vType = ''
        self._vID = ''
        self._routeID = ''
        self._routeString = ''
	self._routes = {}
        
    def startElement(self, name, attrs):
        if name == 'vehicle':
            self._vType = attrs['type']
            self._vID = attrs['id']
            self._vDepart = attrs['depart']
	    if 'route' in attrs:
        	self._routeString = self._routes[attrs['route']]
        elif name == 'route':
            if not self._vID:
                self._routeID = attrs['id']
            self._routeString = ''
        elif name == 'routes':
	    print "<tripdefs>"

    def endElement(self, name):
        if name == 'route':
            if not self._vID:
    		self._routes[self._routeID] = self._routeString
                self._routeString = ''
            self._routeID = ''
        elif name == 'vehicle':
	    edges = self._routeString.split()
	    print '    <tripdef depart="%s" from="%s" to="%s" type="%s"/>'\
	          % (self._vDepart, edges[0], edges[-1], self._vType)
            self._vType = ''
            self._vID = ''
            self._routeString = ''
        elif name == 'routes':
	    print "</tripdefs>"

    def characters(self, content):
        self._routeString += content


            
if len(sys.argv) < 2:
    print "Usage: " + sys.argv[0] + " <routes>"
    sys.exit()
parser = make_parser()
parser.setContentHandler(RouteReader())
parser.parse(sys.argv[1])
