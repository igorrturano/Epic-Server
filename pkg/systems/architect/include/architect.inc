// $Id: architect.inc,v 1.10 2006/07/19 06:35:18 bytehawk Exp $

/*
HILITE_MARK       This is the color that marked parts will be hilited with.
                  Since I'm colorblind, the default might hurt "normal" peoples eyes :)
LANGUAGE          Two languages are available atm: "ENG" and "GER". Should you
                  translate the entries in :architect:config/language.cfg to another
                  language, I'd ask you to email me the updated language.cfg for
                  including it in an updated version of Sigena architect.
MAXSAVES          The max number of designs each player can save. A high number would
                  overflow the gump and you'd have to script a pagination. So better
                  keep it below 20 or so :)
SEEPLAYERDESIGNS  staff with a least this level can load player designs
                  default: 3 (GM)
SITESIZE          The x and y size of the construction site.

startloc          The upper left corner of the construction site. The lower right corner
                  is calculated by adding SITESIZE to these coords.
websites          The URIs of wiki entries in the available languages. Again, should you
                  add an entry in a new language, please email me.
*/

const ARCHITECT_VERSION:= "1.11";
const CALLFROMCITY:= 0;           // call Architect from within cities only
const HILITE_MARK:= 42;           // color for highlighting marked parts
const LANGUAGE:= "PT";           // guess what :)  (DEU or ENG)
const MAXSAVES:= 10;              // maximum number of saved designs per player
const SEEPLAYERDESIGNS:= 5;       // cmdlevel needed to see player designs
const SITESIZE:= 30;              // height and width of construction site
const STAFFMODELEVEL:= 1;         // min level for staff mode

var startloc:= array{1510, 1242, 0};
var websites:= dictionary {"DEU" -> "http://www.uosigena.de/wiki/index.php/architect",
                           "ENG" -> "http://www.uosigena.de/wiki/index.php/architect_english"};

function GetResidenciasRegion(who)

  var df := opendatafile(":architect:areas");
  foreach elem in (df.Keys())
    var region := df.FindElement(elem);
    var rect := region.getProp("Rect");
    if (InRect(who, SplitWords(rect)))
      return elem;
    endif
  endforeach
  return "Whole_World";

endfunction

function InRect(who, rect)
	var x1 := CInt(rect[1]);
	var y1 := CInt(rect[2]);
	var x2 := CInt(rect[3]);
	var y2 := CInt(rect[4]);
  var realm := rect[5];

	return ( who.realm == realm && (CInt(who.x) >= x1 && CInt(who.x) <= x2) && (CInt(who.y) >= y1 && CInt(who.y) <= y2) );
endfunction
