" Fichier de configuration de Vim
" Ensimag 2009.

set term=xterm-color
" set mouse=a

filetype on       " enable file type detection
syntax on         " syntax highlighting

filetype plugin on
filetype indent on

set smartindent   " smart code indentation
set smarttab      " smart tabs


" Paramètres sur la recherche

set ignorecase
set hlsearch
set incsearch

" Affiche les parenthèses
set showmatch


"set tabstop=4
set shiftwidth=4
set softtabstop=4
set shiftround
set noexpandtab

set laststatus=2

autocmd FileType make setlocal noexpandtab

" Tell vim to remember certain things when we exit
"  '10  :  marks will be remembered for up to 10 previously edited files
"  "100 :  will save up to 100 lines for each register
"  :20  :  up to 20 lines of command-line history will be remembered
"  %    :  saves and restores the buffer list
"  n... :  where to save the viminfo files
set viminfo='10,\"100,:20,%,n~/.viminfo

function! ResCur()
  if line("'\"") <= line("$")
    normal! g`"
    return 1
  endif
endfunction

augroup resCur
  autocmd!
  autocmd BufWinEnter * call ResCur()
augroup END


" Permet de résoudre le prob :
" quand on lance un fichier .tex vide, lance également
" VimLaTeX
let g:tex_flavor='latex'

" Permet de changer la touche qui lance des commandes
" dans VimLaTeX
let mapleader = ","

imap ß <Plug>Tex_MathBF
imap © <Plug>Tex_MathCal
imap ¬ <Plug>Tex_LeftRight
imap œ <Plug>Tex_InsertItemOnThisLine

let &statusline='%<%f%{&mod?"[+]":""}%r%{&fenc !~ "^$\\|utf-8" || &bomb ? "[".&fenc.(&bomb?"-bom":"")."]" : ""}%=%15.(%l,%c%V %P%)'

autocmd BufRead,BufNewFile   *.c setlocal sw=8
autocmd BufRead,BufNewFile   *.h setlocal sw=8
autocmd BufRead,BufNewFile   *.c setlocal tabstop=8
autocmd BufRead,BufNewFile   *.h setlocal tabstop=8

" <Ctrl-l> redraws the screen and removes any search highlighting.
nnoremap <silent> <C-l> :nohl<CR><C-l>

" Find definition of current symbol using Gtags
map <C-?> <esc>:Gtags -r <CR>

" Find references to current symbol using Gtags
map <C-F> <esc>:Gtags<CR><Enter>:cclose<Enter>
" map <C-F> <esc>:Gtags <CR><Enter>

" Go to previous file
map <C-p> <esc>:bp<CR>

map <F12> <esc>:cclose<CR>

" ctrl o, ctr i pour gtags

autocmd BufWinLeave *.* mkview
autocmd BufWinEnter *.* silent loadview 

"map & f{%mZ%jzf'Z

nmap & /}<CR>zf%<ESC>:nohlsearch<CR>

set mouse=a
