unit tools;

interface
uses header;
type
  pinteger=^integer;

function white_attacked(var data:tdata;pos:integer):boolean;
function black_attacked(var data:tdata;pos:integer):boolean;
function white_checked(var data:tdata):boolean;
function black_checked(var data:tdata):boolean;
procedure clearkiller;
function white_open_check(var data:tdata;moves:word):boolean;
function white_open_check2(var data:tdata;pos2,post,posraja:word):boolean;
function black_open_check2(var data:tdata;pos2,post,posraja:word):boolean;
function black_open_check(var data:tdata;moves:word):boolean;
function IsExtension(var data:tdata;var newlevel:integer;giliran:integer;moves:integer;var cext:integer):boolean;
function winningmaterial(ms:integer;giliran:byte):boolean;
function losingmaterial(ms:integer;giliran:byte):boolean;
function menskak(var data:tdata;giliran:integer):boolean;
function menskak2(var data:tdata;giliran,moves:word):boolean;
function pawnpush(var data:tdata;giliran,moves:word):boolean;
function capture(var data:tdata;moves:word):boolean;
function white_forkAndPin(move:integer;var data:tdata):boolean;
function black_forkAndPin(move:integer;var data:tdata):boolean;
function captured_value(sto:integer;var data:tdata):integer;
function white_threat(var data:tdata;moves:word;var threatpiece:integer):boolean;
function black_threat(var data:tdata;moves:word;var threatpiece:integer):boolean;
function pawn_7_rank(giliran:integer;var data:tdata):boolean;
function pawn_7_rankw(var data:tdata):boolean;
function pawn_7_rankb(var data:tdata):boolean;
function pawn_6_rank(giliran:integer;var data:tdata):boolean;
function good_move_white(var data:tdata;move:integer):boolean;
function good_move_black(var data:tdata;move:integer):boolean;
function wb_threat(giliran:byte;var data:tdata;moves:word;var threatpiece:integer):boolean;
function material(var data:tdata;giliran:Integer):integer;
function GetFen(data:Tdata;giliran:integer):String;
function Good_move(var data:tdata;move,giliran:integer):boolean;
implementation
uses bitboard_mask,usee,sysutils;

var anu:integer;

function GetFen(data:Tdata;giliran:integer):String;
var a,b,Z,m,r:integer;
temp:string;
lastempty:integer;
begin
  b:=0;
  result:='';
  temp:='';
  lastempty:=0;
  for a:=63 downto 0 do
  begin
    Z:=a div 8 * 8;
    m:=7-(a mod 8);
    r:=z+m;
    if data.papan[r]<>0 then
      if lastempty<>0 then
      begin
        temp:=temp+inttostr(lastempty);
        lastempty:=0;
      end;
    case data.papan[r] of
      _PIONHITAM : temp:=temp+'p';
      _KUDAHITAM : temp:=temp+'n';
      _GAJAHHITAM : temp:=temp+'b';
      _BENTENGHITAM : temp:=temp+'r';
      _MENTRIHITAM : temp:=temp+'q';
      _RAJAHITAM : temp:=temp+'k';
      _PIONPUTIH : temp:=temp+'P';
      _KUDAPUTIH : temp:=temp+'N';
      _GAJAHPUTIH : temp:=temp+'B';
      _BENTENGPUTIH : temp:=temp+'R';
      _MENTRIPUTIH : temp:=temp+'Q';
      _RAJAPUTIH : temp:=temp+'K';
      0 : lastempty:=lastempty+1;
    end;
    if a mod 8 = 0 then
    begin
      if lastempty<>0 then
        temp:=temp+inttostr(lastempty);
      result:=result+temp;
      if (a<=56) and (a>0) then result:=result+'/';
      temp:='';
      lastempty:=0;
    end;

  end;
  result:=result+' ';
  if giliran=_SISIPUTIH then
    result:=result+'w' else
  result:=result+'b';
  result:=result+' - -';
end;



function material(var data:tdata;giliran:Integer):integer;
begin
  if giliran=_SISIPUTIH then
    result:=data.NILAI_perwira_putih
  else
    result:=data.NILAI_perwira_hitam;
end;

function good_move_white(var data:tdata;move:integer):boolean;
begin
  result:=false;
  if move>63 then exit;
  if (data.papan[move]=_BENTENGPUTIH) and (move>=a7) then
  begin
    result:=true;
  end else
  if (data.papan[move]=_PIONPUTIH) and (w_pion_bebas_mask[move] and data.pawnblack=0) then
  begin
    result:=true;
  end;
end;

function good_move_black(var data:tdata;move:integer):boolean;
begin
  result:=false;
  if move>63 then exit;
  if (data.papan[move]=_BENTENGHITAM) and (move<=h2) then
  begin
    result:=true;
  end else
  if (data.papan[move]=_PIONHITAM) and (b_pion_bebas_mask[move] and data.pawnwhite=0) then
  begin
    result:=true;
  end;
end;

function Good_move(var data:tdata;move,giliran:integer):boolean;
begin
  if giliran=_SISIPUTIh then
    result:=Good_move_white(data,move)
  else
    result:=Good_move_black(data,move);
end;

function pawn_6_rank(giliran:integer;var data:tdata):boolean;
var a:integer;
begin
  if giliran=_SISIHITAM then
  begin
      result:=false;
//      for a:=a3 to h3 do
      if (data.papan[d3]=_PIONHITAM) and
      (data.papan[d2]=0) and (data.papan[d1]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[e3]=_PIONHITAM) and
      (data.papan[e2]=0) and (data.papan[e1]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[a3]=_PIONHITAM) and
      (data.papan[a2]=0) and (data.papan[a1]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[b3]=_PIONHITAM) and
      (data.papan[b2]=0) and (data.papan[b1]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[c3]=_PIONHITAM) and
      (data.papan[c2]=0) and (data.papan[c1]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[f3]=_PIONHITAM) and
      (data.papan[f2]=0) and (data.papan[f1]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[g3]=_PIONHITAM) and
      (data.papan[g2]=0) and (data.papan[g1]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[h3]=_PIONHITAM) and
      (data.papan[h2]=0) and (data.papan[h1]=0) then
      begin
        result:=true;exit;
      end;

  end else
  begin
      result:=false;
      if (data.papan[d6]=_PIONPUTIH) and
      (data.papan[d7]=0) and (data.papan[d8]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[e6]=_PIONPUTIH) and
      (data.papan[e7]=0) and (data.papan[e8]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[a6]=_PIONPUTIH) and
      (data.papan[a7]=0) and (data.papan[a8]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[b6]=_PIONPUTIH) and
      (data.papan[b7]=0) and (data.papan[b8]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[c6]=_PIONPUTIH) and
      (data.papan[c7]=0) and (data.papan[c8]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[f6]=_PIONPUTIH) and
      (data.papan[f7]=0) and (data.papan[f8]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[g6]=_PIONPUTIH) and
      (data.papan[g7]=0) and (data.papan[g8]=0) then
      begin
        result:=true;exit;
      end;
      if (data.papan[h6]=_PIONPUTIH) and
      (data.papan[h7]=0) and (data.papan[h8]=0) then
      begin
        result:=true;exit;
      end;

  end;
end;


function pawn_7_rank(giliran:integer;var data:tdata):boolean;
var a:integer;
begin
  if giliran=_SISIHITAM then
  begin
      result:=rank_mask[1] and data.pawnblack<>0
  end else
  begin
      result:=rank_mask[6] and data.pawnwhite<>0
  end;
end;

function pawn_7_rankw(var data:tdata):boolean;
var a:integer;
begin
  begin
      result:=rank_mask[6] and data.pawnwhite<>0
  end;
end;

function pawn_7_rankb(var data:tdata):boolean;
var a:integer;
begin
  begin
      result:=rank_mask[1] and data.pawnblack<>0
  end;
end;


function captured_value(sto:integer;var data:tdata):integer;
begin
  if data.cp=0 then result:=0 else
  begin
    if sto=_EN_PASSANT then result:=_NILAI_PION
    else if sto>=_ROKADEPENDEK then result:=0
    else
    result:=nilai_piece[data.papan[sto]];
    inc(result,50);
  end;
end;

function capture;
var sto:byte;
begin
  result:=true;
  sto:=moves shr 7 and 127;
  if sto=_EN_PASSANT then exit;
  if (sto>63) or (data.papan[sto]=0) then
  result:=false;
end;

function wb_threat(giliran:byte;var data:tdata;moves:word;var threatpiece:integer):boolean;
begin
  if giliran=_SISIPUTIH then
    result:=white_threat(data,moves,threatpiece)
  else
    result:=black_threat(data,moves,threatpiece)
end;

function black_threat(var data:tdata;moves:word;var threatpiece:integer):boolean;
var attack,raja,anu:int64;
sto,posraja:byte;
temp,x,y:integer;
begin
 threatPiece:=0;
 posraja:=lastbitp(@data.kingwhite);
 sto:=moves shr 7 and 127;
 if sto>63 then
 begin
   if (sto=_ROKADEPENDEK) or (sto=_ROKADEPANJANG) then
   begin
     threatpiece:=50;result:=false;exit;
   end;
   result:=true;exit;
 end;
 result:=false;
 attack:=0;
 raja:=kingmask[posraja] or bit2nmask[posraja];
 case data.papan[sto] of
   _PIONHITAM :
   begin
     if sto<=h2 then
     begin
       result:=true;exit;
     end;
     attack:=b_pawn_attack[sto];
   end;
   _KUDAHITAM : attack:=knightmask[sto];
   _GAJAHHITAM :
   begin
     attack:=0;
     x:=sto and 7;
     y:=sto shr 3;
     inc(x);inc(y);
     while (x<=7) and (y<=7) and (x>=0) and (y>=0) do
     begin
       temp:=y shl 3 +x;
       attack:=attack or bit2nmask[temp];
       if data.papan[temp]=_MENTRIHITAM then
       begin
         inc(x);inc(y);
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       inc(x);inc(y);
     end;
     x:=sto and 7;
     y:=sto shr 3;
     dec(x);inc(y);
     while (x<=7) and (y<=7) and (x>=0) and (y>=0) do
     begin
       temp:=y shl 3 +x;
       attack:=attack or bit2nmask[temp];
       if data.papan[temp]=_MENTRIHITAM then
       begin
         dec(x);inc(y);
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       dec(x);inc(y);
     end;
     x:=sto and 7;
     y:=sto shr 3;
     inc(x);dec(y);
     while (x<=7) and (y<=7) and (x>=0) and (y>=0) do
     begin
       temp:=y shl 3 +x;
       attack:=attack or bit2nmask[temp];
       if data.papan[temp]=_PIONHITAM then
       begin
         inc(x);dec(y);
         temp:=y shl 3 +x;
         attack:=attack or bit2nmask[temp];
         break;
       end;
       if data.papan[temp]=_MENTRIHITAM then
       begin
         inc(x);dec(y);
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       inc(x);dec(y);
     end;
     x:=sto and 7;
     y:=sto shr 3;
     dec(x);dec(y);
     while (x<=7) and (y<=7) and (x>=0) and (y>=0) do
     begin
       temp:=y shl 3 +x;
       attack:=attack or bit2nmask[temp];
       if data.papan[temp]=_PIONHITAM then
       begin
         dec(x);dec(y);
         temp:=y shl 3 +x;
         attack:=attack or bit2nmask[temp];
         break;
       end;

       if data.papan[temp]=_MENTRIHITAM then
       begin
         dec(x);dec(y);
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       dec(x);dec(y);
     end;

     attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
     attack:=attack or diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
   end;
   _BENTENGHITAM :
   begin
     anu:=data.allpieces and not data.queenblack;
     attack:=horzmask2[sto,(anu shr (sto and 56+1)) and 63];
     //attack:=attack or vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
     temp:=sto+8;
     while temp<=63 do
     begin
       attack:=attack or bit2nmask[temp];
       if (data.papan[temp]=_BENTENGHITAM) or (data.papan[temp]=_MENTRIHITAM) then
       begin
         temp:=temp+8;
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       temp:=temp+8;
     end;
     temp:=sto-8;
     while temp>=0 do
     begin
       attack:=attack or bit2nmask[temp];
       if (data.papan[temp]=_BENTENGHITAM) or (data.papan[temp]=_MENTRIHITAM) then
       begin
         temp:=temp-8;
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       temp:=temp-8;
     end;

   end;
   _MENTRIHITAM:
   begin
     attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
     attack:=attack or diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
     anu:=data.allpieces and not data.rookblack;
     attack:=attack or horzmask2[sto,(anu shr (sto and 56+1)) and 63];
     //attack:=attack or vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
     temp:=sto+8;
     while temp<=63 do
     begin
       attack:=attack or bit2nmask[temp];
       if (data.papan[temp]=_BENTENGHITAM) or (data.papan[temp]=_MENTRIHITAM) then
       begin
         temp:=temp+8;
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       temp:=temp+8;
     end;
     temp:=sto-8;
     while temp>=0 do
     begin
       attack:=attack or bit2nmask[temp];
       if (data.papan[temp]=_BENTENGHITAM) or (data.papan[temp]=_MENTRIHITAM) then
       begin
         temp:=temp-8;
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       temp:=temp-8;
     end;

   end;
   _RAJAHITAM : attack:=knightmask[sto];
 end;
 threatpiece:=0;
 if attack and raja<>0 then
 begin
   result:=true;exit;
 end;
 if attack and data.queenwhite<>0 then threatpiece:=_NILAI_MENTRI else
 if attack and data.rookwhite<>0 then threatpiece:=_NILAI_BENTENG else
 if attack and data.bishopwhite<>0 then threatpiece:=_NILAI_GAJAH else
 if attack and data.knightwhite<>0 then threatpiece:=_NILAI_KUDA else
 if attack and data.pawnwhite<>0 then threatpiece:=_NILAI_PION;

end;


function white_threat(var data:tdata;moves:word;var threatpiece:integer):boolean;
var attack,raja,anu:int64;
sto,posraja:byte;
temp,x,y:integer;
begin
 threatPiece:=0;
 posraja:=lastbitp(@data.kingblack);
 sto:=moves shr 7 and 127;
 if sto>63 then
 begin
   if (sto=_ROKADEPENDEK) or (sto=_ROKADEPANJANG) then
   begin
     threatpiece:=50;result:=false;exit;
   end;
   result:=true;exit;
 end;
 result:=false;
 attack:=0;
 raja:=kingmask[posraja] or bit2nmask[posraja];
 case data.papan[sto] of
   _PIONPUTIH :
   begin
     if sto>=a7 then
     begin
       result:=true;exit;
     end;
     attack:=w_pawn_attack[sto];
   end;
   _KUDAPUTIH : attack:=knightmask[sto];
   _GAJAHPUTIH :
   begin
     attack:=0;
     x:=sto and 7;
     y:=sto shr 3;
     inc(x);inc(y);
     while (x<=7) and (y<=7) and (x>=0) and (y>=0) do
     begin
       temp:=y shl 3 +x;
       attack:=attack or bit2nmask[temp];
       if data.papan[temp]=_PIONPUTIH then
       begin
         inc(x);inc(y);
         temp:=y shl 3 +x;
         attack:=attack or bit2nmask[temp];
         break;
       end;
       if data.papan[temp]=_MENTRIPUTIH then
       begin
         inc(x);inc(y);
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       inc(x);inc(y);
     end;
     x:=sto and 7;
     y:=sto shr 3;
     dec(x);inc(y);
     while (x<=7) and (y<=7) and (x>=0) and (y>=0) do
     begin
       temp:=y shl 3 +x;
       attack:=attack or bit2nmask[temp];
       if data.papan[temp]=_PIONPUTIH then
       begin
         dec(x);inc(y);
         temp:=y shl 3 +x;
         attack:=attack or bit2nmask[temp];
         break;
       end;
       if data.papan[temp]=_MENTRIPUTIH then
       begin
         dec(x);inc(y);
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       dec(x);inc(y);
     end;
     x:=sto and 7;
     y:=sto shr 3;
     inc(x);dec(y);
     while (x<=7) and (y<=7) and (x>=0) and (y>=0) do
     begin
       temp:=y shl 3 +x;
       attack:=attack or bit2nmask[temp];
       if data.papan[temp]=_MENTRIPUTIH then
       begin
         inc(x);dec(y);
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       inc(x);dec(y);
     end;
     x:=sto and 7;
     y:=sto shr 3;
     dec(x);dec(y);
     while (x<=7) and (y<=7) and (x>=0) and (y>=0) do
     begin
       temp:=y shl 3 +x;
       attack:=attack or bit2nmask[temp];
       if data.papan[temp]=_MENTRIPUTIH then
       begin
         dec(x);dec(y);
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       dec(x);dec(y);
     end;
   end;
   _BENTENGPUTIH :
   begin
     anu:=data.allpieces and not data.queenwhite;
     attack:=horzmask2[sto,(anu shr (sto and 56+1)) and 63];
     temp:=sto+8;
     while temp<=63 do
     begin
       attack:=attack or bit2nmask[temp];
       if (data.papan[temp]=_BENTENGPUTIH) or (data.papan[temp]=_MENTRIPUTIH) then
       begin
         temp:=temp+8;
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       temp:=temp+8;
     end;
     temp:=sto-8;
     while temp>=0 do
     begin
       attack:=attack or bit2nmask[temp];
       if (data.papan[temp]=_BENTENGPUTIH) or (data.papan[temp]=_MENTRIPUTIH) then
       begin
         temp:=temp-8;
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       temp:=temp-8;
     end;

     //attack:=attack or vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
   end;
   _MENTRIPUTIH:
   begin
     attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
     attack:=attack or diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
     anu:=data.allpieces and not data.rookwhite;
     attack:=attack or horzmask2[sto,(anu shr (sto and 56+1)) and 63];
     temp:=sto+8;
     while temp<=63 do
     begin
       attack:=attack or bit2nmask[temp];
       if (data.papan[temp]=_BENTENGPUTIH) or (data.papan[temp]=_MENTRIPUTIH) then
       begin
         temp:=temp+8;
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       temp:=temp+8;
     end;
     temp:=sto-8;
     while temp>=0 do
     begin
       attack:=attack or bit2nmask[temp];
       if (data.papan[temp]=_BENTENGPUTIH) or (data.papan[temp]=_MENTRIPUTIH) then
       begin
         temp:=temp-8;
         continue;
       end;
       if data.papan[temp]<>0 then
         break;
       temp:=temp-8;
     end;

     //attack:=attack or vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
   end;
   _RAJAPUTIH : attack:=kingmask[sto];
 end;
 threatpiece:=0;
 if attack and raja<>0 then
 begin
   result:=true;exit;
 end;
 if attack and data.queenblack<>0 then threatpiece:=_NILAI_MENTRI else
 if attack and data.rookblack<>0 then threatpiece:=_NILAI_BENTENG else
 if attack and data.bishopblack<>0 then threatpiece:=_NILAI_GAJAH else
 if attack and data.knightblack<>0 then threatpiece:=_NILAI_KUDA else
 if attack and data.pawnblack<>0 then threatpiece:=_NILAI_PION;

end;

function white_menskak(var data:tdata;moves:word):boolean;
var sfrom,sto:byte;
posraja,promosi:integer;
attack:int64;
piece:shortint;
begin
  result:=false;
  posraja:=lastbitp(@data.kingblack);
  sfrom:=moves and 127;sto:=moves shr 7 and 127;
  promosi:=(moves shr 14) and 7;

  piece:=data.papan[sfrom];
  if promosi=PROMOSI_MENTRI then piece:=_MENTRIPUTIH
  else if promosi=PROMOSI_KUDA then piece:=_KUDAPUTIH;

  if (sto=65) or (sto=66) then
  begin
    if sto=65 then sto:=f1
    else sto:=d1;
    piece:=_BENTENGPUTIH;
  end
  else if sto=67 then
  begin
    IF data.ep>sfrom and 7 THEN
       sto:=sfrom+9
    ELSE
       sto:=sfrom+7;
  end;

  case piece of
    _PIONPUTIH :
      if w_pawn_attack[sto] and bit2nmask[posraja]<>0 then result:=true;
    _KUDAPUTIH :
      if knightmask[sto] and bit2nmask[posraja]<>0 then result:=true;
    _GAJAHPUTIH :
      begin
        case
        abs(direction[sto,posraja]) of
          7 :
          begin
            attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          9 :
          begin
            attack:=diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
        end;
      end;
    _BENTENGPUTIH :
      begin
        case
        abs(direction[sto,posraja]) of
          1 :
          begin
            attack:=horzmask2[sto,(data.allpieces shr (sto and 56+1)) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          8 :
          begin
            attack:=vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
        end;
      end;
    _MENTRIPUTIH :
      begin
        case
        abs(direction[sto,posraja]) of
          1 :
          begin
            attack:=horzmask2[sto,(data.allpieces shr (sto and 56+1)) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          8 :
          begin
            attack:=vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          7 :
          begin
            attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          9 :
          begin
            attack:=diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
        end;
      end;

  end;
end;

function ipopcount(p:int64):byte;
begin
  result:=0;
  while p<>0 do
  begin
    p:=p and (p-1);
    inc(result);
  end;
end;

function white_forkAndPin(move:integer;var data:tdata):boolean;
var t64:int64;pos,c:integer;
begin
  result:=false;

  if move>63 then exit;
  case data.papan[move] of
    _PIONPUTIH :
    begin
      c:=0;
      if (move and 7>0) and (data.papan[move+7]<-1) then inc(c)
      else exit;
      if (move and 7<7) and (data.papan[move+9]<-1) then inc(c);
  {    if c=2 then
        result:=true;
      exit;}
      result:=c=2;
    end;
    _KUDAPUTIH:
    begin
      t64:=knightmask[move] and data.blackpieces;
      c:=0;
      while t64<>0 do
      begin
        pos:=firstbitp(@t64);
        t64:=t64 and bit2nmasknot[pos];
        if (data.papan[pos]<=_BENTENGHITAM) then
        begin
          inc(c);
        end;
        if (data.papan[pos]=_PIONHITAM) and (data.papan[pos-8]=_PIONPUTIH)
//        and not white_attacked(data,pos) then
        and (see(_SISIPUTIH,data,move,pos)>0) then
        begin
          result:=true;exit;
        end;
      end;
      result:=c>=2;
    end;
    _GAJAHPUTIH:
    if data.nilai_perwira_hitam<=17 then
    begin
      t64:=diaga8h1mask[move,(data.allpiecesa8h1 shr a8h1shiftmask[move]) and (255)];
      t64:=t64 or diagh8a1mask[move,(data.allpiecesh8a1 shr h8a1shiftmask[move]) and (255)];
      t64:=t64 and data.pawnblack;
      while t64<>0 do
      begin
        pos:=firstbitp(@t64);
        if (data.papan[pos-8]=_PIONPUTIH)
        and (see(_SISIPUTIH,data,move,pos)>0) then
        begin
          result:=true;exit;
        end;
        t64:=t64 and bit2nmasknot[pos];
      end;
    end;
    _BENTENGPUTIH:
    if data.nilai_perwira_hitam<=17 then
    begin
      t64:=horzmask2[move,(data.allpieces shr (move and 56+1)) and 63];
      t64:=t64 or vertmask2[move,(data.allpiecesr90 shr vershiftmask[move]) and 63];
      t64:=t64 and data.pawnblack;
      while t64<>0 do
      begin
        pos:=firstbitp(@t64);
        if (data.papan[pos-8]=_PIONPUTIH)
        and (see(_SISIPUTIH,data,move,pos)>0) then
        begin
          result:=true;exit;
        end;
        t64:=t64 and bit2nmasknot[pos];
      end;
    end;
    _RAJAPUTIH:
    if (data.nilai_perwira_hitam<=3)
    then
    begin
      t64:=kingmask[move] and data.pawnblack;
      while t64<>0 do
      begin
        pos:=firstbitp(@t64);
        t64:=t64 and bit2nmasknot[pos];
        if (kingmask[pos] and data.kingblack=0) and
        ((w_pawn_attack[pos] and data.pawnblack)=0) then
        begin
  //        total_node:=pos+data.papan[pos];
          result:=true;exit;
        end;
      end;
    end;
  end;
end;

function black_forkAndPin(move:integer;var data:tdata):boolean;
var t64:int64;pos,c:integer;
begin
  result:=false;
  if move>63 then exit;
  case data.papan[move] of
    _PIONHITAM :
    begin
      c:=0;
      if (move and 7>0) and (data.papan[move-9]>1) then inc(c) else exit;
      if (move and 7<7) and (data.papan[move-7]>1) then inc(c) else exit;
  {    if c=2 then
        result:=true;}
      result:=c=2;
  //    exit;
    end;
    _KUDAHITAM:
    begin
      t64:=knightmask[move] and data.whitepieces;
      c:=0;
      while t64<>0 do
      begin
        pos:=firstbitp(@t64);
        t64:=t64 and bit2nmasknot[pos];
        if (data.papan[pos]>=_BENTENGPUTIH) then
        begin
          inc(c);
        end;
        if (data.papan[pos]=_PIONPUTIH) and (data.papan[pos+8]=_PIONHITAM)
        and (see(_SISIHITAM,data,move,pos)>0) then
        begin
          result:=true;exit;
        end;

      end;
      result:=c>=2;
    end;
    _GAJAHHITAM:
    if data.nilai_perwira_putih<=17 then
    begin
      t64:=diaga8h1mask[move,(data.allpiecesa8h1 shr a8h1shiftmask[move]) and (255)];
      t64:=t64 or diagh8a1mask[move,(data.allpiecesh8a1 shr h8a1shiftmask[move]) and (255)];
      t64:=t64 and data.pawnwhite;
      while t64<>0 do
      begin
        pos:=firstbitp(@t64);
        if (data.papan[pos+8]=_PIONHITAM)
        and (see(_SISIHITAM,data,move,pos)>0) then
        begin
          result:=true;exit;
        end;
        t64:=t64 and bit2nmasknot[pos];
      end;
    end;
    _BENTENGHITAM:
    if data.nilai_perwira_putih<=17 then    
    begin
      t64:=horzmask2[move,(data.allpieces shr (move and 56+1)) and 63];
      t64:=t64 or vertmask2[move,(data.allpiecesr90 shr vershiftmask[move]) and 63];
      t64:=t64 and data.pawnwhite;
      while t64<>0 do
      begin
        pos:=firstbitp(@t64);

        if (data.papan[pos+8]=_PIONHITAM)
        and (see(_SISIHITAM,data,move,pos)>0) then
        begin
          result:=true;exit;
        end;
        t64:=t64 and bit2nmasknot[pos];
      end;
    end;

    _RAJAHITAM:
    if (data.nilai_perwira_putih<=3)
    then
    begin
      t64:=kingmask[move] and data.pawnwhite;
      while t64<>0 do
      begin
        pos:=firstbitp(@t64);
        t64:=t64 and bit2nmasknot[pos];
        if (kingmask[pos] and data.kingwhite=0) and
        ((b_pawn_attack[pos] and data.pawnwhite)=0) then
        begin
  //        total_node:=pos+data.papan[pos];
          result:=true;exit;
        end;
      end;
    end;
  end;
end;


function black_menskak(var data:tdata;moves:word):boolean;
var sfrom,sto:byte;
posraja,promosi:integer;
attack:int64;
piece:shortint;
begin
  result:=false;
  posraja:=firstbitp(@data.kingwhite);
  sfrom:=moves and 127;sto:=moves shr 7 and 127;
  promosi:=(moves shr 14) and 7;
  piece:=data.papan[sfrom];
  if promosi=PROMOSI_MENTRI then piece:=_MENTRIHITAM
  else if promosi=PROMOSI_KUDA then piece:=_KUDAHITAM;

  if (sto=65) or (sto=66) then
  begin
    if sto=65 then sto:=f8
    else sto:=d8;
    piece:=_BENTENGHITAM;
  end
  else if sto=67 then
  begin
    IF data.ep>sfrom and 7 THEN
       sto:=sfrom-7
    ELSE
       sto:=sfrom-9;
  end;

  case piece of
    _PIONHITAM :
      if b_pawn_attack[sto] and bit2nmask[posraja]<>0 then result:=true;
    _KUDAHITAM :
      if knightmask[sto] and bit2nmask[posraja]<>0 then result:=true;
    _GAJAHHITAM :
      begin
        case
        abs(direction[sto,posraja]) of
          7 :
          begin
            attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          9 :
          begin
            attack:=diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
        end;
      end;
    _BENTENGHITAM :
      begin
        case
        abs(direction[sto,posraja]) of
          1 :
          begin
            attack:=horzmask2[sto,(data.allpieces shr (sto and 56+1)) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          8 :
          begin
            attack:=vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
        end;
      end;
    _MENTRIHITAM :
      begin
        case
        abs(direction[sto,posraja]) of
          1 :
          begin
            attack:=horzmask2[sto,(data.allpieces shr (sto and 56+1)) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          8 :
          begin
            attack:=vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          7 :
          begin
            attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          9 :
          begin
            attack:=diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
        end;
      end;

  end;
end;

function black_menskak3(var data:tdata;moves:word;var threatpiece:integer):boolean;
var sfrom,sto:byte;
posraja:integer;
attack,raja:int64;
begin
  result:=false;
  posraja:=firstbitp(@data.kingwhite);
  sfrom:=moves and 127;sto:=moves shr 7 and 127;
  if sto>63 then
  begin
    result:=true;exit;
  end;
  raja:=bit2nmask[posraja];
  case data.papan[sfrom] of
    _PIONHITAM :
      if b_pawn_attack[sto] and bit2nmask[posraja]<>0 then result:=true;
    _KUDAHITAM :
      if knightmask[sto] and bit2nmask[posraja]<>0 then result:=true;
    _GAJAHHITAM :
      begin
        case
        abs(direction[sto,posraja]) of
          7 :
          begin
            attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          9 :
          begin
            attack:=diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
        end;
      end;
    _BENTENGHITAM :
      begin
        case
        abs(direction[sto,posraja]) of
          1 :
          begin
            attack:=horzmask2[sto,(data.allpieces shr (sto and 56+1)) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          8 :
          begin
            attack:=vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
        end;
      end;
    _MENTRIHITAM :
      begin
        case
        abs(direction[sto,posraja]) of
          1 :
          begin
            attack:=horzmask2[sto,(data.allpieces shr (sto and 56+1)) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          8 :
          begin
            attack:=vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          7 :
          begin
            attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
          9 :
          begin
            attack:=diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
            if attack and bit2nmask[posraja]<>0 then result:=true;
          end;
        end;
      end;

  end;
end;


function menskak(var data:tdata;giliran:integer):boolean;
begin
  result:=false;
  case giliran of
    _SISIPUTIH : if black_checked(data) then result:=true;
    _SISIHITAM : if white_checked(data) then result:=true;
  end;
end;

function menskak2(var data:tdata;giliran,moves:word):boolean;
begin
  result:=false;
  case giliran of
    _SISIPUTIH : if white_menskak(data,moves) or black_open_check(data,moves) then result:=true;
    _SISIHITAM : if black_menskak(data,moves) or white_open_check(data,moves) then result:=true;
  end;
end;


function winningmaterial(ms:integer;giliran:byte):boolean;
begin
  result:=false;
  if (giliran=_sisihitam) and  (data.materialscore<=-_NILAI_BENTENG) then
    result:=true
  else if (giliran=_sisiPUTIH) and  (data.materialscore>=_NILAI_BENTENG) then
    result:=true;
end;

function losingmaterial(ms:integer;giliran:byte):boolean;
begin
  result:=false;
  if (giliran=_sisihitam) and  (data.materialscore>=_NILAI_BENTENG) then
    result:=true
  else if (giliran=_sisiPUTIH) and  (data.materialscore<=-_NILAI_BENTENG) then
    result:=true;
end;

function pawnpush;
var pp:byte;
begin
end;


function IsExtension(var data:tdata;var newlevel:integer;giliran:integer;moves:integer;var cext:integer):boolean;
var pos:byte;d:shortint;
begin
        result:=false;
        pos:=moves and 127;
        if giliran=_SISIHITAM then pos:=63-pos;
        if (pos<=h4) then exit;
        if pos<=h5 then
        begin
          if (data.nilai_perwira_putih>15) and (data.nilai_perwira_hitam>15) then exit;
          if newlevel>24 then exit;
          if (giliran=_SISIPUTIH) then
          begin
            if (data.papan[pos+16]<>0) or (data.papan[pos+24]<>0) then exit;
            if (pos and 7<7) and (data.papan[pos+17]=_PIONHITAM) then exit;
            if (pos and 7>0) and (data.papan[pos+15]=_PIONHITAM) then exit;
            result:=true;
            inc(newlevel,8);dec(cext,8);exit;
          end else
          begin
            pos:=moves and 127;
            if (data.papan[pos-16]<>0) or (data.papan[pos-24]<>0) then exit;
            if (pos and 7<7) and (data.papan[pos-15]=_PIONPUTIH) then exit;
            if (pos and 7>0) and (data.papan[pos-17]=_PIONPUTIH) then exit;
            result:=true;
            inc(newlevel,8);dec(cext,8);exit;
          end;
          exit;
        end;
        if (data.nilai_perwira_putih<=12) and (data.nilai_perwira_hitam<=12)  then
        begin
          inc(newlevel,8);
//          if pp=5 then inc(newlevel,8);
          dec(cext,8);
          result:=true;exit;
        end else
        if (data.nilai_perwira_putih<=17) and (data.nilai_perwira_hitam<=17) then
        begin
          inc(newlevel,8);
          dec(cext,8);
          result:=true;exit;
        end else
        begin
          inc(newlevel,PUSHED_PAWN_EXTENSION_MIDGAME);
          dec(cext,PUSHED_PAWN_EXTENSION_MIDGAME);
          result:=true;exit;
        end;

end;

function black_open_check;
var dn,pos2,adn,post,posraja:integer;
temp:int64;
begin
  pos2:=moves and 127;

  posraja:=lastbitp(@data.kingblack);
  dn:=direction[posraja,pos2];
  if dn=0 then
  begin
    result:=false;exit;
  end;
  post:=moves shr 7 and 127;
  if post =_EN_PASSANT then
  begin
    IF data.ep>pos2 and 7 THEN
      post:=pos2+9
    ELSE
      post:=pos2+7;

  end else
  if post>63 then
  begin
    result:=false;exit;
  end;

  adn:=abs(dn);
  if abs(direction[pos2, post])=adn then
  begin
    result:=false;
    exit;
  end;

  if (adn=7) then
  begin
    temp:=data.allpiecesa8h1 and not bit2nmaska8h1[pos2];
    if diaga8h1mask[posraja,(temp shr a8h1shiftmask[posraja]) and (255)] and (data.queenwhite or data.bishopwhite)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=9) then
  begin
    temp:=data.allpiecesh8a1 and not bit2nmaskh8a1[pos2];
    if diagh8a1mask[posraja,(temp shr h8a1shiftmask[posraja]) and (255)] and (data.queenwhite or data.bishopwhite)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=8) then
  begin
    temp:=data.allpiecesr90 and not bit2nmask90[pos2];
    if vertmask2[posraja,(temp shr vershiftmask[posraja]) and 63] and (data.queenwhite or data.rookwhite)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=1) then
  begin
    temp:=data.allpieces and not bit2nmask[pos2];
    if horzmask2[posraja,(temp shr (posraja and 56+1)) and 63] and (data.queenwhite or data.rookwhite)<>0
    then
    begin
      result:=true;exit;
    end;
  end;
  result:=false;
end;


function white_open_check;
var dn,pos2,adn,post,posraja:integer;
temp:int64;
begin
  pos2:=moves and 127;
  posraja:=firstbitp(@data.kingwhite);

  dn:=direction[posraja,pos2];
  if dn=0 then
  begin
    result:=false;exit;
  end;
  adn:=abs(dn);
  post:=moves shr 7 and 127;
  if post =_EN_PASSANT then
  begin
    IF data.ep>pos2 and 7 THEN
      post:=pos2-7
    ELSE
     post:=pos2-9;
  end else if post>63 then
  begin
    result:=false;exit;
  end;

  if abs(direction[pos2, post])=adn then
  begin
    result:=false;
    exit;
  end;

  if (adn=7) then
  begin
    temp:=data.allpiecesa8h1 and not bit2nmaska8h1[pos2];
    if diaga8h1mask[posraja,(temp shr a8h1shiftmask[posraja]) and (255)] and (data.queenblack or data.bishopblack)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=9) then
  begin
    temp:=data.allpiecesh8a1 and not bit2nmaskh8a1[pos2];
    if diagh8a1mask[posraja,(temp shr h8a1shiftmask[posraja]) and (255)] and (data.queenblack or data.bishopblack)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=8) then
  begin
    temp:=data.allpiecesr90 and not bit2nmask90[pos2];
    if vertmask2[posraja,(temp shr vershiftmask[posraja]) and 63] and (data.queenblack or data.rookblack)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=1) then
  begin
    temp:=data.allpieces and not bit2nmask[pos2];
    if horzmask2[posraja,(temp shr (posraja and 56+1)) and 63] and (data.queenblack or data.rookblack)<>0
    then
    begin
      result:=true;exit;
    end;
  end;
  result:=false;
end;

function white_open_check2;
var dn,adn:integer;
temp:int64;
begin
//  posraja:=firstbitp(@data.kingwhite);

  dn:=direction[posraja,pos2];
  if dn=0 then
  begin
    result:=false;exit;
  end;
  adn:=abs(dn);

  if abs(direction[pos2, post])=adn then
  begin
    result:=false;
    exit;
  end;

  if (adn=7) then
  begin
    temp:=data.allpiecesa8h1 and not bit2nmaska8h1[pos2];
    if diaga8h1mask[posraja,(temp shr a8h1shiftmask[posraja]) and (255)] and (data.queenwhite or data.bishopwhite)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=9) then
  begin
    temp:=data.allpiecesh8a1 and not bit2nmaskh8a1[pos2];
    if diagh8a1mask[posraja,(temp shr h8a1shiftmask[posraja]) and (255)] and (data.queenwhite or data.bishopwhite)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=8) then
  begin
    temp:=data.allpiecesr90 and not bit2nmask90[pos2];
    if vertmask2[posraja,(temp shr vershiftmask[posraja]) and 63] and (data.queenwhite or data.rookwhite)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=1) then
  begin
    temp:=data.allpieces and not bit2nmask[pos2];
    if horzmask2[posraja,(temp shr (posraja and 56+1)) and 63] and (data.queenwhite or data.rookwhite)<>0
    then
    begin
      result:=true;exit;
    end;
  end;
  result:=false;
end;

function black_open_check2;
var dn,adn:integer;
temp:int64;
begin
//  posraja:=firstbitp(@data.kingwhite);

  dn:=direction[posraja,pos2];
  if dn=0 then
  begin
    result:=false;exit;
  end;
  adn:=abs(dn);

  if abs(direction[pos2, post])=adn then
  begin
    result:=false;
    exit;
  end;

  if (adn=7) then
  begin
    temp:=data.allpiecesa8h1 and not bit2nmaska8h1[pos2];
    if diaga8h1mask[posraja,(temp shr a8h1shiftmask[posraja]) and (255)] and (data.queenblack or data.bishopblack)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=9) then
  begin
    temp:=data.allpiecesh8a1 and not bit2nmaskh8a1[pos2];
    if diagh8a1mask[posraja,(temp shr h8a1shiftmask[posraja]) and (255)] and (data.queenblack or data.bishopblack)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=8) then
  begin
    temp:=data.allpiecesr90 and not bit2nmask90[pos2];
    if vertmask2[posraja,(temp shr vershiftmask[posraja]) and 63] and (data.queenblack or data.rookblack)<>0
    then
    begin
      result:=true;exit;
    end;
  end else
  if (adn=1) then
  begin
    temp:=data.allpieces and not bit2nmask[pos2];
    if horzmask2[posraja,(temp shr (posraja and 56+1)) and 63] and (data.queenblack or data.rookblack)<>0
    then
    begin
      result:=true;exit;
    end;
  end;
  result:=false;
end;



procedure clearkiller;
var a:integer;
begin
  for a:=0 to 63 do
  begin
    killer0[a]:=_NO_MOVE;
    killer1[a]:=_NO_MOVE;
  end;
end;

{function noattacking(var data:tdata;giliran,sto:integer):boolean;
var attack:int64;
begin
  result:=true;
  if giliran=_SISIPUTIh then
  begin
    case (data.papan[sto]) of
    _KUDAPUTIH :
       if (knightmask[sto] and data.blackpieces=0) then exit;
    _PIONPUTIH :
       if (w_pawn_attack[sto] and data.blackpieces=0) and (sto shr 3 <=5) then exit;
    _GAJAHPUTIH :
      begin
        attack:=diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)];
        attack:=attack or diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)];
        if attack and data.blackpieces = 0 then exit;
      end;
    end;
  end;
end;}

function white_checked(var data:tdata):boolean;
var
attack:int64;
pos:integer;
BEGIN
  pos:=firstbitp(@data.kingwhite);
  if knightmask[pos] and data.knightblack<>0 then
  begin
      white_checked:=true;exit;
  end;

  attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
  attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
  if attack and (data.queenblack or data.rookblack)<>0 then
  begin
      white_checked:=true;exit;
  end;

  attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
  attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
  if attack and (data.queenblack or data.bishopblack)<>0 then
  begin
      white_checked:=true;exit;
  end;

  IF w_pawn_attack[pos] and data.pawnblack <>0  THEN BEGIN
  	white_checked:=true;exit;
  END;
  if kingmask[pos] and data.kingblack<>0 then
  begin
      white_checked:=true;exit;
  end;

  white_checked:=false;
END;

function black_checked;
var
attack:int64;
pos:integer;
BEGIN
  pos:=lastbitp(@data.kingblack);
  if knightmask[pos] and data.knightwhite<>0 then
  begin
      black_checked:=true;exit;
  end;

  attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
  attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
  if attack and (data.queenwhite or data.rookwhite)<>0 then
  begin
      black_checked:=true;exit;
  end;

  attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
  attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
  if attack and (data.queenwhite or data.bishopwhite)<>0 then
  begin
      black_checked:=true;exit;
  end;

  if b_pawn_attack[pos] and data.pawnwhite<>0 then
  begin
    result:=true;exit;
  end;

  if kingmask[pos] and data.kingwhite<>0 then
  begin
      black_checked:=true;exit;
  end;

  black_checked:=false;
END;


function white_attacked;
var
attack:int64;
BEGIN
  if pos>63 then
  begin
    result:=false;exit;
  end;
  if knightmask[pos] and data.knightblack<>0 then
  begin
      white_attacked:=true;exit;
  end;

  attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
  attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
  if attack and (data.queenblack or data.rookblack)<>0 then
  begin
      white_attacked:=true;exit;
  end;

  attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
  attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
  if attack and (data.queenblack or data.bishopblack)<>0 then
  begin
      white_attacked:=true;exit;
  end;

  IF (pos and 7 >0) and (pos<=63-7 )and (data.papan[pos+7] = _PIONHITAM) THEN BEGIN
  	white_attacked:=true;exit;
  END;
  IF (pos and 7 <7) and (pos<=63-9 ) and (data.papan[pos+9] = _PIONHITAM) THEN BEGIN
  	white_attacked:=true;exit;
  END;

  if kingmask[pos] and data.kingblack<>0 then
  begin
      white_attacked:=true;exit;
  end;

  white_attacked:=false;
END;

function black_attacked;
var
attack:int64;
BEGIN
  if pos>63 then
  begin
    result:=false;exit;
  end;

  if knightmask[pos] and data.knightwhite<>0 then
  begin
      black_attacked:=true;exit;
  end;

  attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
  attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
  if attack and (data.queenwhite or data.rookwhite)<>0 then
  begin
      black_attacked:=true;exit;
  end;

  attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
  attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
  if attack and (data.queenwhite or data.bishopwhite)<>0 then
  begin
      black_attacked:=true;exit;
  end;

  IF (pos and 7 <7) and (pos>6) and (data.papan[pos-7] = _PIONPUTIH) THEN BEGIN
  	black_attacked:=true;exit;
  END;
  IF (pos and 7 >0) and (pos>8) and (data.papan[pos-9] = _PIONPUTIH) THEN BEGIN
  	black_attacked:=true;exit;
  END;

  if kingmask[pos] and data.kingwhite<>0 then
  begin
      black_attacked:=true;exit;
  end;

  black_attacked:=false;
END;



end.
