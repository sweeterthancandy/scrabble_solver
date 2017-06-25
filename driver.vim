
fu! Log(msg)
        call system("echo ". a:msg . " >> driver.log")
endfu
fu! LoggedSystem(cmd)
        call Log(a:cmd)
        return system(a:cmd)
endfu



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

fu! AddHeat(x,y,percent)
        let l:hi = 'pct_' . str2nr(a:percent . '' )
        return ScrabbleTileAdd(l:hi, a:x, a:y)
endfu
fu! HeatMap()
        if exists('g:heat_map')
                for c in g:heat_map
                        call matchdelete(c)
                endfor
        endif
        let l:raw_input = LoggedSystem('./driver heat-map --orientation horizontal')
        let l:input_lines = split(l:raw_input, "\n")
                
        let g:heat_map = []
        for l:input_line in l:input_lines
                let l:item = json_decode(l:input_line)
                let c = AddHeat(l:item.x, l:item.y, l:item.metric  * 100 / l:item.max )
                let g:heat_map += [c]
        endfor
endfu

function! Complete(findstart, base)

        let l:sc = VimToScrabble(col('.'), line('.'))
                
        let l:the_command = 'complete --y ' . l:sc[1] . ' --orientation horizontal'

        if a:findstart == 1
                let l:raw_input = LoggedSystem('./driver '. l:the_command)
                let l:input_lines = split(l:raw_input, "\n")

                echom l:raw_input
                echom len(l:raw_input)
                if len(l:raw_input) == 0 
                        echom "empty"
                        return col('.')
                endif


                let g:current_line = getline('.')
                call setline('.', getline('.')[0:(g:scrabble_xo)])
                return g:scrabble_xo - 1
        endif


        let l:raw_input = LoggedSystem('./driver '. l:the_command)
        let l:input_lines = split(l:raw_input, "\n")
                
        if len(l:raw_input) == 0 
                return []
        endif

        let l:ret = []

        for l:input_line in l:input_lines
                let l:item = json_decode(l:input_line)

                call Log(string(l:item))

                if l:item.orientation ==# "horizontal"
                        let l:cand = ''
                        echom g:current_line
                        call Log(string(l:item))
                        for l:i in range(g:scrabble_xlen)
                                if l:item.x <= l:i && l:i < l:item.x + len(l:item.word)
                                        let l:cand .= l:item.word[ l:i - l:item.x]
                                else
                                        let l:cand .= g:current_line[g:scrabble_xo-1 + l:i]
                                endif
                        endfor
                        call Log(string(l:item))
                        let l:cand .= '|'
                        call add(l:ret, l:cand)
                        call Log(string(l:item))
                endif
        endfor

        if len(l:ret) == 0
                call setline('.', g:current_line)
        endif

        call Log(string(l:ret))

        return l:ret

endfu

fu! CreateGame()
        "call LoggedSystem("./driver init")
        :e scrabble.scratch

        call Init()
endfu

fu! TileAdd(c,x,y)
        return matchadd(a:c, '\%' . a:x . 'c\%' . a:y . 'l' )
endfu
fu! ScrabbleTileAdd(c,x,y)
        return TileAdd(a:c, a:x + g:scrabble_xo, a:y + g:scrabble_yo)
endfu
fu! Init()

        let l:world = []
        let l:world += ['black', 'darkBlue', 'darkGreen', 'darkCyan' ]
        let l:world += ['darkRed', 'darkMagenta', 'brown', 'lightGray']
        let l:world += ['darkGray', 'blue', 'green', 'cyan', 'red']
        let l:world += ['magenta', 'yellow', 'white']
        let index = 0
        for c in l:world
                call TileAdd(c, index, 1)
                let index += 1
        endfor

        let tw = 'red'
        let dw = 'brown'
        let tl = 'blue'
        let dl = 'darkBlue'
        let __ = 'green'
        let st = 'brown'

        let decl = []
        let decl += [[tw,__,__,dl,__,__,__,tw,__,__,__,dl,__,__,tw]]
        let decl += [[__,dw,__,__,__,tl,__,__,__,tl,__,__,__,dw,__]]
        let decl += [[__,__,dw,__,__,__,dl,__,dl,__,__,__,dw,__,__]]
        let decl += [[dl,__,__,dw,__,__,__,dl,__,__,__,dw,__,__,dl]]
        let decl += [[__,__,__,__,dw,__,__,__,__,__,dw,__,__,__,__]]
        let decl += [[__,tl,__,__,__,tl,__,__,__,tl,__,__,__,tl,__]]
        let decl += [[__,__,dl,__,__,__,dl,__,dl,__,__,__,dl,__,__]]
        let decl += [[tw,__,__,dl,__,__,__,st,__,__,__,dl,__,__,tw]]
        let decl += [[__,__,dl,__,__,__,dl,__,dl,__,__,__,dl,__,__]]
        let decl += [[__,tl,__,__,__,tl,__,__,__,tl,__,__,__,tl,__]]
        let decl += [[__,__,__,__,dw,__,__,__,__,__,dw,__,__,__,__]]
        let decl += [[dl,__,__,dw,__,__,__,dl,__,__,__,dw,__,__,dl]]
        let decl += [[__,__,dw,__,__,__,dl,__,dl,__,__,__,dw,__,__]]
        let decl += [[__,dw,__,__,__,tl,__,__,__,tl,__,__,__,dw,__]]
        let decl += [[tw,__,__,dl,__,__,__,tw,__,__,__,dl,__,__,tw]]

        "let l = g:scrabble_yo
        "for line in decl
                "let c = g:scrabble_xo
                "for item in line
                        "call TileAdd(item, c, l)
                        "let c += 1
                "endfor
                "let l += 1
        "endfor

        "call TileAdd('hi_tl', g:scrabble_xo     , g:scrabble_yo)
        "call TileAdd('hi_tl', g:scrabble_xo + 7 , g:scrabble_yo)
        "call TileAdd('hi_tl', g:scrabble_xo + 14, g:scrabble_yo)
        "call TileAdd('hi_tl', g:scrabble_xo     , g:scrabble_yo + 7)
        "call TileAdd('hi_tl', g:scrabble_xo + 14, g:scrabble_yo + 7)
        "call TileAdd('hi_tl', g:scrabble_xo     , g:scrabble_yo + 14)
        "call TileAdd('hi_tl', g:scrabble_xo + 7 , g:scrabble_yo + 14)
        "call TileAdd('hi_tl', g:scrabble_xo + 14, g:scrabble_yo + 14)

        call setpos('.', [0, g:scrabble_yo + 4, g:scrabble_xo + 5, 0])

        for i in range(0,100)
                let l:gg = printf('%02x',i * 255 / 100)
                let l:rr = printf('%02x', 255 - i* 255 / 100)
                let l:bb = printf('%02x',i * 255 / 100)
                let l:rrggbb = l:rr . l:gg . l:bb
                let l:hi = 'pct_' . l:i
                let l:cmd = 'hi ' . l:hi .' guibg=#' . l:rrggbb  . ' guifg=#' . l:rrggbb
                echom l:cmd
                execute l:cmd
                call TileAdd( l:hi, i+1, 1)
        endfor

endfu

fu! Update()
        echom 'Update'
        "silent! w!
        call LoggedSystem("./driver move")
        silent! e!
        redraw
endfu
fu! Rotate()
        call LoggedSystem("./driver rotate")
        silent! e!
        redraw
endfu

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
set autoread

let g:scrabble_xo = 6
let g:scrabble_yo = 5
let g:scrabble_xlen = 15
let g:scrabble_ylen = 15


hi green  ctermbg=green

hi black       ctermbg=Black
hi darkBlue    ctermbg=DarkBlue
hi DarkGreen   ctermbg=DarkGreen
hi DarkCyan    ctermbg=DarkCyan
hi DarkRed     ctermbg=DarkRed
hi DarkMagenta ctermbg=DarkMagenta
hi Brown       ctermbg=Brown
hi LightGray   ctermbg=LightGray
hi DarkGray    ctermbg=DarkGray
hi Blue        ctermbg=Blue
hi Green       ctermbg=Green
hi Cyan        ctermbg=Cyan
hi Red         ctermbg=Red
hi Magenta     ctermbg=Magenta
hi Yellow      ctermbg=Yellow
hi white       ctermbg=White

hi link hi_tl red
hi link hi___ green


nnoremap p :echo VimToScrabble(col('.'), line('.'))
nnoremap <C-n> i<C-x><C-o>
nnoremap q :q!<CR>
"nnoremap <Space> :call Update()<CR>
nnoremap G :call Rotate()<CR>
nnoremap H :call HeatMap()<CR>

set omnifunc=Complete

augroup Reload 
  au! 
  au BufWritePost scrabble.scratch call Update()
augroup END 


let &runtimepath.=',/home/dest/.vim/bundle/vim-colors-solarized/'
set background=light
colorscheme solarized

set guioptions=

set lines=100 columns=100
