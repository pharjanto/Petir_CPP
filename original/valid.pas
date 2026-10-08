unit valid;

interface
uses header;
function IsValid(var data:tdata;moves,giliran:integer):boolean;

implementation
uses bitboard_mask,tools;

function IsValid(var data:tdata;moves,giliran:integer):boolean;
var sfrom,sto,piece,piece2,d:integer;
bitto,attack:int64;
begin
{  if moves=_NO_MOVE then
  begin
    result:=false;exit;
  end;}
  sfrom:=moves and 127;
  sto:=(moves shr 7) and 127;
  result:=false;
//  if sfrom=sto then exit;
  piece:=data.papan[sfrom];


  if sto<=63 then
  begin
    if (piece>0) and (data.papan[sto]>0) then
      exit
    else if (piece<0) and (data.papan[sto]<0) then exit;
  end else
  if ((sto=65) or (sto=66)) then
  begin
    if (giliran=_SISIPUTIH) and (data.papan[e1]<>_RAJAPUTIH) then exit;
    if (giliran=_SISIHITAM) and (data.papan[e8]<>_RAJAHITAM) then exit;
  end;
  if (giliran=_SISIPUTIH) and (data.papan[sfrom]<=0) then exit;
  if (giliran=_SISIHITAM) and (data.papan[sfrom]>=0) then exit;

  piece2:=abs(piece);
  if sto<=63 then
    bitto:=bit2nmask[sto];
    
  if piece=_PIONPUTIH then
  begin
    d:=sto-sfrom;
    if sto=_EN_PASSANT then
    begin
      IF (abs((sfrom and 7)-data.ep)=1)
      AND (sfrom shr 3=4) THEN
      begin
        result:=true;exit;
      end;
    end;
    if ((d=7) or (d=9)) and (data.papan[sto]<0) then
    begin
      result:=true;exit;
    end;
    if (d=8) and (data.papan[sto]=0)  then
    begin
      result:=true;exit;
    end;
    if (d=16) and (data.papan[sto]=0) and (data.papan[sfrom+8]=0) and (sfrom<=h2) then
    begin
      result:=true;exit;
    end;
  end;
  if piece=_PIONHITAM then
  begin
    d:=sfrom-sto;
    if sto=_EN_PASSANT then
    begin
      IF (abs((sfrom and 7)-data.ep)=1)
      AND (sfrom shr 3=3) THEN
      begin
        result:=true;exit;
      end;
    end;
    if ((d=7) or (d=9)) and (data.papan[sto]>0) then
    begin
      result:=true;exit;
    end;
    if (d=8) and (data.papan[sto]=0)  then
    begin
      result:=true;exit;
    end;

    if (d=16) and (data.papan[sto]=0) and (data.papan[sfrom-8]=0) and (sfrom>=a7) then
    begin
      result:=true;exit;
    end;
  end;

  if piece2=_KUDAPUTIH then
  begin
    if knightmask[sfrom] and bitto<>0 then
    begin
      result:=true;exit;
    end;
  end;
  if (piece2=_BENTENGPUTIH) or (piece2=_MENTRIPUTIH) then
  begin
    attack:=horzmask2[sfrom,(data.allpieces shr (sfrom and 56+1)) and 63];
    attack:=attack or vertmask2[sfrom,(data.allpiecesr90 shr vershiftmask[sfrom]) and 63];
    if (attack and bitto<>0) and ((moves shr 14) and 7=0) then
    begin
      result:=true;exit;
    end;
  end;
  if (piece2=_GAJAHPUTIH) or (piece2=_MENTRIPUTIH) then
  begin
    attack:=diaga8h1mask[sfrom,(data.allpiecesa8h1 shr a8h1shiftmask[sfrom]) and (255)];
    attack:=attack or diagh8a1mask[sfrom,(data.allpiecesh8a1 shr h8a1shiftmask[sfrom]) and (255)];
    if (attack and bitto<>0)  and ((moves shr 14) and 7=0) then
    begin
      result:=true;exit;
    end;
  end;
  if piece=_RAJAPUTIH then
  begin
    if sto=_ROKADEPENDEK then
    begin
      if (data.flagrokade AND BITPUTIHSC<>0)
      AND (data.papan[e1]=_RAJAPUTIH) AND
      (data.papan[f1]=0) AND (data.papan[g1]=0)
      AND NOT white_attacked(data,e1)
      AND NOT white_attacked(data,f1)
      AND NOT white_attacked(data,g1)
      AND (data.papan[h1]=_BENTENGPUTIH) then
      begin
        result:=true;exit;
      end;
    end else
    if sto=_ROKADEPANJANG then
    begin
      if (data.flagrokade AND BITPUTIHLC<>0)
      AND (data.papan[e1]=_RAJAPUTIH) AND
      (data.papan[d1]=0) AND (data.papan[c1]=0) AND (data.papan[b1]=0)
      AND NOT white_attacked(data,e1)

      AND NOT white_attacked(data,d1)
      AND NOT white_attacked(data,c1)
      AND (data.papan[a1]=_BENTENGPUTIH) then
      begin
        result:=true;exit;
      end;
    end else
    if kingmask[sfrom] and bitto<>0 then
    begin
      result:=true;exit;
    end;
  end;
  if piece=_RAJAHITAM then
  begin
    if sto=_ROKADEPENDEK then
    begin
      if (data.flagrokade AND BITHITAMSC<>0) AND NOT black_attacked(data,e8)
      AND (data.papan[e8]=_RAJAHITAM)
      AND
      (data.papan[f8]=0) AND (data.papan[g8]=0)
      AND NOT black_attacked(data,f8)
      AND NOT black_attacked(data,g8)
      AND (data.papan[h8]=_BENTENGHITAM) then
      begin
        result:=true;exit;
      end;
    end else
    if sto=_ROKADEPANJANG then
    begin
      if (data.flagrokade AND BITHITAMLC<>0)
      AND (data.papan[e8]=_RAJAHITAM) AND
      NOT black_attacked(data,e8) AND
      (data.papan[d8]=0) AND (data.papan[c8]=0) AND (data.papan[b8]=0)
      AND NOT black_attacked(data,d8)
      AND NOT black_attacked(data,c8)
      AND (data.papan[a8]=_BENTENGHITAM) then
      begin
        result:=true;exit;
      end;
    end else
    if kingmask[sfrom] and bitto<>0 then
    begin
      result:=true;exit;
    end;

  end;

end;


end.
