
use uo;
use cfgfile;
include ":blood:blood";

function DesmembreCorpse(cadaver)
    SetObjProperty(cadaver, "morto", 1);
  
    var beardlist := {0x203e, 0x203f, 0x2040, 0x2041, 0x204b, 0x204c, 0x204d };
    var hairlist  := {0x2044, 0x2045, 0x2046, 0x2047, 0x2048, 0x2049, 0x204a, 0x203b, 0x203c, 0x203d };
//    foreach thing in EnumerateItemsInContainer(cadaver)
 //     if((thing.objtype in hairlist) or (thing.objtype in beardlist))
 //       DestroyItem(thing);
//      else
//        MoveObjectToLocation(thing, cadaver.x, cadaver.y, cadaver.z, thing.realm, MOVEOBJECT_FORCELOCATION);
//      endif
 //   endforeach
    var partsarray := {};
    var part;
    part := CreateItemAtLocation(cadaver.x-1, cadaver.y, cadaver.z, 0x1da0, 1);
    partsarray.append(part);
    SetObjProperty(part, "serial", GetObjProperty(cadaver, "serial"));
    var aname := cadaver.name;
    aname["A corpse of"] := "";
    part.name := "A cabeca de " + aname;
//    part := CreateItemAtLocation(cadaver.x-1, cadaver.y, cadaver.z, 0x1da1, 1);
 //   partsarray.append(part);
  //  part := CreateItemAtLocation(cadaver.x-1, cadaver.y+1, cadaver.z, 0x1da2, 1);
  //  partsarray.append(part);
  //  part := CreateItemAtLocation(cadaver.x+1, cadaver.y, cadaver.z, 0x1da3, 1);
  //  partsarray.append(part);
  //  part := CreateItemAtLocation(cadaver.x+1, cadaver.y+1, cadaver.z, 0x1da4, 1);
  //  partsarray.append(part);
  //  part := CreateItemAtLocation(cadaver.x, cadaver.y, cadaver.z, 0x1d9f, 1);
  //  partsarray.append(part);
    MakeBleed(cadaver);
  //  DestroyItem(cadaver);
    
endfunction

function GetSkinningDifficulty(corpseitm)

	var skin_cfg := ReadConfigFile(":survival:config/skinning");
	var diff_elem := FindConfigElem(skin_cfg, "SkinningDifficulty");
	return cint( GetConfigInt(diff_elem, corpseitm) );

endfunction

function GetMinSkinningAmt(corpseitm)

	var skin_cfg := ReadConfigFile(":survival:config/skinning");
	var diff_elem := FindConfigElem(skin_cfg, "SkinningAmount");
	return cint(GetConfigInt(diff_elem, corpseitm) );

endfunction
