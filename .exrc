if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
inoremap <S-Tab> 
inoremap <silent> <SNR>23_AutoPairsReturn =AutoPairsReturn()
inoremap <silent> <Plug>NERDCommenterInInsert  <BS>:call NERDComment(0, "insert")
map ,mq <Plug>MBEMarkCurrent
map ,mbt <Plug>TMiniBufExplorer
map ,mbu <Plug>UMiniBufExplorer
map ,mbc <Plug>CMiniBufExplorer
map ,mbe <Plug>MiniBufExplorer
nmap ,ihn :IHN
nmap ,is :IHS:A
nmap ,ih :IHS
nmap ,ca <Plug>NERDCommenterAltDelims
vmap ,cA <Plug>NERDCommenterAppend
nmap ,cA <Plug>NERDCommenterAppend
vmap ,c$ <Plug>NERDCommenterToEOL
nmap ,c$ <Plug>NERDCommenterToEOL
vmap ,cu <Plug>NERDCommenterUncomment
nmap ,cu <Plug>NERDCommenterUncomment
vmap ,cn <Plug>NERDCommenterNest
nmap ,cn <Plug>NERDCommenterNest
vmap ,cb <Plug>NERDCommenterAlignBoth
nmap ,cb <Plug>NERDCommenterAlignBoth
vmap ,cl <Plug>NERDCommenterAlignLeft
nmap ,cl <Plug>NERDCommenterAlignLeft
vmap ,cy <Plug>NERDCommenterYank
nmap ,cy <Plug>NERDCommenterYank
vmap ,ci <Plug>NERDCommenterInvert
nmap ,ci <Plug>NERDCommenterInvert
vmap ,cs <Plug>NERDCommenterSexy
nmap ,cs <Plug>NERDCommenterSexy
vmap ,cm <Plug>NERDCommenterMinimal
nmap ,cm <Plug>NERDCommenterMinimal
vmap ,c  <Plug>NERDCommenterToggle
nmap ,c  <Plug>NERDCommenterToggle
vmap ,cc <Plug>NERDCommenterComment
nmap ,cc <Plug>NERDCommenterComment
nmap gx <Plug>NetrwBrowseX
map <silent> mm <Plug>Vm_toggle_sign 
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
map <S-F2> <Plug>Vm_goto_prev_sign
map <F2> <Plug>Vm_goto_next_sign
map <C-F2> <Plug>Vm_toggle_sign
nmap <F7> :call ToggleGDB()
nmap <silent> <Plug>NERDCommenterAppend :call NERDComment(0, "append")
nnoremap <silent> <Plug>NERDCommenterToEOL :call NERDComment(0, "toEOL")
vnoremap <silent> <Plug>NERDCommenterUncomment :call NERDComment(1, "uncomment")
nnoremap <silent> <Plug>NERDCommenterUncomment :call NERDComment(0, "uncomment")
vnoremap <silent> <Plug>NERDCommenterNest :call NERDComment(1, "nested")
nnoremap <silent> <Plug>NERDCommenterNest :call NERDComment(0, "nested")
vnoremap <silent> <Plug>NERDCommenterAlignBoth :call NERDComment(1, "alignBoth")
nnoremap <silent> <Plug>NERDCommenterAlignBoth :call NERDComment(0, "alignBoth")
vnoremap <silent> <Plug>NERDCommenterAlignLeft :call NERDComment(1, "alignLeft")
nnoremap <silent> <Plug>NERDCommenterAlignLeft :call NERDComment(0, "alignLeft")
vmap <silent> <Plug>NERDCommenterYank :call NERDComment(1, "yank")
nmap <silent> <Plug>NERDCommenterYank :call NERDComment(0, "yank")
vnoremap <silent> <Plug>NERDCommenterInvert :call NERDComment(1, "invert")
nnoremap <silent> <Plug>NERDCommenterInvert :call NERDComment(0, "invert")
vnoremap <silent> <Plug>NERDCommenterSexy :call NERDComment(1, "sexy")
nnoremap <silent> <Plug>NERDCommenterSexy :call NERDComment(0, "sexy")
vnoremap <silent> <Plug>NERDCommenterMinimal :call NERDComment(1, "minimal")
nnoremap <silent> <Plug>NERDCommenterMinimal :call NERDComment(0, "minimal")
vnoremap <silent> <Plug>NERDCommenterToggle :call NERDComment(1, "toggle")
nnoremap <silent> <Plug>NERDCommenterToggle :call NERDComment(0, "toggle")
vnoremap <silent> <Plug>NERDCommenterComment :call NERDComment(1, "norm")
nnoremap <silent> <Plug>NERDCommenterComment :call NERDComment(0, "norm")
nnoremap <silent> <F6> :Tlist
nnoremap <silent> <F3> :Grep
nnoremap <silent> <F12> :A
nnoremap <silent> <F5> :NERDTreeToggle
map <C-F12> :!ctags -R --c++-kinds=+p --fields=+iaS --extra=+q . 
imap 	 
inoremap <expr>  omni#cpp#maycomplete#Complete()
imap  =CtrlXPP()
imap ,ihn :IHN
imap ,is :IHS:A
imap ,ih :IHS
inoremap <expr> . omni#cpp#maycomplete#Dot()
inoremap <expr> : omni#cpp#maycomplete#Scope()
inoremap <expr> > omni#cpp#maycomplete#Arrow()
let &cpo=s:cpo_save
unlet s:cpo_save
set background=dark
set backspace=indent,eol,start
set completeopt=longest,menu
set noequalalways
set fileencodings=utf-8,gbk
set helplang=cn
set hlsearch
set nomodeline
set omnifunc=omni#cpp#complete#Main
set runtimepath=~/.vim,~/.vim/bundle/vundle,~/.vim/bundle/auto-pairs,~/.vim/bundle/SuperTab,/usr/local/share/vim/vimfiles,/usr/local/share/vim/vim73,/usr/local/share/vim/vimfiles/after,~/.vim/after,~/.vim/bundle/vundle/,~/.vim/bundle/vundle/after,~/.vim/bundle/auto-pairs/after,~/.vim/bundle/SuperTab/after
set shiftwidth=4
set tabstop=4
set tags=./tags,./TAGS,tags,TAGS,/usr/include/syscall.tags,/data/limeng/base/vdn/adapter/tags,/data/limeng/base/commlib/tags,/data/limeng/base/vdn/rtsp/ffmpeg-2.0.1.new/tags,/data/liuhua/commonentDB_read/tags,/data/liuhua/base/functest/httpthread/tags,/data/liuhua/commlib/tags,/data/guoqq/download/redis-cplusplus-client-fa63f7f275b373830d99ccffce34bf3500b0bd32/tags,/data/xiemin/sourcelib/nginx-live-p2p/tags,/data/xiemin/sourcelib/nginxdelay-1.0.7/tags,/data/xiemin/sourcelib/axel-2.4/tags,/data/xiemin/sourcelib/acl-code/tags,/data/chenxin/nginxdelay-1.0.5/src/sysguard/tags,/data/chenxin/base2/CommentSystem/IDServer/memthread/tags,/data/chenxin/base2/CommentSystem/IdServer/tags,/data/chenxin/base2/CommentSystem/CommentDB_write/tags,/data/zsy/base/functest/zhousy/weather/tags
set termencoding=utf-8
set updatetime=300
" vim: set ft=vim :
