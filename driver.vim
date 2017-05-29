
fu! CreateGame()
        call system("./driver init")
        :e scrabble.scratch
endfu

fu! Update()
        call system("./driver move")
endfu

autocmd BufWritePost scrabble.scratch call Update()
autocmd! VimEnter * call CreateGame()
