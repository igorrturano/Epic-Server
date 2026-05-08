use uo;
use os;
use util; // Adiciona RandomInt
use polsys;
use datafile;

include ":datafile:datafile";

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

// Retorna um serial aleatório de livro lacrado disponível (apenas publicados)
function SealedBooks_GetRandomSerial()
    var df := OpenDataFile(SEALED_BOOKS_DATAFILE);
    if (df.errortext) // Corrected check
        return 0;
    endif

    var keys := df.Keys();
    var published_serials := array{};
    
    // Filtra apenas livros com status "publicado"
    foreach serial in keys
        var elem := df.FindElement(serial);
        if (elem && elem.GetProp("status") == "publicado")
            published_serials.append(serial);
        endif
        sleepms(1);
    endforeach
    
    if (!published_serials.size())
        return 0;
    endif

    var idx := RandomInt(published_serials.size()) + 1;
    var serial := published_serials[idx];
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
function Debug_CreateSealedBook(author, title, contents_param, original_player_name := "Desconhecido")
    var df := OpenDataFile(SEALED_BOOKS_DATAFILE);
    if (df.errortext)
        df := CreateDataFile(SEALED_BOOKS_DATAFILE, DF_KEYTYPE_STRING);
        if (df.errortext)
            // Print("[sealedBooks Debug_CreateSealedBook] ERRO CRITICO ao criar datafile '" + SEALED_BOOKS_DATAFILE + "': " + df.errortext);
            return 0; 
        endif
    endif

    var new_datafile_serial := CStr(ReadGameClock()) + "_" + CStr(RandomInt(100000));
    // Define 'elem' (ou datafile_element) AQUI, ANTES de usá-lo
    var elem := df.CreateElement(new_datafile_serial); 

    // Verifica se a criação do elemento falhou
    if (!elem || elem.errortext) // Agora 'elem' está definido para esta verificação
        // Print("[sealedBooks Debug_CreateSealedBook] ERRO ao criar elemento no datafile. Chave: " + new_datafile_serial + (elem ? " Erro: " + elem.errortext : " CreateElement retornou nulo/falso"));
        return 0; 
    endif

    // Se chegou aqui, 'elem' é um DataFileElemRef válido.
    var success := 1; 

    if (success)
        success := elem.SetProp("author", CStr(author));
    endif
    if (success && !success.errortext) 
        success := elem.SetProp("title", CStr(title));
    endif
    if (success && !success.errortext)
        success := elem.SetProp("original_player_name", CStr(original_player_name));
    endif
    
    var final_contents;
    if (TypeOf(contents_param) == "Array")
        final_contents := contents_param;
    elseif (contents_param) 
        final_contents := array{ CStr(contents_param) };
    else 
        final_contents := array{};
    endif

    if (success && !success.errortext)
        success := elem.SetProp("contents", final_contents);
    endif

    if (!success || success.errortext) 
        // Print("[Debug_CreateSealedBook] Uma ou mais chamadas SetProp falharam. Deletando elemento criado: " + new_datafile_serial);
        df.DeleteElement(new_datafile_serial); 
        return 0; 
    endif
    
    // Print("[sealedBooks Debug_CreateSealedBook] Livro salvo. Serial DF: " + new_datafile_serial);
    return new_datafile_serial; 
endfunction

function SealBookToDatafile(book_object, account_name_string) // Recebe STRING diretamente
    var author, title, contents_array;
    
    // Usar diretamente o parâmetro recebido (já é string)
    var account_name_of_sealer := account_name_string ? CStr(account_name_string) : "Desconhecido";

    // 1. Tenta ObterPropriedades do objeto livro
    author := GetObjProperty(book_object, "Author");
    title := GetObjProperty(book_object, "Title");
    contents_array := GetObjProperty(book_object, "Contents");

    // 2. Tenta Métodos de Script se CProps falharem
    if (!author && TypeOf(book_object.GetAuthor) == "ScriptMethod")
        author := book_object.GetAuthor();
    endif
    if (!title && TypeOf(book_object.GetTitle) == "ScriptMethod")
        title := book_object.GetTitle();
    endif
    var contents_is_valid_array := (TypeOf(contents_array) == "Array" && contents_array.size() > 0);
    if (!contents_is_valid_array && TypeOf(book_object.GetContents) == "ScriptMethod")
        contents_array := book_object.GetContents();
    endif
    
    // 3. Fallback para datafile :sysbook:books
    if (!author || !title || (TypeOf(contents_array) != "Array" || contents_array.size() == 0) )
        var book_datafile_individual := DFOpenDataFile(":sysbook:books", DF_NO_CREATE);
        if (book_datafile_individual && !book_datafile_individual.errortext)
            var book_elem_individual := DFFindElement(book_datafile_individual, Hex(book_object.serial), DF_NO_CREATE);
            if (book_elem_individual && !book_elem_individual.errortext)
                if (!author) 
                    author := book_elem_individual.GetProp("Author");
                endif
                if (!title) 
                    title := book_elem_individual.GetProp("Title");
                endif
                contents_is_valid_array := (TypeOf(contents_array) == "Array" && contents_array.size() > 0);
                if (!contents_is_valid_array)
                    contents_array := book_elem_individual.GetProp("Contents");
                endif
            endif
        endif
    endif
    
    // 4. Aplicar Defaults
    if (!title || title == "0" || title == error) title := "Livro Sem Título"; endif
    if (!author || author == "0" || author == error) author := "Autor Desconhecido"; endif
    if (TypeOf(contents_array) != "Array") 
        contents_array := array{"[Conteúdo não pôde ser recuperado]"};
    endif
    
    // Salva no datafile de livros lacrados (:sysbook:sealedbooks)
    return Debug_CreateSealedBook(author, title, contents_array, account_name_of_sealer);
endfunction