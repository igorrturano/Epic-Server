include "include/client";
include ":epicAI:include/npcDataCache";    // NPC_GetCachedData

function HasHair(mobile)
    return GetEquipmentByLayer(mobile, LAYER_HAIR);
endfunction

function HasBeard(mobile)
    return GetEquipmentByLayer(mobile, LAYER_BEARD);
endfunction

function makeRace(mobile, race := 0)
   if (!race)
      var races := array{ANAO, ELFO, DROW, ORC, PEQUILHO, HUMANO};
      race := races[RandomInt(races.size()) + 1];
   endif
   
   case (race)
      ANAO:   makeDwarven(mobile);
      ELFO:   makeElven(mobile);
      DROW:   makeDrow(mobile);
      ORC:    makeOrc(mobile);
      PEQUILHO: makePequilho(mobile);
      HUMANO: makeHuman(mobile);
   endcase

   SetObjProperty(mobile, "myrace", race);
   return 1;
endfunction

function makeDwarven(mobile)
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 1828;
   else
      mobile.graphic := 1829;
   endif

   // Get NPC config to check for existing colors - O(1) cache lookup
   var cache := NPC_GetCachedData(mobile);
   var npc_elem := cache.npc_elem;

   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Set skin color - use NPC config if available, otherwise random
   if (npc_elem.Color || npc_elem.TrueColor)
      if (npc_elem.Color)
         mobile.color := CInt(npc_elem.Color);
      endif
      if (npc_elem.TrueColor)
         mobile.truecolor := CInt(npc_elem.TrueColor);
      else
         mobile.truecolor := mobile.color;
      endif
   else
      // Get skin colors from config
      var skin_elem := skin_cfg["list"];
      var skin_colors := GetConfigStringArray(skin_elem, "color");
      mobile.color := CInt(skin_colors[RandomInt(skin_colors.size()) + 1]);
      mobile.truecolor := mobile.color;
   endif

   var hair_elem := hair_cfg["Cabelo Anao"];
   var hair_colors := GetConfigStringArray(hair_elem, "Color");
   var hair_color := CInt(hair_colors[RandomInt(hair_colors.size()) + 1]);

   if (!HasHair(mobile))
      var hair_types := types_cfg["todos"];
      var hair_styles := GetConfigStringArray(hair_types, "Style");
      var valid_hair_styles := array{};
      foreach style in hair_styles
         style := SplitWords(style);
         valid_hair_styles.append(CInt(style[2]));
      endforeach
      var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomInt(valid_hair_styles.size()) + 1], 1);
      hair.color := hair_color;
      EquipItem(mobile, hair);
   endif

   if (isMale(mobile) == 1 && !HasBeard(mobile))
      var beard_types := types_cfg["dwarwen_beard"];
      var beard_styles := GetConfigStringArray(beard_types, "Style");
      var valid_beard_styles := array{};
      foreach style in beard_styles
         style := SplitWords(style);
         valid_beard_styles.append(CInt(style[2]));
      endforeach
      var beard := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_beard_styles[RandomInt(valid_beard_styles.size()) + 1], 1);
      beard.color := hair_color;
      EquipItem(mobile, beard);
   endif

   return 1;
endfunction

function makeElven(mobile)
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 1830; // Male elf graphic
   else
      mobile.graphic := 1831; // Female elf graphic
   endif

   // Get NPC config to check for existing colors - O(1) cache lookup
   var cache := NPC_GetCachedData(mobile);
   var npc_elem := cache.npc_elem;

   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Set skin color - use NPC config if available, otherwise random
   if (npc_elem.Color || npc_elem.TrueColor)
      if (npc_elem.Color)
         mobile.color := CInt(npc_elem.Color);
      endif
      if (npc_elem.TrueColor)
         mobile.truecolor := CInt(npc_elem.TrueColor);
      else
         mobile.truecolor := mobile.color;
      endif
   else
      // Get skin colors from config
      var skin_elem := skin_cfg["elfo"];
      var skin_colors := GetConfigStringArray(skin_elem, "color");
      mobile.color := CInt(skin_colors[RandomInt(skin_colors.size()) + 1]);
      mobile.truecolor := mobile.color;
   endif

   if (!HasHair(mobile))
      var hair_elem := hair_cfg["Cabelo Elfo"];
      var hair_colors := GetConfigStringArray(hair_elem, "Color");
      var hair_color := CInt(hair_colors[RandomInt(hair_colors.size()) + 1]);
      var hair_types := types_cfg["todos"];
      var hair_styles := GetConfigStringArray(hair_types, "Style");
      var valid_hair_styles := array{};
      foreach style in hair_styles
         style := SplitWords(style);
         valid_hair_styles.append(CInt(style[2]));
      endforeach
      var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomInt(valid_hair_styles.size()) + 1], 1);
      hair.color := hair_color;
      EquipItem(mobile, hair);
   endif

   return 1;
endfunction

function makeDrow(mobile)
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 1830; // Male elf graphic
   else
      mobile.graphic := 1831; // Female elf graphic
   endif

   // Get NPC config to check for existing colors
   var cache := NPC_GetCachedData(mobile);
   var npc_elem := cache.npc_elem;
   
   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Set skin color - use NPC config if available, otherwise random
   if (npc_elem.Color || npc_elem.TrueColor)
      if (npc_elem.Color)
         mobile.color := CInt(npc_elem.Color);
      endif
      if (npc_elem.TrueColor)
         mobile.truecolor := CInt(npc_elem.TrueColor);
      else
         mobile.truecolor := mobile.color;
      endif
   else
      // Get skin colors from config
      var skin_elem := skin_cfg["drow"];
      var skin_colors := GetConfigStringArray(skin_elem, "color");
      mobile.color := CInt(skin_colors[RandomIntMinMax(1, skin_colors.size())]);
      mobile.truecolor := mobile.color;
   endif

   // Get hair colors from config
   if (!HasHair(mobile))
      var hair_elem := hair_cfg["Cabelo Drow"];
      var hair_colors := GetConfigStringArray(hair_elem, "Color");
      var hair_color := CInt(hair_colors[RandomIntMinMax(1, hair_colors.size())]);
      var hair_types := types_cfg["todos"];
      var hair_styles := GetConfigStringArray(hair_types, "Style");
      var valid_hair_styles := array{};
      foreach style in hair_styles
         style := SplitWords(style);
         valid_hair_styles.append(CInt(style[2]));
      endforeach
      var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomIntMinMax(1, valid_hair_styles.size())], 1);
      hair.color := hair_color;
      EquipItem(mobile, hair);
   endif

   return 1;
endfunction

function makeOrc(mobile)
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 1832; // Male orc graphic
   else
      mobile.graphic := 1833; // Female orc graphic
   endif

   // Get NPC config to check for existing colors
   var cache := NPC_GetCachedData(mobile);
   var npc_elem := cache.npc_elem;
   
   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Set skin color - use NPC config if available, otherwise random
   if (npc_elem.Color || npc_elem.TrueColor)
      if (npc_elem.Color)
         mobile.color := CInt(npc_elem.Color);
      endif
      if (npc_elem.TrueColor)
         mobile.truecolor := CInt(npc_elem.TrueColor);
      else
         mobile.truecolor := mobile.color;
      endif
   else
      // Get skin colors from config
      var skin_elem := skin_cfg["orc"];
      var skin_colors := GetConfigStringArray(skin_elem, "Color");
      mobile.color := CInt(skin_colors[RandomIntMinMax(1, skin_colors.size())]);
      mobile.truecolor := mobile.color;
   endif

   // Get hair colors from config
   var hair_elem := hair_cfg["Cabelo Orc"];
   var hair_colors := GetConfigStringArray(hair_elem, "Color");
   var hair_color := CInt(hair_colors[RandomIntMinMax(1, hair_colors.size())]);

   if (!HasHair(mobile))
      var hair_types := types_cfg["todos"];
      var hair_styles := GetConfigStringArray(hair_types, "Style");
      var valid_hair_styles := array{};
      foreach style in hair_styles
         style := SplitWords(style);
         valid_hair_styles.append(CInt(style[2]));
      endforeach
      var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomIntMinMax(1, valid_hair_styles.size())], 1);
      hair.color := hair_color;
      EquipItem(mobile, hair);
   endif

   // Add beard for male orcs
   if (isMale(mobile) == 1 && RandomInt(2) == 1 && !HasBeard(mobile))
      var beard_types := types_cfg["beard"];
      var beard_styles := GetConfigStringArray(beard_types, "Style");
      var valid_beard_styles := array{};
      foreach style in beard_styles
         style := SplitWords(style);
         valid_beard_styles.append(CInt(style[2]));
      endforeach
      var beard := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_beard_styles[RandomIntMinMax(1, valid_beard_styles.size())], 1);
      beard.color := hair_color;
      EquipItem(mobile, beard);
   endif

   return 1;
endfunction

function makePequilho(mobile)
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 1834;
   else
      mobile.graphic := 1835;
   endif

   // Get NPC config to check for existing colors
   var cache := NPC_GetCachedData(mobile);
   var npc_elem := cache.npc_elem;
   
   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Set skin color - use NPC config if available, otherwise random
   if (npc_elem.Color || npc_elem.TrueColor)
      if (npc_elem.Color)
         mobile.color := CInt(npc_elem.Color);
      endif
      if (npc_elem.TrueColor)
         mobile.truecolor := CInt(npc_elem.TrueColor);
      else
         mobile.truecolor := mobile.color;
      endif
   else
      // Get skin colors from config
      var skin_elem := skin_cfg["list"];
      var skin_colors := GetConfigStringArray(skin_elem, "color");
      mobile.color := CInt(skin_colors[RandomIntMinMax(1, skin_colors.size())]);
      mobile.truecolor := mobile.color;
   endif

   // Get hair colors from config
   var hair_elem := hair_cfg["Index"];
   if (!HasHair(mobile))
      var hair_colors := GetConfigStringArray(hair_elem, "Color");
      var hair_color := CInt(hair_colors[RandomIntMinMax(1, hair_colors.size())]);
      var hair_types := types_cfg["todos"];
      var hair_styles := GetConfigStringArray(hair_types, "Style");
      var valid_hair_styles := array{};
      foreach style in hair_styles
         style := SplitWords(style);
         valid_hair_styles.append(CInt(style[2]));
      endforeach
      var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomIntMinMax(1, valid_hair_styles.size())], 1);
      hair.color := hair_color;
      EquipItem(mobile, hair);
   endif

   return 1;
endfunction

function makeHuman(mobile)
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 400; // Male human graphic
   else
      mobile.graphic := 401; // Female human graphic
   endif

   // Get NPC config to check for existing colors
   var cache := NPC_GetCachedData(mobile);
   var npc_elem := cache.npc_elem;
   
   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Set skin color - use NPC config if available, otherwise random
   if (npc_elem.Color || npc_elem.TrueColor)
      if (npc_elem.Color)
         mobile.color := CInt(npc_elem.Color);
      endif
      if (npc_elem.TrueColor)
         mobile.truecolor := CInt(npc_elem.TrueColor);
      else
         mobile.truecolor := mobile.color;
      endif
   else
      // Get skin colors from config
      var skin_elem := skin_cfg["list"];
      var skin_colors := GetConfigStringArray(skin_elem, "color");
      mobile.color := CInt(skin_colors[RandomIntMinMax(1, skin_colors.size())]);
      mobile.truecolor := mobile.color;
   endif

   var hair_elem := hair_cfg["Imperial"];
   var hair_colors := GetConfigStringArray(hair_elem, "Color");
   var hair_color := CInt(hair_colors[RandomIntMinMax(1, hair_colors.size())]);

   if (!HasHair(mobile))
      var hair_types := types_cfg["todos"];
      var hair_styles := GetConfigStringArray(hair_types, "Style");
      var valid_hair_styles := array{};
      foreach style in hair_styles
         style := SplitWords(style);
         valid_hair_styles.append(CInt(style[2]));
      endforeach
      var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomIntMinMax(1, valid_hair_styles.size())], 1);
      hair.color := hair_color;
      EquipItem(mobile, hair);
   endif

   if (isMale(mobile) == 1 && RandomInt(2) == 1 && !HasBeard(mobile))
      var beard_types := types_cfg["beard"];
      var beard_styles := GetConfigStringArray(beard_types, "Style");
      var valid_beard_styles := array{};
      foreach style in beard_styles
         style := SplitWords(style);
         valid_beard_styles.append(CInt(style[2]));
      endforeach
      var beard := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_beard_styles[RandomIntMinMax(1, valid_beard_styles.size())], 1);
      beard.color := hair_color;
      EquipItem(mobile, beard);
   endif

   return 1;
endfunction

function isMale(mobile)
   var female := GetObjProperty(mobile, "female");   
   var isBoss := GetObjProperty(mobile, "EpicBossStruct");
   var result := 0;
   if (female || mobile.gender == 1 || Cint(isBoss.gender) == 1)
      result := 0;
   else 
      result := 1;
   endif
   
   return result;
endfunction