unit usee;

interface
uses header;
function see(giliran:integer;var data:tdata;sfrom,sto:integer):integer;

implementation
uses bitboard_mask{,windows};


const
  _NILAI_PION=1;
  _NILAI_GAJAH=3;
  _NILAI_KUDA=3;
  _NILAI_BENTENG=5;
  _NILAI_MENTRI=9;
  _NILAI_RAJA=99;
  nilai_piece:array[-6..6] of byte=(_nilai_raja,_nilai_mentri,_nilai_benteng,_nilai_gajah,_nilai_kuda,_nilai_pion,0,_nilai_pion,_nilai_kuda,_nilai_gajah,_nilai_benteng,_nilai_mentri,_nilai_raja);


procedure xray(var attack:int64;square,target:integer;var data:tdata);
var dn,a:integer;
temp:int64;
begin
  //a:=getTickCount;
  dn:=direction2[target,square];
  if dn=0 then exit;
  case dn of
  8  :
  begin
    temp:=(data.bishopqueen) and bishop_attack_plus7_mask[square];
    if temp<>0 then
    attack:=attack or
   (diaga8h1mask[square,(data.allpiecesa8h1 shr a8h1shiftmask[square]) and (255)] and temp);
  end;
  7 :
  begin
    temp:=(data.bishopqueen) and bishop_attack_min7_mask[square];
    if temp<>0 then
    attack:=attack or
   (diaga8h1mask[square,(data.allpiecesa8h1 shr a8h1shiftmask[square]) and (255)] and temp);
  end;
  6  :
  begin
    temp:=(data.bishopqueen) and bishop_attack_plus9_mask[square];
    if temp<>0 then
    attack:=attack or (diagh8a1mask[square,(data.allpiecesh8a1 shr h8a1shiftmask[square]) and (255)] and temp);
  end;
  5 :
  begin
    temp:=(data.bishopqueen) and bishop_attack_min9_mask[square];
    if temp<>0 then
      attack:=attack or (diagh8a1mask[square,(data.allpiecesh8a1 shr h8a1shiftmask[square]) and (255)] and temp);
  end;
  4 :
  begin
    temp:=(data.rookqueen) and rook_attack_right_mask[square];
    if temp<>0 then
      attack:=attack or (horzmask2[square,(data.allpieces shr (square and 56+1)) and 63] and temp);
  end;
  3 :
  begin
    temp:=(data.rookqueen) and rook_attack_left_mask[square];
    if temp<>0 then
      attack:=attack or (horzmask2[square,(data.allpieces shr (square and 56+1)) and 63] and temp);
  end;
  2 :
  begin
    temp:=(data.rookqueen) and rook_attack_up_mask[square];
    if temp<>0 then
    attack:=attack or (vertmask2[square,(data.allpiecesr90 shr vershiftmask[square]) and 63] and temp);
  end;
  else
  begin
    temp:= (data.rookqueen) and rook_attack_down_mask[square];
    if temp<>0 then
    attack:=attack or (vertmask2[square,(data.allpiecesr90 shr vershiftmask[square]) and 63] and temp);
  end;
  end;
  //tx:=tx+(gettickcount-a);
end;

function see(giliran:integer;var data:tdata;sfrom,sto:integer):integer;
var list:array[0..32] of shortint;
attack,temp:int64;
piece:integer;
square,n:integer;
begin
  attack:=knightmask[sto] and (data.knightblack or data.knightwhite);
  attack:=attack or
  ((horzmask2[sto,(data.allpieces shr (sto and 56+1)) and 63] or vertmask2[sto,(data.allpiecesr90 shr vershiftmask[sto]) and 63])
  and (data.rookqueen));

  attack:=attack or
  ((diaga8h1mask[sto,(data.allpiecesa8h1 shr a8h1shiftmask[sto]) and (255)] or
  diagh8a1mask[sto,(data.allpiecesh8a1 shr h8a1shiftmask[sto]) and (255)])
  and (data.bishopqueen));

  attack:=attack or
  (w_pawn_attack[sto] and data.pawnblack);
  attack:=attack or
  (b_pawn_attack[sto] and data.pawnwhite);

  attack:=attack or (kingmask[sto] and (data.kingblack or data.kingwhite));
  list[0]:=nilai_piece[data.papan[sto]];
  xray(attack,sfrom,sto,data);
  attack:=attack and not bit2nmask[sfrom];
  giliran:=3-giliran;
  piece:=nilai_piece[data.papan[sfrom]];
  n:=1;

  while attack<>0 do
  begin

    if giliran=_SISIPUTIH then
    begin
      temp:=attack and data.pawnwhite;
      if temp<>0 then
         square:=lastbitp(@temp)
      else
      begin
        temp:=attack and data.knightwhite;
        if temp<>0 then
           square:=lastbitp(@temp)
        else
        begin
          temp:=attack and data.bishopwhite;
          if temp<>0 then
             square:=lastbitp(@temp)
          else
          begin
            temp:=attack and data.rookwhite;
            if temp<>0 then
              square:=lastbitp(@temp)
            else
            begin
              temp:=attack and data.queenwhite;
              if temp<>0 then
                square:=lastbitp(@temp)
              else
              begin
                temp:=attack and data.kingwhite;
                if temp<>0 then
                  square:=firstbitp(@temp)
                else break;
              end;
            end;
          end;
        end;
      end;
    end else
    begin
      temp:=attack and data.pawnblack;
      if temp<>0 then
         square:=firstbitp(@temp)
      else
      begin
        temp:=attack and data.knightblack;
        if temp<>0 then
           square:=firstbitp(@temp)
        else
        begin
          temp:=attack and data.bishopblack;
          if temp<>0 then
             square:=firstbitp(@temp)
          else
          begin
            temp:=attack and data.rookblack;
            if temp<>0 then
              square:=firstbitp(@temp)
            else
            begin
              temp:=attack and data.queenblack;
              if temp<>0 then
                square:=firstbitp(@temp)
              else
              begin
                temp:=attack and data.kingblack;
                if temp<>0 then
                  square:=lastbitp(@temp)
                else break;
              end;
            end;
          end;
        end;
      end;

    end;

    list[n]:=-list[n-1]+piece;
    piece:=nilai_piece[data.papan[square]];
    xray(attack,square,sto,data);
    attack:=attack and bit2nmasknot[square];
    inc(n);
    giliran:=3-giliran;
  end;//while attack<>0
  dec(n);
  while n>0 do
  begin
    if list[n]>-list[n-1] then
      list[n-1]:=-list[n];
    dec(n);
  end;
  result:=list[0];
end;


end.
