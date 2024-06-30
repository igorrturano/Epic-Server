
function process_text(inspeech, mood, templatename)
  var retval;
  var elem;
  var tmpword;
  var words := SplitWords(inspeech);
  elem := speech_cfg[templatename];
  
  foreach word in words
    var wordgroup := GetConfigInt(elem, mood);
    if(!wordgroup)
      wordgroup := GetConfigInt(elem, "default");
    endif
    if(!wordgroup)
      wordgroup := templatename;
    endif
    elem := speech_cfg[wordgroup];
    tmpword := GetConfigString(elem,  word );
    if(tmpword)
      retval := tmpword;
    endif
  endforeach
  if(!retval)
    retval := GetConfigString(elem, "default" );
  endif
  if((!retval) || (retval == "5"))
    retval := "";
  endif
  return retval;
endfunction