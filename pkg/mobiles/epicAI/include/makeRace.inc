include "include/client";

function makeRace(mobile, race := 0)
    if (!race)
        race := RandomInt(array{ANAO, ELFO, DROW, ORC, POLSKI, HUMANO});
    endif

   case (race)
      ANAO:   makeDwarven(mobile);
      ELFO:   makeElven(mobile);
      DROW:   makeDrow(mobile);
      ORC:    makeOrc(mobile);
      POLSKI: makePolski(mobile);
      default: makeHuman(mobile);
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

   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Get skin colors from config
   var skin_elem := skin_cfg["list"];
   var skin_colors := GetConfigStringArray(skin_elem, "color");
   mobile.color := CInt(skin_colors[RandomInt(skin_colors.size()) + 1]);
   mobile.truecolor := mobile.color;

   // Get hair colors from config
   var hair_elem := hair_cfg["Cabelo Anao"];
   var hair_colors := GetConfigStringArray(hair_elem, "Color");
   var hair_color := CInt(hair_colors[RandomInt(hair_colors.size()) + 1]);

   // Get hair styles from config
   var hair_types := types_cfg["todos"];
   var hair_styles := GetConfigStringArray(hair_types, "Style");
   var valid_hair_styles := array{};
   foreach style in hair_styles
      style := SplitWords(style);
      valid_hair_styles.append(CInt(style[2]));
   endforeach
   
   // Create and equip hair
   var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomInt(valid_hair_styles.size()) + 1], 1);
   hair.color := hair_color;
   EquipItem(mobile, hair);

   // Get beard styles for males
   if (mobile.gender == 0)
      var beard_types := types_cfg["dwarwen_beard"];
      var beard_styles := GetConfigStringArray(beard_types, "Style");
      var valid_beard_styles := array{};
      foreach style in beard_styles
         style := SplitWords(style);
         valid_beard_styles.append(CInt(style[2]));
      endforeach
      
      // Create and equip beard
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

   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Get skin colors from config
   var skin_elem := skin_cfg["elfo"];
   var skin_colors := GetConfigStringArray(skin_elem, "color");
   mobile.color := CInt(skin_colors[RandomInt(skin_colors.size()) + 1]);
   mobile.truecolor := mobile.color;

   // Get hair colors from config
   var hair_elem := hair_cfg["Cabelo Elfo"];
   var hair_colors := GetConfigStringArray(hair_elem, "Color");
   var hair_color := CInt(hair_colors[RandomInt(hair_colors.size()) + 1]);

   // Get hair styles from config
   var hair_types := types_cfg["todos"];
   var hair_styles := GetConfigStringArray(hair_types, "Style");
   var valid_hair_styles := array{};
   foreach style in hair_styles
      style := SplitWords(style);
      valid_hair_styles.append(CInt(style[2]));
   endforeach
   
   // Create and equip hair
   var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomInt(valid_hair_styles.size()) + 1], 1);
   hair.color := hair_color;
   EquipItem(mobile, hair);

   return 1;
endfunction

function makeDrow(mobile)
   Print("Making Drow - Gender: " + mobile.gender);
   
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 1832; // Male drow graphic
      Print("Setting male drow graphic: 1832");
   else
      mobile.graphic := 1833; // Female drow graphic
      Print("Setting female drow graphic: 1833");
   endif
   Print("Current graphic: " + mobile.graphic);

   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");
   Print("Configs loaded");

   // Get skin colors from config
   var skin_elem := skin_cfg["drow"];
   Print("Skin element: " + skin_elem);
   var skin_colors := GetConfigStringArray(skin_elem, "color");
   Print("Available skin colors: " + skin_colors.size());
   mobile.color := CInt(skin_colors[RandomIntMinMax(1, skin_colors.size())]);
   Print("Selected skin color: " + mobile.color);
   mobile.truecolor := mobile.color;

   // Get hair colors from config
   var hair_elem := hair_cfg["Cabelo Drow"];
   Print("Hair element: " + hair_elem);
   var hair_colors := GetConfigStringArray(hair_elem, "Color");
   Print("Available hair colors: " + hair_colors.size());
   var hair_color := CInt(hair_colors[RandomIntMinMax(1, hair_colors.size())]);
   Print("Selected hair color: " + hair_color);

   // Get hair styles from config
   var hair_types := types_cfg["todos"];
   var hair_styles := GetConfigStringArray(hair_types, "Style");
   var valid_hair_styles := array{};
   foreach style in hair_styles
      style := SplitWords(style);
      valid_hair_styles.append(CInt(style[2]));
   endforeach
   Print("Available hair styles: " + valid_hair_styles.size());
   
   // Create and equip hair
   var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomIntMinMax(1, valid_hair_styles.size())], 1);
   hair.color := hair_color;
   EquipItem(mobile, hair);
   Print("Hair equipped with color: " + hair.color);

   return 1;
endfunction

function makeOrc(mobile)
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 1834; // Male orc graphic
   else
      mobile.graphic := 1835; // Female orc graphic
   endif

   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Get skin colors from config
   var skin_elem := skin_cfg["orc"];
   var skin_colors := GetConfigStringArray(skin_elem, "color");
   mobile.color := CInt(skin_colors[RandomIntMinMax(1, skin_colors.size())]);
   mobile.truecolor := mobile.color;

   // Get hair colors from config
   var hair_elem := hair_cfg["Cabelo Orc"];
   var hair_colors := GetConfigStringArray(hair_elem, "Color");
   var hair_color := CInt(hair_colors[RandomIntMinMax(1, hair_colors.size())]);

   // Get hair styles from config
   var hair_types := types_cfg["todos"];
   var hair_styles := GetConfigStringArray(hair_types, "Style");
   var valid_hair_styles := array{};
   foreach style in hair_styles
      style := SplitWords(style);
      valid_hair_styles.append(CInt(style[2]));
   endforeach
   
   // Create and equip hair
   var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomIntMinMax(1, valid_hair_styles.size())], 1);
   hair.color := hair_color;
   EquipItem(mobile, hair);

   // Add beard for male orcs
   if (mobile.gender == 0)
      var beard_types := types_cfg["beard"];
      var beard_styles := GetConfigStringArray(beard_types, "Style");
      var valid_beard_styles := array{};
      foreach style in beard_styles
         style := SplitWords(style);
         valid_beard_styles.append(CInt(style[2]));
      endforeach
      
      // Create and equip beard
      var beard := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_beard_styles[RandomIntMinMax(1, valid_beard_styles.size())], 1);
      beard.color := hair_color;
      EquipItem(mobile, beard);
   endif

   return 1;
endfunction

function makePolski(mobile)
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 1836; // Male polski graphic
   else
      mobile.graphic := 1837; // Female polski graphic
   endif

   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Get skin colors from config
   var skin_elem := skin_cfg["list"];
   var skin_colors := GetConfigStringArray(skin_elem, "color");
   mobile.color := CInt(skin_colors[RandomIntMinMax(1, skin_colors.size())]);
   mobile.truecolor := mobile.color;

   // Get hair colors from config
   var hair_elem := hair_cfg["Index"];
   var hair_colors := GetConfigStringArray(hair_elem, "Color");
   var hair_color := CInt(hair_colors[RandomIntMinMax(1, hair_colors.size())]);

   // Get hair styles from config
   var hair_types := types_cfg["todos"];
   var hair_styles := GetConfigStringArray(hair_types, "Style");
   var valid_hair_styles := array{};
   foreach style in hair_styles
      style := SplitWords(style);
      valid_hair_styles.append(CInt(style[2]));
   endforeach
   
   // Create and equip hair
   var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomIntMinMax(1, valid_hair_styles.size())], 1);
   hair.color := hair_color;
   EquipItem(mobile, hair);

   return 1;
endfunction

function makeHuman(mobile)
   // Set base graphics by gender
   if (mobile.gender == 0) // Male
      mobile.graphic := 400; // Male human graphic
   else
      mobile.graphic := 401; // Female human graphic
   endif

   // Get configs
   var skin_cfg := ReadConfigFile(":fls_core:config/skincolors");
   var hair_cfg := ReadConfigFile(":hair:config/hairColors");
   var types_cfg := ReadConfigFile(":fls_core:config/hairtypes");

   // Get skin colors from config
   var skin_elem := skin_cfg["list"];
   var skin_colors := GetConfigStringArray(skin_elem, "color");
   mobile.color := CInt(skin_colors[RandomIntMinMax(1, skin_colors.size())]);
   mobile.truecolor := mobile.color;

   // Get hair colors from config
   var hair_elem := hair_cfg["Imperial"];
   var hair_colors := GetConfigStringArray(hair_elem, "Color");
   var hair_color := CInt(hair_colors[RandomIntMinMax(1, hair_colors.size())]);

   // Get hair styles from config
   var hair_types := types_cfg["todos"];
   var hair_styles := GetConfigStringArray(hair_types, "Style");
   var valid_hair_styles := array{};
   foreach style in hair_styles
      style := SplitWords(style);
      valid_hair_styles.append(CInt(style[2]));
   endforeach
   
   // Create and equip hair
   var hair := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_hair_styles[RandomIntMinMax(1, valid_hair_styles.size())], 1);
   hair.color := hair_color;
   EquipItem(mobile, hair);

   // Add beard for male humans (50% chance)
   if (mobile.gender == 0 && RandomInt(2) == 1)
      var beard_types := types_cfg["beard"];
      var beard_styles := GetConfigStringArray(beard_types, "Style");
      var valid_beard_styles := array{};
      foreach style in beard_styles
         style := SplitWords(style);
         valid_beard_styles.append(CInt(style[2]));
      endforeach
      
      // Create and equip beard
      var beard := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, valid_beard_styles[RandomIntMinMax(1, valid_beard_styles.size())], 1);
      beard.color := hair_color;
      EquipItem(mobile, beard);
   endif

   return 1;
endfunction

