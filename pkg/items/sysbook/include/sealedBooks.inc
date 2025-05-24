use uo;
use os;
use util; // Adiciona RandomInt
use polsys;
use datafile;

const SEALED_BOOKS_DATAFILE := ":sysbook:sealedbooks";

// Constante para verificar se é um livro
// Se POLCLASS_BOOK não existir, use uma constante alternativa ou remova a verificação
const POLCLASS_BOOK := 0; // Valor padrão caso não exista na sua configuração

// Função para tentar criar um livro usando diferentes estratégias
function CreateBookInContainer(container)
    var book := CreateItemInContainer(container, 0xFEF);
    if (book && !book.errortext)
        return book;
    else
        return error{"errortext" := "Couldn't create 0xFEF"};
    endif
endfunction

// Retorna um serial aleatório de livro lacrado disponível
function SealedBooks_GetRandomSerial()
    var df := OpenDataFile(SEALED_BOOKS_DATAFILE);
    if (df.errortext) // Corrected check
        return 0;
    endif

    var keys := df.Keys();
    
    if (!keys.size())
        
        return 0;
    endif

    var idx := RandomInt(keys.size()) + 1;
    var serial := keys[idx];
    return serial;
endfunction

// Função para definir livro como somente leitura
function SetBookAsReadOnly(book_item)
    
    // Tenta método ReadOnly
    if (book_item.ReadOnly)
        var result := book_item.ReadOnly(1);
        if (result.errortext)
        else
            return result;
        endif
    endif

    // Tenta definir propriedade readonly
    if (book_item.readonly != error)
        book_item.readonly := 1;
        return 1;
    endif

    // Fallback para SetObjProperty
    SetObjProperty(book_item, "ReadOnly", 1);
    return 1;
endfunction

// Clona o livro lacrado para o container, tornando-o readonly/lacrado
function SealedBooks_CloneToContainer(serial, container)
    
    var df := OpenDataFile(SEALED_BOOKS_DATAFILE);
    if (df.errortext)
        return error{"errortext" := "Datafile error: " + df.errortext};
    endif

    var elem := df.FindElement(serial);
    if (!elem)
        return error{"errortext" := "Serial not found"};
    endif

    var prop_names := elem.PropNames();
    for idx := 1 to prop_names.size()
        var prop_value := elem.GetProp(prop_names[idx]);
    endfor

    var author := elem.GetProp("author");
    var title := elem.GetProp("title");
    var contents := elem.GetProp("contents");


    var book := CreateBookInContainer(container);
    if (!book || book.errortext)
        return error{"errortext" := "CreateItem error: " + (book ? book.errortext : "Failed to create book object")};
    endif

    // SOLUÇÃO: Sempre usar SetObjProperty para garantir compatibilidade
    SetObjProperty(book, "Author", author);
    
    SetObjProperty(book, "Title", title);
    SetName(book, title); // Define o nome visível do item
    
    // Definir Conteúdo via SetObjProperty
    if (TypeOf(contents) == "Array")
        SetObjProperty(book, "Contents", contents);
    elseif (contents)
         SetObjProperty(book, "Contents", array{CStr(contents)});
    else
        SetObjProperty(book, "Contents", array{});
    endif

    // Define como somente leitura
    SetObjProperty(book, "ReadOnly", 1);

    // Marca como lacrado
    SetObjProperty(book, "sealed_serial", serial);
    SetObjProperty(book, "Selado", "sim");

     return book;
endfunction

// Função adicional para debug de criação de livros
function Debug_CreateSealedBook(author, title, contents_param)
    
    var df := OpenDataFile(SEALED_BOOKS_DATAFILE);
    if (df.errortext)
        df := CreateDataFile(SEALED_BOOKS_DATAFILE, DF_KEYTYPE_STRING);
        if (df.errortext)
            return 0;
        endif
    endif

    var serial := CStr(ReadGameClock()) + "_" + CStr(RandomInt(100000));

    var elem := df.CreateElement(serial);

    // Corrected check for element creation
    if (elem.errortext)
        return 0;
    elseif (!elem)
        return 0;
    endif

    elem.SetProp("author", CStr(author));
    elem.SetProp("title", CStr(title));

    var final_contents;
    if (TypeOf(contents_param) == "Array")
        final_contents := contents_param;
    elseif (contents_param)
        final_contents := array{ CStr(contents_param) };
    else
        final_contents := array{};
    endif
    elem.SetProp("contents", final_contents);

    return serial;
endfunction

function SealBookToDatafile(book_object)
    var author, title, contents_array;

    author := GetObjProperty(book_object, "Author");
    if (!author && TypeOf(book_object.GetAuthor) == "ScriptMethod")
        author := book_object.GetAuthor();
    endif
    
    title := GetObjProperty(book_object, "Title");
    if (!title && TypeOf(book_object.GetTitle) == "ScriptMethod")
        title := book_object.GetTitle();
    endif
    
    contents_array := GetObjProperty(book_object, "Contents");
    if (!contents_array && TypeOf(book_object.GetContents) == "ScriptMethod")
        contents_array := book_object.GetContents();
    endif
    
    if (!author || !title)
        var book_df := DFOpenDataFile(":sysbook:books", DF_NO_CREATE);
        if (book_df && !book_df.errortext)
            var book_elem := DFFindElement(book_df, Hex(book_object.serial), DF_NO_CREATE);
            if (book_elem && !book_elem.errortext)
                if (!author) 
                    author := book_elem.GetProp("Author");
                endif
                if (!title) 
                    title := book_elem.GetProp("Title");
                endif
                if (!contents_array || contents_array.size() == 0)
                    contents_array := book_elem.GetProp("Contents");
                endif
            endif
        endif
    endif
    
    // Só usa defaults se realmente não encontrou nada
    if (!title || title == "0") title := "Livro Sem Título"; endif
    if (!author || author == "0") author := "Autor Desconhecido"; endif
    if (TypeOf(contents_array) != "Array") contents_array := array{"[Conteúdo não encontrado]"}; endif
    
   
    // Salva no datafile de livros lacrados
    return Debug_CreateSealedBook(author, title, contents_array);
endfunction