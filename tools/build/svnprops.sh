find tests -regex ".*\.\(xml\|complex\|dfrouter\|duarouter\|jtrrouter\|netconvert\|netgen\|od2trips\|polyconvert\|sumo\|meso\|tools\|traci\)" -print0 | xargs -0 svn ps svn:eol-style LF
find tests -regex ".*\.\(xml\|complex\|dfrouter\|duarouter\|jtrrouter\|netconvert\|netgen\|od2trips\|polyconvert\|sumo\|meso\|tools\|traci\)" -executable -print0 | xargs -0 svn pd svn:executable

find src tools tests -regex ".*\.\(cpp\|h\|py\|pl\|java\)" -print0 | xargs -0 svn ps svn:eol-style LF
find src tools tests -regex ".*\.\(cpp\|h\|py\|pl\|java\)" -print0 | xargs -0 svn ps svn:keywords "HeadURL Id LastChangedBy LastChangedDate LastChangedRevision"
find src tools -regex ".*\.\(cpp\|h\|java\)" -executable -print0 | xargs -0 svn pd svn:executable

