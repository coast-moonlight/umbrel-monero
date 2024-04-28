\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{dexercise}

% Counters
\newcounter{task}
\setcounter{task}{1}
\newcounter{sheet}
\setcounter{sheet}{1}
\newcommand\fontparam{10pt}

\providecommand{\taskname}{Aufgabe}
\providecommand{\sheetname}{Übungszettel}
\providecommand{\proofname}{Beweis}
\providecommand{\claimname}{Behauptung}

% Options
\RequirePackage{etoolbox}
\RequirePackage{pgfkeys}
\RequirePackage{pgfopts}
\pgfkeys{
  /dexercise/.cd,
  prefix/.code={\renewcommand{\thetask}{\thesheet.\arabic{task}}},
  sheet/.code={\setcounter{sheet}{#1}},
  task/.code={\setcounter{task}{#1}},
  english/.code={
    \renewcommand{\taskname}{Task}
    \renewcommand{\sheetname}{Exercise Sheet}
    \renewcommand{\proofname}{Proof}
    \renewcommand{\claimname}{Claim}
  },
  12pt/.code={\renewcommand{\fontparam}{12pt}},
  11pt/.code={\renewcommand{\fontparam}{11pt}},
  10pt/.code={\renewcommand{\fontparam}{10pt}},
}
\ProcessPgfOptions{/dexercise}
\LoadClass[\fontparam]{article}

\newcounter{chapter}
\setcounter{chapter}{0}
\setcounter{secnumdepth}{0}

% Math
\RequirePackage[fleqn]{amsmath}
\RequirePackage{amssymb}
\RequirePackage{amsthm}

% Fonts
\RequirePackage{fontspec}
\RequirePackage[bold-style=ISO]{unicode-math}
\setmainfont{TeX Gyre Termes}
\setmathfont{TeX Gyre Termes Math}
\setsansfont{TeX Gyre Heros}
\DeclareMathAlphabet{\mathcal}{OMS}{cmsy}{m}{n}
\RequirePackage[sf]{titlesec}

% Margins
\RequirePackage[
  paper=a4paper,
  left=2.5cm,
  right=4cm,
  top=4cm,
  marginparsep=0.5cm,
  marginparwidth=1.5cm,
  bottom=2cm,
  headsep=1cm,
  head=1.1cm,
  footskip=1cm,
]{geometry}

% Header and footer
\RequirePackage{fancyhdr}
\pagestyle{fancy}
\addtolength{\headheight}{\baselineskip}
\chead{\sffamily\large~\\\vskip-0.9em\textbf{\@title}\\\vskip0.1em \textcolor{maincolor}{\sheetname{} \thesheet}}
\rhead{\vspace*{\fill}\sffamily\small\itshape\@author\vspace*{\fill}}
\lhead{}

% Theorems
\RequirePackage{amsthm}
\newtheoremstyle{def}{\topsep}{\topsep}{}{}{\sffamily\bfseries}{}{5pt plus 1pt minus 1pt}%
{\thmname{#1}\thmnumber{ #2}\thmnote{{\normalfont\ (#3) \\}}}
\newtheoremstyle{thm}{\topsep}{\topsep}{\itshape}{}{\sffamily\bfseries}{}{5pt plus 1pt minus 1pt}%
{\thmname{#1}\thmnumber{ #2}\thmnote{{\normalfont\ (#3) \\}}}

\theoremstyle{thm}
\newtheorem{theorem}{Theorem}[section]
\newtheorem{lemma}[theorem]{Lemma}
\newtheorem{claim}[theorem]{\claimname}
\newtheorem*{claim*}{\claimname}

\theoremstyle{def}
\newtheorem{definition}[theorem]{Definition}
\newtheorem{example}[theorem]{Example}

\RequirePackage[shortlabels]{enumitem}
\setlist{itemsep=0pt, topsep=0pt}
\setlist[enumerate, 1]{label={\color{maincolor}\sffamily\small\arabic*.}}
\setlist[enumerate, 2]{label={\sffamily\small\arabic{enumi}.\arabic*}}

% PDF index
\RequirePackage[breaklinks,hidelinks,bookmarksnumbered]{hyperref}

% Color
\RequirePackage[table]{xcolor}
\colorlet{maincolor}{blue!70!white!80!black}

% Code listings
\RequirePackage{listings}

\lstdefinelanguage{Algebraic}{
  sensitive=true,
  morekeywords={spec,basedon,sorts,consts,ops,vars,axioms,end,if,else,then,fi,def},
  moredelim=[is][\bfseries]{|}{|},
  literate={->}{{$\to$\ }}2 {*}{{$\times$\ }}1 {!}{{$\neg$}}1 {!=}{{$\ne$\ }}2 {=>}{{$\Rightarrow$\ }}2 {||}{{$\lor$\ }}2 {&&}{{$\land$\ }}2 {<=>}{{$\iff$\ }}3,
}

\lstdefinelanguage{pseudocode}{
  morekeywords={
    algorithm,
    if,then,else,while,do,repeat,until,seq,
    seqdo,return,call,
    for,pardo,in,foreach,print,output,input,exit,
    break,loop,end,begin,goto,par,global,local,
    read,write,to,stop,idle,procedure,function,
    throw,catch,null
  },
  sensitive=true,
  morecomment=[l]{//},
  morestring=[b]",
  literate={<-}{{$\gets$\ }}2,
}

\lstdefinelanguage{hlpsl}{
  morekeywords={
    syn,keyword,hlpslKeyword,accept,authentication_on,composition,const,def,
    delete,end,goal,iknows,in,init,local,new,not,played_by,role,
    secrecy_of,weak_authentication_on,transition
  },
  sensitive=true,
  morecomment=[l]{//},
  morestring=[b]",
  literate={<-}{{$\gets$\ }}2,
}

\lstdefinestyle{pseudocode}{
  language=algebraic,
  basicstyle=\rmfamily,
  identifierstyle=\itshape,
  keywordstyle=\bfseries,
  columns=fullflexible,
  numbers=none,
  escapechar=$,
  escapebegin=$,
  escapeend=$,
}

\lstdefinestyle{code}{
  basicstyle=\ttfamily\small,
  keywordstyle=\bfseries\color{maincolor},
  numbers=none,
}

\lstnewenvironment{Algebraic}{\lstset{style=pseudocode,language=Algebraic}}{}
\lstnewenvironment{Pseudocode}{\lstset{style=pseudocode,language=pseudocode}}{}
\lstnewenvironment{Python}{\lstset{style=code,language=python}}{}
\lstnewenvironment{HLPSL}{\lstset{style=code,language=hlpsl}}{}

% Commands for this class
\newcommand{\settitle}[1]{\title{#1}}
\newcommand{\addstudent}[2][]{\ifstrempty{\@author}{\author{#2{} \ifstrempty{#1}{}{(#1)}}}{\g@addto@macro\@author{\\ #2{} \ifstrempty{#1}{}{(#1)}}}}
\newcommand{\task}[1][]{\subsubsection[\taskname{} \thetask\ifstrempty{#1}{}{: #1}]{%
  \sffamily\textbf{\color{maincolor}\taskname{} \thetask}%
  \ifstrempty{#1}{}{\quad\mdseries\color{black}#1}}\stepcounter{task}}

% Easy notation for common sets
\newcommand{\R}{\mathbb{R}}
\newcommand{\N}{\mathbb{N}}
\newcommand{\Z}{\mathbb{Z}}
\newcommand{\Q}{\mathbb{Q}}

% Swap varphi with phi and varepsilon with epsilon
\AfterEndPreamble{%
  \let\oldphi\phi%
  \let\phi\varphi%
  \let\varphi\oldphi%
  \let\oldepsilon\epsilon%
  \let\epsilon\varepsilon%
  \let\varepsilon\oldepsilon%
}

\newcommand{\range}[2][0]{#1\,..{<}\,#2}
