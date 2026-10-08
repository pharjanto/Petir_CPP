unit opening_book;

interface
uses header;
const maxhashbook=262144 div 2;
 minimum_elo=2540;
 thr_min=-25;
 depth_check=5;
type
  tbook=packed record
    key:int64;
    bobot:word;
  end;
  thashbook=packed record
    book:tbook;
    used:boolean;
  end;
  hashbook=array[0..1,0..maxhashbook-1] of thashbook;


var
  book:hashbook;

procedure learn_pgn(a:string);
procedure initbook;
procedure loadbook;
function searchbook(data:tdata;giliran:integer):integer;
function searchhashbook(key:int64;giliran:byte;var weight:integer):boolean;
function searchhashbook2(key:int64;giliran:byte;var weight:integer):boolean;
function edithashbook(key:int64;giliran:byte;newbobot:integer):boolean;
procedure savebook;
implementation
uses udata,hashing_header,dialogs,notation,makemove,sysutils,unit1,movgen,tools,search;
var no:integer=0; a:integer;

procedure learn;
begin
end;

function searchhashbook(key:int64;giliran:byte;var weight:integer):boolean;
var hkey,temp:int64;
p:^thashbook;
blackwin:integer;
begin
  hkey:=key AND (maxhashbook-1);
  temp:=hkey;
  dec(giliran);
  while (book[giliran,hkey].used) and (book[giliran,hkey].book.key<>key) do
  begin
    inc(hkey);
    if hkey>maxhashbook-1 then hkey:=0;
    if hkey=temp then
    begin
      result:=false;
//      showmessage('not found');
      exit;
    end;
  end;
  p:=@book[giliran,hkey];
  if (p^.used=false) or (p^.book.bobot<4) then
  begin
    result:=false;exit;
  end else
  begin
    result:=true;
    inc(giliran);

    if giliran=_SISIPUTIH then
      weight:=(p^.book.bobot)
    else
      weight:=p^.book.bobot;
    if weight<=0 then result:=false;
  end;

end;

function searchhashbook2(key:int64;giliran:byte;var weight:integer):boolean;
var hkey,temp:int64;
p:^thashbook;
blackwin:integer;
begin
  hkey:=key AND (maxhashbook-1);
  temp:=hkey;
  dec(giliran);
  while (book[giliran,hkey].used) and (book[giliran,hkey].book.key<>key) do
  begin
    inc(hkey);
    if hkey>maxhashbook-1 then hkey:=0;
    if hkey=temp then
    begin
      result:=false;
//      showmessage('not found');
      exit;
    end;
  end;
  p:=@book[giliran,hkey];
  if (p^.used=false) or (p^.book.bobot<4) then
  begin
    result:=false;exit;
  end else
  begin
    result:=true;
    inc(giliran);

    if giliran=_SISIPUTIH then
      weight:=(p^.book.bobot)
    else
      weight:=p^.book.bobot;
  end;

end;


function searchbook(data:tdata;giliran:integer):integer;
var temp:tdata;
ml:tmovelist;
jml,n,n2:integer;
pp:boolean;
s:byte;
m:array[1..20] of record
  moves:word;
  weight:integer;
end;
totalw,w,r:integer;
begin
{  if not searchhashbook(data.hashkey,3-giliran,w) and (data.hashkey<>3423812783342919194) then
  begin
    result:=_NO_MOVE;exit;
  end;}
  temp:=data;
  n:=0;totalw:=0;
  result:=_NO_MOVE;
  fillchar(m,sizeof(m),0);
  if giliran=_SISIPUTIH then
  begin
    white_movgen_noncaps(data,ml,jml,0);
    for a:=1 to jml do
    begin
      data:=temp;
      makewhitemove(ml[a].moves,data);
      if white_checked(data) then continue;
      if searchhashbook(data.hashkey,giliran,w) then
      begin
//        cleartable;
          inc(n);m[n].moves:=ml[a].moves;m[n].weight:=w;inc(totalw,w);

      end;
    end;
    data:=temp;
    white_movgen_caps(data,ml,jml);
    for a:=1 to jml do
    begin
      data:=temp;
      makewhitemove(ml[a].moves,data);
      if white_checked(data) then continue;
      if searchhashbook(data.hashkey,giliran,w) then
      begin
//        cleartable;
          inc(n);m[n].moves:=ml[a].moves;m[n].weight:=w;inc(totalw,w);

      end;
    end;

  end else
  begin
    black_movgen_noncaps(data,ml,jml);
    for a:=1 to jml do
    begin
      data:=temp;
      makeblackmove(ml[a].moves,data);
      if black_checked(data) then continue;
      if searchhashbook(data.hashkey,giliran,w) then
      begin
          inc(n);m[n].moves:=ml[a].moves;m[n].weight:=w;inc(totalw,w);

      end;
    end;
    data:=temp;
    black_movgen_caps(data,ml,jml);
    for a:=1 to jml do
    begin
      data:=temp;
      makeblackmove(ml[a].moves,data);
      if black_checked(data) then continue;
      if searchhashbook(data.hashkey,giliran,w) then
      begin
//        cleartable;
          inc(n);m[n].moves:=ml[a].moves;m[n].weight:=w;inc(totalw,w);

      end;
    end;

  end;
  if n=0 then
  begin
    result:=_NO_MOVE;exit;
  end else
  begin
    r:=random(totalw)+1;w:=0;
    for a:=1 to n do
    begin
      inc(w,m[a].weight);
      if w>=r then
      begin
        result:=m[a].moves;exit;
      end;
    end;
  end;

end;

procedure initbook;
var a,b:integer;
begin
  for a:=0 to 1 do
    for b:=0 to maxhashbook-1 do
    begin
      book[a,b].used:=false;
    end;
end;

function addhashbook(key:int64;hasil,giliran:byte;whiteelo,blackelo:integer):boolean;
var hkey,temp:int64;
p:^thashbook;
begin
  result:=true;
  hkey:=key AND (maxhashbook-1);
  temp:=hkey;
  dec(giliran);
  while (book[giliran,hkey].used) and (book[giliran,hkey].book.key<>key) do
  begin
    inc(hkey);
    if hkey>maxhashbook-1 then hkey:=0;
    if hkey=temp then
    begin
      result:=false;
      showmessage('hash penuh');      
      exit;
    end;
  end;
  p:=@book[giliran,hkey];
  inc(giliran);
  if p^.used=false then
  begin
    p^.used:=true;
    p^.book.key:=key;
    p^.book.bobot:=0;
    if hasil=RESULT_DRAW then
      inc(p^.book.bobot) else
    if (giliran=_SISIPUTIH) and (hasil=RESULT_WHITE_WIN) then
      inc(p^.book.bobot,2) else
    if (giliran=_SISIHITAM) and (hasil=RESULT_BLACK_WIN) then
      inc(p^.book.bobot,2);
  end else
  begin
    if hasil=RESULT_DRAW then
      inc(p^.book.bobot) else
    if (giliran=_SISIPUTIH) then
    begin
      if(hasil=RESULT_WHITE_WIN) then
      begin
        inc(p^.book.bobot,2);
        if (blackelo-whiteelo)>=200 then
          inc(p^.book.bobot,3)
        else if (blackelo-whiteelo)>=100 then
          inc(p^.book.bobot,1);
      end else
      begin
        if p^.book.bobot>=2 then
        begin
         dec(p^.book.bobot,2);
         if (whiteelo-blackelo>=200) and (p^.book.bobot>=2) then
           dec(p^.book.bobot,2) else
         if (whiteelo-blackelo>=100) and (p^.book.bobot>=1) then
           dec(p^.book.bobot);
        end;
      end;
    end else
    if (giliran=_SISIHITAM) then
    begin
      if (hasil=RESULT_BLACK_WIN) then
      begin
        inc(p^.book.bobot,2);
        if (whiteelo-blackelo)>=200 then
          inc(p^.book.bobot,4)
        else if (whiteelo-blackelo)>=100 then
          inc(p^.book.bobot,2)
        else if (whiteelo-blackelo)>=50 then
          inc(p^.book.bobot,1);
      end else
      begin
        if p^.book.bobot>=2 then
        begin
         dec(p^.book.bobot,2);
         if (blackelo-whiteelo>=250) and (p^.book.bobot>=2) then
           dec(p^.book.bobot,2) else
         if (blackelo-whiteelo>=150) and (p^.book.bobot>=1) then
           dec(p^.book.bobot);

        end;
      end;
    end;
  end;

end;

function edithashbook(key:int64;giliran:byte;newbobot:integer):boolean;
var hkey,temp:int64;
p:^thashbook;
begin
  result:=true;
  hkey:=key AND (maxhashbook-1);
  temp:=hkey;
  dec(giliran);
  while (book[giliran,hkey].used) and (book[giliran,hkey].book.key<>key) do
  begin
    inc(hkey);
    if hkey>maxhashbook-1 then hkey:=0;
    if hkey=temp then
    begin
      result:=false;
      showmessage('hash penuh');
      exit;
    end;
  end;
  p:=@book[giliran,hkey];
  p^.book.bobot:=newbobot;
  //inc(giliran);

  

end;


function storebook(b:tbook;giliran:integer):boolean;
var hkey,temp:int64;
p:^thashbook;
begin
  result:=true;
  hkey:=b.key AND (maxhashbook-1);
  temp:=hkey;
  dec(giliran);
  while (book[giliran,hkey].used) and (book[giliran,hkey].book.key<>b.key) do
  begin
    inc(hkey);
    if hkey>maxhashbook-1 then hkey:=0;
    if hkey=temp then
    begin
      result:=false;
      showmessage('hash penuh');
      exit;
    end;
  end;
  p:=@book[giliran,hkey];
  p^.book:=b;
  p^.used:=true;

end;


procedure make(notasi:string;var data:tdata;giliran:byte);
var move:integer;pp:boolean;
begin
  if notasi[1]<>'O' then
    notasi:=shortnotationtolong(notasi,data,giliran);
  move:=notationtomove(notasi,data,giliran);
  if giliran=_SISIPUTIH then
    makewhitemove(move,data)
  else
    makeblackmove(move,data)
end;

procedure loadbook;
var f:file;
buf:array[1..10000] of tbook;
a,numread:integer;
begin
  if fileexists(form1.defaultdir+'\white.book') then
  begin
  assignfile(f,form1.defaultdir+'\white.book');

  reset(f,1);
  if ioresult<>0 then exit;
  repeat
    blockread(f,buf,sizeof(buf),numread);
    numread:=numread div sizeof(tbook);
    for a:=1 to numread do
    begin
      no:=numread;
      storebook(buf[a],_SISIPUTIH);
    end;
  until numread=0;
  closefile(f);
  end;
  if fileexists(form1.defaultdir+'\black.book') then
  begin

  assignfile(f,form1.defaultdir+'\black.book');

  reset(f,1);
  if ioresult<>0 then exit;
  repeat
    blockread(f,buf,sizeof(buf),numread);
    numread:=numread div sizeof(tbook);
    for a:=1 to numread do
    begin
      storebook(buf[a],_SISIHITAM);
    end;
  until numread=0;
  closefile(f);
  end;
end;

procedure savebook;
var a,b:integer;
f:file of tbook;
begin
  assignfile(f,form1.defaultdir+'\white.book');
  rewrite(f);
  for b:=0 to maxhashbook-1 do
  begin
    if book[_SISIPUTIH-1,b].used then
    begin
      write(f,book[_SISIPUTIH-1,b].book);
    end;
  end;
  closefile(f);
  assignfile(f,form1.defaultdir+'\black.book');
  rewrite(f);
  for b:=0 to maxhashbook-1 do
  begin
    if book[_SISIHITAM-1,b].used then
    begin
      write(f,book[_SISIHITAM-1,b].book);
    end;
  end;
  closefile(f);

end;

procedure learn_pgn(a:string);
var f:text;
s,notasi,s2:string;
ma,mt,p,jml,spasi,lastspasi:byte;
data:tdata;
err,stop:boolean;
oldnotasi:string;
k:int64;
giliran,whiteelo,blackelo,code:integer;
hasil:byte;
begin
  assignfile(f,a);
  reset(f);
  giliran:=_sisiputih;
  jml:=0;
  stop:=false;
//  hashbook[1268].used:=true;

  while not eof(f) do
  begin
    readln(f,s);
    if s<>'' then s:=s+' ';
    lastspasi:=1;
    if uppercase(copy(s,1,7))=uppercase('[Result') then
    begin
      whiteelo:=0;blackelo:=0;
      if s[11]='/' then hasil:=RESULT_DRAW
      else if s[10]='1' then hasil:=RESULT_WHITE_WIN
      else if s[10]='0' then hasil:=RESULT_BLACK_WIN
      else if s[10]='*' then hasil:=RESULT_DRAW
      else
      begin
        showmessage('result error');
        exit;
      end;
    end else
    if uppercase(copy(s,1,9))='[WHITEELO' THEN
    begin
      s2:=copy(s,12,4);
      val(s2,whiteelo,code);
    end else
    if uppercase(copy(s,1,9))='[BLACKELO' THEN
    begin
      s2:=copy(s,12,4);
      val(s2,blackelo,code);
    end;

    if (length(s)>0) and (s[1]<>'[') then
    begin

      if copy(s,1,2)='1.' then
      begin
          initstate(data);
          giliran:=_sisiputih;
          jml:=0;
          stop:=false;
          inc(no);
      end;

      while (pos(' ',s)<>0) and not stop do
      begin
        spasi:=pos(' ',s);
        notasi:=copy(s,lastspasi,spasi-lastspasi);
        delete(s,lastspasi,(spasi-lastspasi)+1);
        if not (notasi[1] in ['1'..'9']) then
        begin
          k:=hashvalue(data,giliran);
          oldnotasi:=notasi;

          if notasi[length(notasi)]='+' then
            notasi:=copy(notasi,1,length(notasi)-1);
          try
            make(notasi,data,giliran);
            if (whiteelo>=MINIMUM_ELO) and (BLACKELO>=MINIMUM_ELO) and (jml>=20) then
            begin
              if not addhashbook(data.hashkey,hasil,giliran,whiteelo,blackelo) then
                exit;
            end;

            inc(jml);
            if jml>=34 then stop:=true;
          except
            stop:=true;
          end;

          giliran:=3-giliran;
        end;
      end;
    end;
  end;

  closefile(f);
  showmessage('finish');
  savebook;
end;


end.
