

function! Complete(findstart, base)
        let l:line = getline('.')
        let l:start = col('.')
        let l:l = line('.')
        "while l:start > 1 && l:line[l:start - 2] =~ '\w'
                "let l:start -= 1
        "endwhile

        if a:findstart == 1
                return l:start
        endif
        echom l:start

        echom a:base
   

        let l:x = l:start - g:scrabble_xo
        let l:y = l:l - g:scrabble_yo
   
        let l:the_command = 'complete --x ' . l:x . ' --y ' . l:y 
        echom l:the_command
        let l:raw_input = system('./driver '. l:the_command)
        let l:input_lines = split(l:raw_input, "\n")
   
        for l:input_line in l:input_lines
                echom l:input_line
                let l:item = json_decode(l:input_line)
                if complete_add(l:item) == 0
                   return []
                endif
                if complete_check()
                   return []
                endif
        endfor

endfu

fu! CreateGame()
        "call system("./driver init")
        :e scrabble.scratch

        call Init()
endfu

fu! TileAdd(c,x,y)
        call matchadd(a:c, '\%' . a:x . 'c\%' . a:y . 'l' )
endfu
fu! Init()
        call TileAdd('red', g:scrabble_xo     , g:scrabble_yo)
        call TileAdd('red', g:scrabble_xo + 7 , g:scrabble_yo)
        call TileAdd('red', g:scrabble_xo + 14, g:scrabble_yo)
        call TileAdd('red', g:scrabble_xo     , g:scrabble_yo + 7)
        call TileAdd('red', g:scrabble_xo + 14, g:scrabble_yo + 7)
        call TileAdd('red', g:scrabble_xo     , g:scrabble_yo + 14)
        call TileAdd('red', g:scrabble_xo + 7 , g:scrabble_yo + 14)
        call TileAdd('red', g:scrabble_xo + 14, g:scrabble_yo + 14)
endfu

fu! Update()
        call system("./driver move")
        silent! e!
endfu

autocmd BufWritePost scrabble.scratch call Update()
autocmd! VimEnter * call CreateGame()

set cursorcolumn
set cursorline
set omnifunc=Complete

let g:scrabble_xo = 6
let g:scrabble_yo = 5


hi red        guifg=red        guibg=lightred    gui=bold ctermbg=red



