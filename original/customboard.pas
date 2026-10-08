unit customboard;
//unit yang digunakan untuk mengkonfigurasi papan

interface
uses header;

procedure fillbitboard(var data:tdata);
procedure readnotation(n:string;var data:tdata;var wtm:boolean);

implementation
uses sysutils,bitboard_mask;
var a,b,pos,kar:integer;

procedure readnotation(n:string;var data:tdata;var wtm:boolean);
var wq,wk,bq,bk:boolean;
begin
  pos:=56;
  data.flagrokade:=0;


  for a:=1 to length(n) do
  begin

    if n[a] in ['0'..'9'] then
    begin
      for b:=pos to pos+strtoint(n[a])-1 do
        data.papan[b]:=0;
      pos:=pos+strtoint(n[a]);
    end else
    if n[a]='R' then data.papan[pos]:=_bentengputih else
    if n[a]='B' then data.papan[pos]:=_gajahputih else
    if n[a]='N' then data.papan[pos]:=_kudaputih else
    if n[a]='Q' then data.papan[pos]:=_mentriputih else
    if n[a]='P' then data.papan[pos]:=_pionputih else
    if n[a]='K' then data.papan[pos]:=_rajaputih else
    if n[a]='r' then data.papan[pos]:=_bentenghitam else
    if n[a]='b' then data.papan[pos]:=_gajahhitam else
    if n[a]='n' then data.papan[pos]:=_kudahitam else
    if n[a]='q' then data.papan[pos]:=_mentrihitam else
    if n[a]='p' then data.papan[pos]:=_pionhitam else
    if n[a]='k' then data.papan[pos]:=_rajahitam else
    if n[a]='/' then pos:=pos-16 else
    if n[a]=' ' then begin
      if n[a+1]='b' then wtm:=false
      else if n[a+1]='w' then wtm:=true;
      b:=a;
      inc(b,3);
      wq:=false;wk:=false;bq:=false;bk:=false;
      while (b<=length(n)) and (n[b] in ['K','Q','k','q'])do
      begin
        case n[b] of
          'K' : wk:=true;
          'Q' : wq:=true;
          'k' : bk:=true;
          'q' : bq:=true;
        end;
        inc(b);
      end;
      if wk then
        data.flagrokade:=data.flagrokade or bitputihSC;
      if wq then
        data.flagrokade:=data.flagrokade or bitputihlc;
      if bk then
        data.flagrokade:=data.flagrokade or bithitamsc;
      if bq then
        data.flagrokade:=data.flagrokade or bithitamlc;

      exit;
    end;
    if n[a] in['a'..'z','A'..'Z'] then inc(pos);
  end;
{
  if data.papan[e1]<>_RAJAPUTIH then data.flagrokade:=data.flagrokade or BITRAJAPUTIHNOROKADE;
  if data.papan[e8]<>_RAJAHITAM then data.flagrokade:=data.flagrokade or BITRAJAHITAMNOROKADE;
  if data.papan[h1]<>_BENTENGPUTIH then data.flagrokade:=data.flagrokade or BITBENTENGPUTIH2MOVE;
  if data.papan[a1]<>_BENTENGPUTIH then data.flagrokade:=data.flagrokade or BITBENTENGPUTIH1MOVE;
  if data.papan[h8]<>_BENTENGHITAM then data.flagrokade:=data.flagrokade or BITBENTENGHITAM2MOVE;
  if data.papan[a8]<>_BENTENGHITAM then data.flagrokade:=data.flagrokade or BITBENTENGHITAM1MOVE;
}

end;

procedure fillbitboard(var data:tdata);
var a:integer;
begin
  with data do
  begin
    whitepieces:=0;blackpieces:=0;allpiecesr90:=0;allpiecesa8h1:=0;allpiecesh8a1:=0;
    allpieces:=0;
    kingblack:=0;kingwhite:=0;pawnblack:=0;pawnwhite:=0;
    knightblack:=0;knightwhite:=0;bishopblack:=0;bishopwhite:=0;
    rookblack:=0;rookwhite:=0;queenblack:=0;queenwhite:=0;
    materialscore:=0;
    data.hashkey:=0;
    data.ep:=_NO_EP;
    data.nilai_perwira_putih:=0;data.nilai_perwira_hitam:=0;
    data.wc:=0;data.bc:=0;
    data.ngajahhitam:=0;data.ngajahputih:=0;

    for a:=0 to 63 do
    begin
        if data.papan[a]=_pionputih then
        begin
          inc(data.materialscore,_nilai_pion);
//          inc(data.jum_materi_putih);
          data.pawnwhite:=data.pawnwhite or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.whitepieces:=data.whitepieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
        end else
        if data.papan[a]=_pionhitam then
        begin
          dec(data.materialscore,_nilai_pion);
//          inc(data.jum_materi_hitam);
          data.pawnblack:=data.pawnblack or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.blackpieces:=data.blackpieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
        end;
        if data.papan[a]=_kudaputih then
        begin
          inc(data.materialscore,_nilai_kuda);
//          inc(data.jum_materi_putih);
          data.knightwhite:=data.knightwhite or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.whitepieces:=data.whitepieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
          inc(data.nilai_perwira_putih,3);
//          inc(data.jum_perwira_putih);
        end else
        if data.papan[a]=_gajahputih then
        begin
          inc(data.materialscore,_nilai_gajah);
//          inc(data.jum_materi_putih);
          data.bishopwhite:=data.bishopwhite or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.whitepieces:=data.whitepieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
          inc(data.ngajahputih);
//          inc(data.jum_perwira_putih);
          inc(data.nilai_perwira_putih,3);
        end else
        if data.papan[a]=_bentengputih then
        begin
          inc(data.materialscore,_nilai_benteng);
          data.rookwhite:=data.rookwhite or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.whitepieces:=data.whitepieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];

          inc(data.nilai_perwira_putih,5);
        end else
        if data.papan[a]=_mentriputih then
        begin
          inc(data.materialscore,_nilai_mentri);
          data.queenwhite:=data.queenwhite or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.whitepieces:=data.whitepieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
          inc(data.nilai_perwira_putih,9);
        end else
        if data.papan[a]=_rajaputih then
        begin
          data.kingwhite:=data.kingwhite or bit2nmask[a];
//          inc(data.jum_materi_putih);
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.whitepieces:=data.whitepieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
        end else
        if data.papan[a]=_kudahitam then
        begin
          dec(data.materialscore,_nilai_kuda);
//          inc(data.jum_materi_hitam);
          data.knightblack:=data.knightblack or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.blackpieces:=data.blackpieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
//          inc(data.jum_perwira_hitam);
          inc(data.nilai_perwira_hitam,3);
        end else
        if data.papan[a]=_gajahhitam then
        begin
          dec(data.materialscore,_nilai_gajah);
//          inc(data.jum_materi_hitam);
          data.bishopblack:=data.bishopblack or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.blackpieces:=data.blackpieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
//          inc(data.jum_perwira_hitam);
          inc(data.nilai_perwira_hitam,3);
          inc(data.ngajahhitam);
        end else
        if data.papan[a]=_bentenghitam then
        begin
          dec(data.materialscore,_nilai_benteng);
//          inc(data.jum_materi_hitam);
          data.rookblack:=data.rookblack or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.blackpieces:=data.blackpieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
//          inc(data.jum_perwira_hitam);
          inc(data.nilai_perwira_hitam,5);
        end else
        if data.papan[a]=_mentrihitam then
        begin
          dec(data.materialscore,_nilai_mentri);
//          inc(data.jum_materi_hitam);
          data.queenblack:=data.queenblack or bit2nmask[a];
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.blackpieces:=data.blackpieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
//          inc(data.jum_perwira_hitam);
          inc(data.nilai_perwira_hitam,9);
        end else
        if data.papan[a]=_rajahitam then
        begin
          data.kingblack:=data.kingblack or bit2nmask[a];
//          inc(data.jum_materi_hitam);
          data.allpieces:=data.allpieces or bit2nmask[a];
          data.blackpieces:=data.blackpieces or bit2nmask[a];
          data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[a];
          data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[a];
          data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[a];
        end;
    end;
  end;
  data.move50count:=0;
  data.rookqueen:=data.queenblack or data.queenwhite or data.rookblack or data.rookwhite;
  data.bishopqueen:=data.queenblack or data.queenwhite or data.bishopblack or data.bishopwhite;
//  data.bishop_oppc:=false;

{    if data.papan[e1]<>_RAJAPUTIH then data.flagrokade:=data.flagrokade or BITRAJAPUTIHNOROKADE;
    if data.papan[e8]<>_RAJAHITAM then data.flagrokade:=data.flagrokade or BITRAJAHITAMNOROKADE;
    if data.papan[h1]<>_BENTENGPUTIH then data.flagrokade:=data.flagrokade or BITBENTENGPUTIH2MOVE;
    if data.papan[a1]<>_BENTENGPUTIH then data.flagrokade:=data.flagrokade or BITBENTENGPUTIH1MOVE;
    if data.papan[h8]<>_BENTENGHITAM then data.flagrokade:=data.flagrokade or BITBENTENGHITAM2MOVE;
    if data.papan[a8]<>_BENTENGHITAM then data.flagrokade:=data.flagrokade or BITBENTENGHITAM1MOVE;
}

end;


end.
