


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

let g:scrabble_xo = 6
let g:scrabble_yo = 5


hi red        guifg=red        guibg=lightred    gui=bold ctermbg=red



