






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
        let l:line = getline('.')
        let l:start = col('.')
        let l:l = line('.')

        let l:start -= 1

        " first find first non blank
        while l:start > 0 && l:line[l:start] =~ '\s'
                let l:start -= 1
        endwhile

        let l:word_start = l:start 
        while l:word_start > 0 && l:line[l:word_start - 1] =~ '\w'
                let l:word_start -= 1
        endwhile
        
        let l:start += 1

        if a:findstart == 1
                call MarkStart(l:start, line('.'))
                return l:start
        endif
        echom l:start

        echom a:base

        "call setline('.', l:line[0:(l:start)])
   
        let l:scrabble_cords = VimToScrabble(l:start, l:l )

        let l:the_command = 'complete --x ' . l:scrabble_cords[0] . ' --y ' . l:scrabble_cords[1] 
        echom l:the_command
        let l:raw_input = system('./driver '. l:the_command)
        let l:input_lines = split(l:raw_input, "\n")

        let l:offset = l:start - l:word_start
	    
        echom l:word_start
        echom l:start
        echom l:offset
        
        let l:ret = []
   
        for l:input_line in l:input_lines
                echom l:input_line
                let l:item = json_decode(l:input_line)

                if l:item.orientation ==# "horizontal" 
                        let l:suffix = l:item.word[(l:offset):]
                        call add(l:ret, l:suffix)
                endif

                "if l:item.orientation ==# "horizontal" 
                        "if complete_add(l:item) == 0
                        ""if complete_add({"word":(l:item.word)}) == 0
                           "return []
                        "endif
                "endif
                "if complete_check()
                   "return []
                "endif
        endfor

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
