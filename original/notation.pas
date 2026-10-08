unit notation;

interface
uses header;

Function movetonotation(move:integer;ep,giliran:integer):string;
function notationtomove(notasi:string;data:tdata;giliran:byte):integer;
function shortnotationtolong(notasi:string;data:tdata;giliran:byte):string;
function getnotasi(square:integer):string;

implementation
uses sysutils,bitboard_mask;

function squaretobinary(s:string):integer;
var x,y:byte;
begin
    x:=ord(s[1])-97;
    y:=strtoint(s[2])-1;
    result:=y*8+x;
end;

function getnotasi(square:integer):string;
begin
  case square of
    a1 : result :='a1';
    a2 : result :='a2';
    a3 : result :='a3';
    a4 : result :='a4';
    a5 : result :='a5';
    a6 : result :='a6';
    a7 : result :='a7';
    a8 : result :='a8';
    b1 : result :='b1';
    b2 : result :='b2';
    b3 : result :='b3';
    b4 : result :='b4';
    b5 : result :='b5';
    b6 : result :='b6';
    b7 : result :='b7';
    b8 : result :='b8';
    c1 : result :='c1';
    c2 : result :='c2';
    c3 : result :='c3';
    c4 : result :='c4';
    c5 : result :='c5';
    c6 : result :='c6';
    c7 : result :='c7';
    c8 : result :='c8';
    d1 : result :='d1';
    d2 : result :='d2';
    d3 : result :='d3';
    d4 : result :='d4';
    d5 : result :='d5';
    d6 : result :='d6';
    d7 : result :='d7';
    d8 : result :='d8';
    e1 : result :='e1';
    e2 : result :='e2';
    e3 : result :='e3';
    e4 : result :='e4';
    e5 : result :='e5';
    e6 : result :='e6';
    e7 : result :='e7';
    e8 : result :='e8';
    f1 : result :='f1';
    f2 : result :='f2';
    f3 : result :='f3';
    f4 : result :='f4';
    f5 : result :='f5';
    f6 : result :='f6';
    f7 : result :='f7';
    f8 : result :='f8';
    g1 : result :='g1';
    g2 : result :='g2';
    g3 : result :='g3';
    g4 : result :='g4';
    g5 : result :='g5';
    g6 : result :='g6';
    g7 : result :='g7';
    g8 : result :='g8';
    h1 : result :='h1';
    h2 : result :='h2';
    h3 : result :='h3';
    h4 : result :='h4';
    h5 : result :='h5';
    h6 : result :='h6';
    h7 : result :='h7';
    h8 : result :='h8';
  end;
end;


function shortnotationtolong(notasi:string;data:tdata;giliran:byte):string;

function notasi_pion:string;
var sfrom,sto,x:integer;
begin
  if length(notasi)=2 then
  begin
     sto:=squaretobinary(notasi);
     if giliran=_SISIPUTIH then
     begin
       if data.papan[sto-8]=_PIONPUTIH then sfrom:=sto-8
       else sfrom:=sto-16;
     end else
     begin
       if data.papan[sto+8]=_PIONHITAM then sfrom:=sto+8
       else sfrom:=sto+16;
     end;
  end else
  begin
    x:=ord(notasi[1])-97;
    sto:=squaretobinary(copy(notasi,3,2));
    if giliran=_SISIPUTIH then
    begin
      if x<sto mod 8 then sfrom:=sto-9
      else sfrom:=sto-7;
    end else
    begin
      if x<sto mod 8 then sfrom:=sto+7
      else sfrom:=sto+9;
    end;
  end;
  result:=getnotasi(sfrom)+getnotasi(sto);
end;

function notasi_perwira:string;
var sfrom,sto,x,y:integer;
a:int64;
begin
  x:=9;y:=9;
  if length(notasi)=3 then
    sto:=squaretobinary(copy(notasi,2,2))
  else
  begin
    sto:=squaretobinary(copy(notasi,length(notasi)-1,2));
    if notasi[2]<>'x' then
    begin
      if notasi[2] in ['a'..'h'] then x:=ord(notasi[2])-97
      else y:=strtoint(notasi[2])-1;
    end;
  end;

  case notasi[1] of
  'N' :
    begin
      a:=knightmask[sto];
      if giliran=_SISIPUTIH then a:=a and data.knightwhite
      else a:=a and data.knightblack;
    end;
  'B' :
    begin
      a:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
      a:=a or diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
      if giliran=_SISIPUTIH then a:=a and data.bishopwhite
      else a:=a and data.bishopblack;
    end;
  'R' :
    begin
      a:=horzmask2[sto,(data.allpieces shr (sto and 56+1)) and 63];
      a:=a or vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
      if giliran=_SISIPUTIH then a:=a and data.rookwhite
      else a:=a and data.rookblack;
    end;
  'Q' :
    begin
      a:=horzmask2[sto,(data.allpieces shr (sto and 56+1)) and 63];
      a:=a or vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
      a:=a or diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
      a:=a or diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];

      if giliran=_SISIPUTIH then a:=a and data.queenwhite
      else a:=a and data.queenblack;
    end;
  'K' :
    begin
      a:=kingmask[sto];
      if giliran=_SISIPUTIH then a:=a and data.kingwhite
      else a:=a and data.kingblack;
    end;
  end;
  if x<>9 then a:=a and file_mask[x]
  else if y<>9 then a:=a and rank_mask[y];
  sfrom:=firstbitp(@a);
  result:=getnotasi(sfrom)+getnotasi(sto);
end;

begin

  if notasi[1] in ['a'..'h'] then result:=notasi_pion else
  if notasi[1] in ['N','B','R','Q','K'] then result:=notasi_perwira;

end;

function notationtomove(notasi:string;data:tdata;giliran:byte):integer;
var x,y:byte;
sfrom,sto:byte;
begin
  if notasi[1] in ['a'..'h'] then
  begin
    x:=ord(notasi[1])-97;
    y:=strtoint(notasi[2])-1;
    sfrom:=y*8+x;
    x:=ord(notasi[3])-97;
    y:=strtoint(notasi[4])-1;
    sto:=y*8+x;
    if (sfrom=e1) and (sto=g1) and (data.papan[e1]=_RAJAPUTIH) then
    begin
      result:=sfrom+(_ROKADEPENDEK shl 7);exit;
    end;
    if (sfrom=e1) and (sto=c1) and (data.papan[e1]=_RAJAPUTIH) then
    begin
      result:=sfrom+(_ROKADEPANJANG shl 7);exit;
    end;
    if (sfrom=e8) and (sto=g8) and (data.papan[e8]=_RAJAHITAM) then
    begin
      result:=sfrom+(_ROKADEPENDEK shl 7);exit;
    end;
    if (sfrom=e8) and (sto=c8) and (data.papan[e8]=_RAJAHITAM) then
    begin
      result:=sfrom+(_ROKADEPANJANG shl 7);exit;
    end;
    if (data.papan[sfrom]=_PIONPUTIH) and ((sto-sfrom) in [7,9]) and (data.papan[sto]=0) then
    begin
      result:=sfrom+ _en_passant shl 7;exit;
    end;
    if (data.papan[sfrom]=_PIONHITAM) and ((sfrom-sto) in [7,9]) and (data.papan[sto]=0) then
    begin
      result:=sfrom+ _en_passant shl 7;exit;
    end;
    result:=sfrom+sto shl 7;
    if length(notasi)>4 then
    begin
      if notasi[5]='q' then result:=result+PROMOSI_MENTRI shl 14
      else if notasi[5]='r' then result:=result+PROMOSI_BENTENG shl 14
      else if notasi[5]='b' then result:=result+PROMOSI_GAJAH shl 14
      else if notasi[5]='n' then result:=result+PROMOSI_KUDA shl 14;
    end;
  end
  else
  if (copy(notasi,1,3)='O-O') and (giliran=_SISIPUTIH) then
      result:=e1+ _ROKADEPENDEK shl 7
  else
  if (copy(notasi,1,3)='O-O') and (giliran=_SISIHITAM) then
      result:=e8+ _ROKADEPENDEK shl 7
  else
  if (copy(notasi,1,5)='O-O-O') and (giliran=_SISIHITAM) then
      result:=e8+ _ROKADEPANJANG shl 7
  else
  if (copy(notasi,1,5)='O-O-O') and (giliran=_SISIPUTIH) then
      result:=e1+ _ROKADEPANJANG shl 7

end;


Function movetonotation;
var sfrom,sto,promosi,n:integer;
begin
  result:='';
  sfrom:=move and 127;
  sto:=(move shr 7) and 127;
  promosi:=(move shr 14) and 7;
  if sto=_ROKADEPENDEK then
  begin
    //result:='0-0'
    if giliran=_SISIPUTIH then result:='e1g1' else
    result:='e8g8';
  end else
  if sto=_ROKADEPANJANG then
  begin
//    result:='0-0-0'
    if giliran=_SISIPUTIH then result:='e1c1' else
    result:='e8c8';
  end else
  if sto=_EN_PASSANT then
  begin
    if giliran=_SISIPUTIH then
    begin
      IF ep>sfrom mod 8 THEN
        sto:=sfrom+9
      ELSE
       sto:=sfrom+7;
    end else
    begin
      IF ep>sfrom mod 8 THEN
       sto:=sfrom-7
      ELSE
       sto:=sfrom-9;
    end;
    result:=getnotasi(sfrom)+getnotasi(sto){+'ep'};
  end else
  begin
     result:=getnotasi(sfrom)+getnotasi(sto);
     if promosi=PROMOSI_MENTRI then result:=result+'q' else
     if promosi=PROMOSI_KUDA then result:=result+'n' else
     if promosi=PROMOSI_GAJAH then result:=result+'b' else
     if promosi=PROMOSI_BENTENG then result:=result+'r';
  end;

end;

end.
