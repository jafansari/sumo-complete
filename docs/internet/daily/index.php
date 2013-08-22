<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html style="width: 100%">

<head>
 <title>SUMO - Status of daily build and tests</title>
 <link rel="stylesheet" type="text/css" href="../sumo.css"/>

  <link rel="schema.DC" href="http://purl.org/dc/elements/1.1/" />
  <META NAME="DC.Title" CONTENT="SUMO - Status of daily build and tests">
  <META NAME="DC.Subject" CONTENT="road traffic simulation package SUMO (Simulation of Urban MObility)">
  <META NAME="DC.Description" CONTENT="">
  <META NAME="DC.Publisher" CONTENT="Institute of Transportation Systems at the German Aerospace Center">
  <META NAME="DC.Type" CONTENT="Text">
  <META NAME="DC.Format" SCHEME="IMT" CONTENT="text/html">
  <META NAME="DC.Identifier" CONTENT="http://sumo-sim.org">
  <META NAME="DC.Language" SCHEME="ISO639-1" CONTENT="en">
  <META NAME="DC.Relation" SCHEME="URL" CONTENT="http://sumo-sim.org/index.html">
  <META NAME="DC.Rights" CONTENT="(c) ITS/DLR">
  <META NAME="DC.Date.X-MetadataLastModified" SCHEME="ISO8601" CONTENT="2011-03-04">
  <META http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
</head>

<body id="bmission" style="width: 100%">
<div id="container" style="width: 100%">



 <div id="logo" align="left"><img src="../logo.png" width="128" height="128" align="left"
     alt="SUMO-Simulation of Urban Mobility" style="margin-left:10px;
     margin-right:20px;"/>
     <div id="logo_text1"> SUMO </div>
     <div id="logo_text2"> Simulation of Urban MObility</div>
 </div>

 <div id="subsMenu"><ul>
  <li class="sub"><a href="/">Home</a></li>
  <li class="sub"><a href="/wiki/Downloads">Download</a></li>
  <li class="sub"><a href="/userdoc/">Documentation</a></li>
  <li class="sub"><a href="/wiki/">Wiki</a></li>
  <li class="sub"><a href="/trac/">Trac</a></li>
  <li class="sub"><a href="/blog/">Blog</a></li>
  <li class="sub"><a href="/userdoc/Contact.html">Contact</a></li>
  <li class="sub"><a href="http://sourceforge.net/projects/sumo/">SF-Project</a></li>
 </ul></div>

    <h2>Status of daily build and tests</h2>
        <table>
<?PHP
$column = 0;
$cells[$column][0] = "";
$cells[$column][1] = "Standard build";
$cells[$column][2] = "Texttest tests";
$cells[$column][3] = "Debug build";
$column++;

foreach (glob("*status.log") as $filename) {
    $prefix = substr($filename, 0, strpos($filename, "status.log"));
    $cells[$column][0] = '';
    $cells[$column][1] = '';
    $cells[$column][3] = '';
    $row = 0;
    $statusdata = file($filename);
    foreach ($statusdata as $line) {
        if (chop($line) == "--") {
            if (strstr($cells[$column][$row], "<pre>")) {
                $cells[$column][$row] .= "</pre></a>";
            }
            $row++;
            continue;
        }
        $br = strstr($line, "batchreport");
        if ($br) {
            $br_arr = split(' ', $br);
            $br_sub = explode('.', substr($br_arr[0], 12));
            if (count($br_sub) == 1) {
                $br_dir = $br_sub[0].'/test_default.html';
            } else {
                $br_dir = $br_sub[0].'/test_default.html#'.$br_sub[1];
            }
            $cells[$column][$row] .= '<a href="'.$prefix.'report/'.$br_dir.'">'.substr($br, 12).'</a><br/>';
        } else {
            if ($cells[$column][$row] == '' && $row > 0) {
                $cells[$column][$row] = '<a href="'.$line.'"><pre>';
            } else {
                $cells[$column][$row] .= $line;
            }
        }
    }

    $column++;
}
for ($j = 0; $j < count($cells[0]); $j++) {
    echo "<tr>";
    for ($i = 0; $i < $column; $i++) {
        if ($i == 0 || $j == 0) {
            echo "<th>".$cells[$i][$j]."</th>\n";
        } else {
            echo "<td>".$cells[$i][$j]."</td>\n";
        }
    }
    echo "</tr>\n";
}
?> 
        </table>
 </div>

 <div id="footer">
   <div>(c) 2011-2013, German Aerospace Center, Institute of Transportation Systems</div>
   <div>Layout based on <a href="http://www.oswd.org/design/preview/id/3365">"Three Quarters"</a> by "SimplyGold"</div>
 </div>
                                                       
 
</div></body>

</html>
