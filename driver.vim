






fu! VimToScrabble(x,y)
        return [ a:x - g:scrabble_xo, a:y - g:scrabble_yo ]
endfu
fu! ScrabbleToVim(x,y)
        return [ a:x + g:scrabble_xo, a:y + g:scrabble_yo ]
endfu

fu! MarkStart(x, y)
        if exists("g:start_mark")
                call matchdelete(g:start_mark)
        endif
        let g:start_mark = TileAdd('green', a:x, a:y)
endfu

function! Complete(findstart, base)

        let l:sc = VimToScrabble(col('.'), line('.'))

        if a:findstart == 1
                return g:scrabble_xo - 1
        endif

        let l:line = deepcopy(getline('.'))

        call setline('.', l:line[0:(g:scrabble_xo)])


        let l:the_command = 'complete'
        let l:raw_input = system('./driver '. l:the_command)
        let l:input_lines = split(l:raw_input, "\n")

        let l:ret = []

        for l:input_line in l:input_lines
                echom l:input_line
                let l:item = json_decode(l:input_line)

                if l:item.orientation ==# "horizontal"
                        if l:item.y == l:sc[1]
                                "let l:cand = l:line[(g:scrabble_xo):(l:item.x)]
                                let l:cand = repeat(' ', l:item.x)
                                let l:cand .= l:item.word
                                call add(l:ret, l:cand)
                        endif
                endif
        endfor

        echom len(l:ret)
        return l:ret

endfu

fu! CreateGame()
        "call system("./driver init")
        :e scrabble.scratch

        call Init()
endfu

fu! TileAdd(c,x,y)
        return matchadd(a:c, '\%' . a:x . 'c\%' . a:y . 'l' )
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

        call setpos('.', [0, g:scrabble_yo + 4, g:scrabble_xo + 5, 0])
endfu

fu! Update()
        call system("./driver move")
        silent! e!
endfu

autocmd BufWritePost scrabble.scratch call Update()
autocmd! VimEnter * call CreateGame()

set cursorcolumn
set cursorline
set wildmenu
set backspace=eol,start,indent
set noswapfile
set mouse=a
set cmdheight=1
set laststatus=2
set undofile                " Save undo's after file closes
set undolevels=100000000       " How many undos
set nocompatible              " be iMproved, required

let g:scrabble_xo = 6
let g:scrabble_yo = 5
let g:scrabble_xlen = 15
let g:scrabble_ylen = 15


hi green      guifg=darkgreen  guibg=lightgreen  gui=bold ctermbg=green
hi red        guifg=red        guibg=lightred    gui=bold ctermbg=red
hi blue       guifg=blue       guibg=lightblue   gui=bold ctermbg=blue


nnoremap p :echo VimToScrabble(col('.'), line('.'))
nnoremap <C-n> i<C-x><C-o>
nnoremap q :q!<CR>

set omnifunc=Complete
set autoread
